#include <limits>
#include "IconTheme.h"

IconTheme::IconTheme(File themeDir) :
path(themeDir.getFullPathName()),
cacheFile(themeDir.getFullPathName() + cacheFileName)
{
    if (!themeDir.isDirectory())
    {
        DBG("IconTheme::IconTheme:: Theme directory does not exist!");
        path = String();
        return;
    }

    File themeIndex(themeDir.getFullPathName() + indexFileName);
    if (!themeIndex.existsAsFile())
    {
        DBG("IconTheme::IconTheme:: Theme index does not exist!");
        path = String();
        return;
    }
    StringArray fileLines = StringArray::fromLines
            (themeIndex.loadFileAsString());

    static const String themeSectionName = "Icon Theme";
    String sectionName;
    IconDirectory currentDir;

    //Map each data key string to a function that saves the key's value
    static const std::map<String,
            std::function<void(IconTheme*, String&, IconDirectory&) >> assnFns ={
        {"Name",
         [](IconTheme* self, String& val, IconDirectory & dir)
            {
                self->name = val;
            }},
        {"Comment",
         [](IconTheme* self, String& val, IconDirectory & dir)
            {
                self->comment = val;
            }},
        {"Inherits",
         [](IconTheme* self, String& val, IconDirectory & dir)
            {
                self->inheritedThemes
                        = StringArray::fromTokens(val, ",", "");
            }},
        {"Hidden",
         [](IconTheme* self, String& val, IconDirectory & dir)
            {
                self->hidden = (val == "true");
            }},
        {"Example",
         [](IconTheme* self, String& val, IconDirectory & dir)
            {
                self->example = val;
            }},
        {"Size",
         [](IconTheme* self, String& val, IconDirectory & dir)
            {
                dir.size = val.getIntValue();
            }},
        {"Scale",
         [](IconTheme* self, String& val, IconDirectory & dir)
            {
                dir.scale = val.getIntValue();
            }},
        {"MaxSize",
         [](IconTheme* self, String& val, IconDirectory & dir)
            {
                dir.maxSize = val.getIntValue();
            }},
        {"MinSize",
         [](IconTheme* self, String& val, IconDirectory & dir)
            {
                dir.minSize = val.getIntValue();
            }},
        {"Threshold",
         [](IconTheme* self, String& val, IconDirectory & dir)
            {
                dir.threshold = val.getIntValue();
            }},
        {"Context",
         [](IconTheme* self, String& val, IconDirectory & dir)
            {
                static const std::map<String, Context> contexts ={
                    {"Actions", actionsCtx},
                    {"Animations", animationsCtx},
                    {"Applications", applicationsCtx},
                    {"Categories", categoriesCtx},
                    {"Devices", devicesCtx},
                    {"Emblems", emblemsCtx},
                    {"Emotes", emotesCtx},
                    {"International", internationalCtx},
                    {"MimeTypes", mimeTypesCtx},
                    {"Places", placesCtx},
                    {"Status", statusCtx}
                };
                auto contextIter = contexts.find(val);
                if (contextIter != contexts.end())
                {
                    dir.context = contextIter->second;
                }
            }},
        {"Type",
         [](IconTheme* self, String& val, IconDirectory & dir)
            {
                static const std::map<String, SizeType> types ={
                    {"Fixed", fixedType},
                    {"Scalable", scalableType},
                    {"Threshold", thresholdType}
                };
                auto typeIter = types.find(val);
                if (typeIter != types.end())
                {
                    dir.type = typeIter->second;
                }
            }}
    };

    for (const String& line : fileLines)
    {
        if (line.startsWithChar('[') && line.endsWithChar(']'))
        {
            if (sectionName.isNotEmpty() && sectionName != themeSectionName)
            {
                currentDir.path = sectionName;
                directories[sectionName] = currentDir;
                currentDir = IconDirectory();
            }
            sectionName = line.substring(1, line.length() - 1);
        }
        else if (line.isNotEmpty())
        {
            int divider = line.indexOfChar('=');
            if (divider == -1)
            {
                jassertfalse;
                continue;
            }
            String key = line.substring(0, divider);
            auto keyActionIter = assnFns.find(key);
            if (keyActionIter != assnFns.end())
            {
                String val = line.substring(divider);
                keyActionIter->second(this, val, currentDir);
            }
        }
    }
    if(sectionName.isNotEmpty())
    {
        currentDir.path = sectionName;
        directories[sectionName] = currentDir;
    }
}

/**
 * Checks if this object represents a valid icon theme.
 */
bool IconTheme::isValidTheme()
{
    return path.isNotEmpty();
}

/*
 * Finds the path of an icon within the theme.  The caller is responsible
 * for searching within all inherited themes if the icon is not found.
 */
String IconTheme::lookupIcon(String icon, int size, Context context, int scale)
{
    if(!isValidTheme())
    {
        return String();
    }
    
    Array<IconDirectory> searchDirs;

    std::map<String, String> cacheMatches;
    if(useCache)
    {
        cacheMatches = cacheFile.lookupIcon(icon);
    }
    if (!cacheMatches.empty())
    {
        for (auto it = cacheMatches.begin(); it != cacheMatches.end(); it++)
        {
            searchDirs.add(directories[it->first]);
        }
    }
    else if (cacheFile.isValidCache() && useCache)
    {
        //cache is valid and doesn't contain the icon, no need to keep looking
        return String();
    }
    else
    {
        for (auto dirIter = directories.begin(); dirIter != directories.end();
             dirIter++)
        {
            if (context == unknownCtx || context == dirIter->second.context)
            {
                if(dirIter->second.path.isEmpty())
                {
                    DBG(dirIter->first << " missing path ");
                }
                searchDirs.add(dirIter->second);
            }
        }
    }

    DirectoryComparator comp(size, scale);
    searchDirs.sort(comp);

    for (const IconDirectory& dir : searchDirs)
    {
        String filePath = path + "/" + dir.path + "/" + icon;
        try
        {
            String extension = cacheMatches.at(dir.path);
            if(File(filePath + extension).existsAsFile())
            {
                return filePath + extension;
            }
            else
            {
                DBG("Bad cache result:" << filePath << extension);
            }
        }
        catch (std::out_of_range e)
        {
            //file extensions not found, continue on to check all possibilities
        }
        static const StringArray extensions = {".png", ".svg", ".xpm"};
        for (const String& ext : extensions)
        {
            File iconFile(filePath + ext);
            if (iconFile.existsAsFile())
            {
                return iconFile.getFullPathName();
            }
        }
    }
    return String();
}

/*
 * Gets the name of the icon theme.
 */
String IconTheme::getName()
{
    return name;
}

/*
 * Gets a short comment describing the icon theme.
 */
String IconTheme::getComment()
{
    return comment;
}

/*
 * Gets the names of all themes inherited by this icon theme.  When findIcon
 * doesn't locate a requested icon, all inherited themes should be searched.
 */
StringArray IconTheme::getInheritedThemes()
{
    return inheritedThemes;
}

/*
 * Checks if this theme should be displayed to the user in theme lists.
 */
bool IconTheme::isHidden()
{
    return hidden;
}

/*
 * Gets the name of an icon to use as an example of this theme.
 */
String IconTheme::getExampleIcon()
{
    return example;
}

/*
 * Compares two icon directories by their distance from a target size 
 * and scale.
 */
int IconTheme::DirectoryComparator::compareElements
(IconDirectory first, IconDirectory second)
{
    bool firstMatches = directoryMatchesSize(first);
    bool secondMatches = directoryMatchesSize(second);

    if (firstMatches)
    {
        if (secondMatches)
        {
            return 0;
        }
        return -1;
    }
    else if (secondMatches)
    {
        return 1;
    }
    return directorySizeDistance(first) - directorySizeDistance(second);
}

/*
 * Checks if an icon directory is suitable for a given size.
 */
bool IconTheme::DirectoryComparator::directoryMatchesSize
(const IconDirectory& subdir)
{
    if (scale != subdir.scale)
    {
        return false;
    }
    switch (subdir.type)
    {
        case fixedType:
            return size == subdir.size;
        case scalableType:
            return size >= subdir.minSize && size <= subdir.maxSize;
        case thresholdType:
            return abs(size - subdir.size) < subdir.threshold;
    }
}

/*
 * Finds the distance between an icon directory's icon size and a 
 * given icon size and scale.
 */
int IconTheme::DirectoryComparator::directorySizeDistance
(const IconDirectory& subdir)
{
    switch (subdir.type)
    {
        case fixedType:
            return abs(subdir.size * subdir.scale - size * scale);
        case scalableType:
            if (size * scale < subdir.minSize * subdir.scale)
            {
                return subdir.minSize * subdir.scale - size * scale;
            }
            if (size * scale > subdir.maxSize * subdir.scale)
            {
                return size * scale - subdir.maxSize * subdir.scale;
            }
            return 0;
        case thresholdType:
            if (size * scale < (subdir.size - subdir.threshold) * subdir.scale)
            {
                return subdir.minSize * subdir.scale - size * scale;
            }
            if (size * scale > (subdir.size + subdir.threshold) * subdir.scale)
            {
                return size * scale - subdir.maxSize * subdir.scale;
            }
            return 0;
    }
}
