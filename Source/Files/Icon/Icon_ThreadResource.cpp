#include "Icon_ThreadResource.h"
#include "Theme_Image_ConfigFile.h"
#include "Assets_XDGDirectories.h"
#include "Assets.h"

#ifdef JUCE_DEBUG
// Print the full class name before all debug output:
static const constexpr char* dbgPrefix = "Icon::ThreadResource::";
#endif

// SharedResource object instance key:
const juce::Identifier Icon::ThreadResource::resourceKey
        = "Icon::ThreadResource";

// Resource thread name:
static const juce::String threadName = "Icon_ThreadResource";

// Home folder application icon directory:
static const constexpr char* homeIconDir = ".icons";

// Legacy application icon directory:
static const constexpr char* pixmapIconPath = "/usr/share/pixmaps";

// Subdirectory to search for icon files within data directories:
static const constexpr char* iconSubDir = "icons";

// The file in the home directory where the icon theme is stored:
static const constexpr char* iconThemeFile = ".gtkrc-2.0";

// Primary icon theme selection key in the iconThemeFile:
static const constexpr char* iconThemeKey = "gtk-icon-theme-name";

// Backup icon theme selection key in the iconThemeFile:
static const constexpr char* backupThemeKey = "gtk-fallback-icon-theme";

// Icon theme file name:
static const constexpr char* indexFileName = "index.theme";

// Default fallback icon theme:
static const constexpr char* fallbackTheme = "hicolor";

// Pocket-home's icon directory:
// TODO: use XDGDirectories to set data directory.
static const constexpr char* pocketHomeIconPath
        = "/usr/share/pocket-home/icons";

Icon::ThreadResource::ThreadResource() :
SharedResource::Thread::Resource(resourceKey, ::threadName)
{
    using juce::StringArray;
    using juce::String;
    using juce::File;

    // Find all icon data directories to search, ordered from highest to lowest
    // priority. The icon directory search list and priority are defined at
    // https://specifications.freedesktop.org/icon-theme-spec
    // /icon-theme-spec-latest.html
    StringArray dataDirs = Assets::XDGDirectories::getDataSearchPaths();
    for (String& dir : dataDirs)
    {
        if (!dir.endsWithChar('/'))
        {
            dir += '/';
        }
        dir += iconSubDir;
    }
    const String homePath = getenv("HOME");
    iconDirectories.add(homePath + "/" + homeIconDir);

    iconDirectories.addArray(dataDirs);
    iconDirectories.add(pixmapIconPath);
    iconDirectories.add(pocketHomeIconPath);

    // Find the icon themes to use and store them sorted from highest to lowest
    // priority:
    StringArray themeNames;
    StringArray themeSettings;
    String gtkSettingPath(homePath + "/" + iconThemeFile);
    File(gtkSettingPath).readLines(themeSettings);
    for (const String& line : themeSettings)
    {
        int divider = line.indexOfChar('=');
        if (divider != -1)
        {
            String key = line.substring(0, divider);
            if (key == iconThemeKey || key == backupThemeKey)
            {
                themeNames.add(line.substring(divider + 1).unquoted());
                if (themeNames.size() > 1)
                {
                    break;
                }
            }
        }
    }
    themeNames.add(fallbackTheme);

    // Create theme index objects for the user's icon theme and all inherited
    // or fallback themes:
    for (int i = 0; i < themeNames.size(); i++)
    {
        for (const String& dir : iconDirectories)
        {
            File themeDir(dir + (dir.endsWithChar('/') ? "" : "/")
                    + themeNames[i]);
            if (themeDir.isDirectory())
            {
                iconThemes.add(new ThemeIndex(themeDir));
                if (iconThemes.getLast()->isValidTheme())
                {
                    StringArray inherited = iconThemes.getLast()
                            ->getInheritedThemes();
                    int insertParentIdx = i + 1;
                    for (const String& parent : inherited)
                    {
                        if (!themeNames.contains(parent))
                        {
                            themeNames.insert(insertParentIdx, parent);
                            insertParentIdx++;
                        }
                    }
                    break;
                }
                else
                {
                    iconThemes.removeLast();
                }
            }
        }
    }
    #ifdef JUCE_DEBUG
    String dbgThemeNames;
    for (ThemeIndex* theme : iconThemes)
    {
        if (dbgThemeNames.isNotEmpty())
        {
            dbgThemeNames += ", ";
        }
        dbgThemeNames += theme->getName();
    }
    DBG(dbgPrefix << __func__ << ": Loaded icon themes: " << dbgThemeNames);
    #endif
}

Icon::ThreadResource::~ThreadResource()
{
    loadedImageCache.clear();
}


// Cancels a pending icon request.
void Icon::ThreadResource::cancelRequest(const RequestID requestID)
{
    requestMap.erase(requestID);
}


// Adds an icon loading request to the queue.
Icon::RequestID Icon::ThreadResource::addRequest(IconRequest request)
{
    using std::function;
    using juce::Image;

    // Ignore requests without valid callbacks.
    if (!request.loadingCallback)
    {
        return 0;
    }
    // First, attempt to load the icon from the loaded image cache or assets.
    Image preLoadedIcon;
    if (request.icon[0] == '/')
    {
        preLoadedIcon = Assets::loadImageAsset(request.icon);
    }
    if (preLoadedIcon.isNull())
    {
        //if icon is a partial path, trim it
        if (request.icon.contains("/"))
        {
            request.icon = request.icon.substring
                (1 + request.icon.lastIndexOf("/"));
        }
        if (loadedImageCache.contains(request.icon))
        {
            preLoadedIcon = loadedImageCache[request.icon];
            jassert(preLoadedIcon.isValid());
        }
    }
    if (preLoadedIcon.isValid()) // Icon already found, apply now.
    {
        request.loadingCallback(preLoadedIcon);
        return 0;
    }
    else
    {
        // Assign the default icon for now:
        if (!defaultIcon.isValid())
        {
            const Theme::Image::ConfigFile imageConfig;
            const juce::String iconPath = imageConfig.getDefaultIconPath();
            if (!iconPath.isEmpty())
            {
                defaultIcon = Assets::loadImageAsset(iconPath);
            }
            else
            {
                // Create an empty valid image so it doesn't keep trying to
                // find a default:
                defaultIcon = juce::Image(juce::Image::ARGB, request.size,
                        request.size, true);
            }
        }
        request.loadingCallback(defaultIcon);

        // Send the request to the icon loading thread, unless it has tried and
        // failed to find this icon in the past:
        if (missingIcons.contains(request.icon))
        {
            return 0;
        }

        static RequestID newID = 0;
        while (newID == 0 || requestMap.count(newID) != 0)
        {
            newID++;
        }
        requestMap[newID] = request;
        if (!isThreadRunning())
        {
            startResourceThread();
        }
        else //Ensure thread isn't sleeping
        {
            notify();
        }
        return newID;
    }
}


// Asynchronously handles queued icon requests.
void Icon::ThreadResource::runLoop(SharedResource::Thread::Lock& lock)
{
    using juce::Image;
    using juce::String;
    using std::function;
    lock.enterRead();
    RequestID requestID = requestMap.begin()->first;
    IconRequest firstRequest = requestMap[requestID];
    lock.exitRead();

    String icon = firstRequest.icon;
    String iconPath = getIconPath(firstRequest);
    if (iconPath.isNotEmpty())
    {
        // Loading svg files into drawable components requires the message
        // thread to be locked.
        Image iconImg;
        {
            const juce::MessageManagerLock mmLock((juce::Thread*) this);
            if (!mmLock.lockWasGained())
            {
                DBG(dbgPrefix << __func__
                        << ":: Exiting, stop instead of loading icon \""
                        << icon << "\"");
                return;
            }
            iconImg = Assets::loadImageAsset(iconPath);
            if (iconImg.isNull())
            {
                DBG(dbgPrefix << __func__ << ": Unable to load icon " << icon);
                return;
            }
        }

        lock.enterWrite();
        loadedImageCache.set(icon, iconImg);
        lock.exitWrite();

        // If callback wasn't removed, lock the message thread, check again
        // that it's still there, run the callback, and remove it from the map.
        const juce::MessageManagerLock mmLock((juce::Thread*) this);
        if (!mmLock.lockWasGained())
        {
            DBG(dbgPrefix << __func__
                    << ":: Exiting, skipping callback for icon \"" << icon
                    << "\"");
            return;
        }

        lock.enterRead();
        const bool requestPresent = (requestMap.count(requestID) != 0);
        if (requestPresent)
        {
            firstRequest.loadingCallback(iconImg);
        }
        lock.exitRead();

        if (requestPresent)
        {
            lock.enterWrite();
            requestMap.erase(requestID);
            lock.exitWrite();
        }
    }
    else
    {
        // Couldn't find the icon, remove the request
        lock.enterWrite();
        missingIcons.add(firstRequest.icon);
        requestMap.erase(requestID);
        lock.exitWrite();
    }
}


// Keeps the thread dormant when all icon requests have been processed.
bool Icon::ThreadResource::threadShouldWait()
{
    return requestMap.empty();
}


// Search icon theme directories for an icon matching a given request.
juce::String Icon::ThreadResource::getIconPath(const IconRequest& request)
{
    using juce::String;
    using juce::File;

    // Don't waste time searching for icons that weren't found before:
    if (missingIcons.contains(request.icon))
    {
        return String();
    }

    // First, search themes in order to find a matching icon:
    for (const ThemeIndex* themeIndex : iconThemes)
    {
        String iconPath = themeIndex->lookupIcon(request.icon, request.size,
                request.context, request.scale);
        if (iconPath.isNotEmpty())
        {
            return iconPath;
        }
    }
    // If not searching within the application context and the icon name is
    // hyphenated, remove the last section of the name to search for a less
    // specific icon.
    if (request.context != Context::applications &&
        request.icon.containsChar('-'))
    {
        IconRequest subRequest = request;
        subRequest.icon = subRequest.icon.upToLastOccurrenceOf("-", false,
                false);
        String iconPath = getIconPath(subRequest);
        if (iconPath.isNotEmpty())
        {
            return iconPath;
        }
    }
    // If that didn't find anything, search for matching unthemed icon files:
    String unindexedPath = getUnindexedIconPath(request);
    if (unindexedPath.isNotEmpty())
    {
        return unindexedPath;
    }
    return String();
}


/**
 * @brief  Sorts directory lists to prioritize ones with non-numeric names, and
 *         prioritize numeric directories with values close to a target integer.
 */
class DirectoryComparator
{
public:
    DirectoryComparator(const int target) : target(target) { }

    int compareElements(juce::File first, juce::File second)
    {
        using juce::String;
        const String numericChars = "0123456789";
        juce::String firstNumber = first.getFileName()
                .initialSectionContainingOnly(numericChars);
        juce::String secondNumber = first.getFileName()
                .initialSectionContainingOnly(numericChars);
        // Prioritize non-numeric
        if (firstNumber.isEmpty())
        {
            return secondNumber.isEmpty() ? 0 : -1;
        }
        else if (secondNumber.isEmpty())
        {
            return 1;
        }
        const int firstDistance = abs(firstNumber.getIntValue() - target);
        const int secondDistance = abs(secondNumber.getIntValue() - target);
        return firstDistance - secondDistance;
    }

private:
    const int target;
};


// Searches icon directories for icon files not indexed by icon themes.
juce::String Icon::ThreadResource::getUnindexedIconPath
(const IconRequest& request)
{
    using juce::String;
    using juce::StringArray;
    using juce::File;
    StringArray searchPaths = iconDirectories;
    DirectoryComparator comp(request.size);
    for (int i = 0; i < searchPaths.size(); i++)
    {
        int insertIndex = i + 1;
        File iconDir(searchPaths[i]);
        juce::Array<File> subDirectories
                = iconDir.findChildFiles(File::findDirectories, false);
        subDirectories.sort(comp);
        for (const File& subDirectory : subDirectories)
        {
            File indexFile = subDirectory.getChildFile(indexFileName);
            if (indexFile.existsAsFile())
            {
                continue;
            }
            searchPaths.insert(insertIndex, subDirectory.getFullPathName());
            insertIndex++;
        }
    }

    for (const String& iconDir : searchPaths)
    {
        static const juce::StringArray iconExtensions
                = {".png", ".xpm", ".svg"};
        for (const String& ext : iconExtensions)
        {
            String iconPath = iconDir + String("/") + request.icon + ext;
            if (File(iconPath).existsAsFile())
            {
                return iconPath;
            }
        }
    }
    return String();
}
