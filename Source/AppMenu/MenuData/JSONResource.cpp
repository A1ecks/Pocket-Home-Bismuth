#include "DesktopEntryData.h"
#include "JSONResource.h"

/* SharedResource object key */
const juce::Identifier AppMenu::JSONResource::resourceKey = "JSONResource";
/* JSON configuration file name */
static const constexpr char * jsonFilename = "apps.json";

/* Menu item JSON keys: */
static const juce::Identifier titleKey("name");
static const juce::Identifier iconKey("icon");
static const juce::Identifier commandKey("command");
static const juce::Identifier launchInTermKey("launch in terminal");
static const juce::Identifier categoryKey("categories");
static const juce::Identifier folderItemKey("folder items");

/* Localized text keys: */
static const constexpr char * remove_APP = "remove_APP";
static const constexpr char * from_favorites = "from_favorites";
static const constexpr char * will_remove_link = "will_remove_link";
static const constexpr char * edit_app = "edit_app";

AppMenu::JSONResource::JSONResource() : Config::FileResource
(resourceKey, jsonFilename)
{
    using namespace juce;
    // Load menu:
    var rootFolder = initProperty<var>(folderItemKey);
    rootFolderItem = MenuItem(new ConfigItemData(rootFolder));
    loadJSONData();   
}

/*
 * Gets a menu item representing the root folder of the application menu.
 */
AppMenu::MenuItem AppMenu::JSONResource::getRootFolderItem() const
{
    return rootFolderItem;
}

/**
 * @brief  Recursively copies a menu item and all of its child folder items
 *         into a juce::var object.
 *
 * @param menuItem  A non-null item located in the application menu.
 *
 * @return          All menu item data packaged in an object var. 
 */
static juce::var itemToVar(const AppMenu::MenuItem& menuItem)
{
    using namespace juce;
    DynamicObject::Ptr itemObject = new DynamicObject();
    itemObject->setProperty(titleKey, menuItem.getTitle());
    itemObject->setProperty(iconKey, menuItem.getIconName());
    if(menuItem.isFolder())
    {
        int configChildCount = menuItem.getMovableChildCount();
        if(configChildCount > 0)
        {
            Array<var> folderItems;
            for(int i = 0; i < configChildCount; i++)
            {
                AppMenu::MenuItem folderItem = menuItem.getFolderItem(i);
                if(!folderItem.isNull())
                {
                    folderItems.add(itemToVar(folderItem));
                }
            }
            itemObject->setProperty(folderItemKey, folderItems);
        }
        StringArray categories = menuItem.getCategories();
        if(categories.isEmpty())
        {
            itemObject->setProperty(categoryKey, categories);
        }
    }
    else
    {
        itemObject->setProperty(commandKey, menuItem.getCommand());
        itemObject->setProperty(launchInTermKey, menuItem.getLaunchedInTerm());
    }
    return var(itemObject);
}

/*
 * Copies all menu data back to the JSON configuration file.
 */
void AppMenu::JSONResource::writeDataToJSON()
{
    using namespace juce;
    MenuItem rootItem = getRootFolderItem();
    int numItems = rootItem.getMovableChildCount();
    if(numItems > 0)
    {
        Array<var> rootFolder;
        for(int i = 0; i < numItems; i++)
        {
            MenuItem folderItem = rootItem.getFolderItem(i);
            if(!folderItem.isNull())
            {
                rootFolder.add(itemToVar(folderItem));
            }
        }
        updateProperty<Array<var>>(folderItemKey, rootFolder);
    }
}

/**
 * Adds a new menu item to the list of menu items.
 */
AppMenu::MenuItem AppMenu::JSONResource::addMenuItem(
        const juce::String& title, 
        const juce::String& icon,
        const juce::String& command,
        const bool launchInTerm,
        const juce::StringArray& categories,
        MenuItem& parentFolder,
        const int index)
{
    if(parentFolder.isNull() || index < 0 
            || index > parentFolder.getMovableChildCount())
    {
        return MenuItem();
    }
    using namespace juce;
    var voidVar;
    ItemData::Ptr newData = new ConfigItemData(voidVar);
    newData->setTitle(title);
    newData->setIconName(icon);
    newData->setCommand(command);
    newData->setLaunchedInTerm(launchInTerm);
    if(!categories.isEmpty())
    {
        newData->setCategories(categories);
        static_cast<ConfigItemData*>(newData.get())->loadDesktopEntryItems();
    }
    MenuItem newItem(newData);
    if(!insertChild(parentFolder, newItem, index))
    {
        return MenuItem();
    }
    return newItem;
}

/*
 * Recursively creates a menu item and all its child folder items.
 */
AppMenu::JSONResource::ConfigItemData::ConfigItemData(juce::var& menuData) :
Localized("ConfigItemData")
{
    using namespace juce;
    pendingCallbackID = 0;
    if(menuData.isVoid())
    {
        return;
    }
    var folderItems;
    if(menuData.isObject())
    {
        title = menuData.getProperty(titleKey, String());
        iconName = menuData.getProperty(iconKey, String());
        command = menuData.getProperty(commandKey, String());
        launchInTerm = menuData.getProperty(launchInTermKey, false);
        var categoryVar = menuData.getProperty(categoryKey, var());
        if(categoryVar.isArray())
        {
            for(const var& category : *categoryVar.getArray())
            {
                categories.add(category.operator String());
            }
        }
        folderItems = menuData.getProperty(folderItemKey, var());
    }
    else if(menuData.isArray())
    {
        folderItems = menuData;
    }
    if(folderItems.isArray())
    {
        for(var& folderItem : *folderItems.getArray())
        {
            insertChild(new ConfigItemData(folderItem), getFolderSize());
        }
    }
    movableChildCount = getFolderSize();
    loadDesktopEntryItems();
}

/*
 * Cancels any desktop entry menu items waiting to load before destroying this 
 * menu item.
 */
AppMenu::JSONResource::ConfigItemData::~ConfigItemData()
{
    if(pendingCallbackID.get())
    {
        DesktopEntryLoader entryLoader;
        entryLoader.clearCallback(pendingCallbackID.get());
    }
}

/*
 * Gets the menu item's displayed title.
 */
juce::String 
AppMenu::JSONResource::ConfigItemData::getTitle() const
{
    return title;
}
/*
 * Gets the name or path used to load the menu item's icon file.
 */
juce::String 
AppMenu::JSONResource::ConfigItemData::getIconName() const
{
    return iconName;
}

/*
 * Gets the menu item's application launch command.
 */
juce::String 
AppMenu::JSONResource::ConfigItemData::getCommand() const
{
    return command;
}

/*
 * Checks if this menu item launches an application in a new terminal window.
 */
bool 
AppMenu::JSONResource::ConfigItemData::getLaunchedInTerm() const
{
    return launchInTerm;
}

/*
 * Gets the application categories used to load this item's desktop entry child 
 * folder items.
 */
juce::StringArray 
AppMenu::JSONResource::ConfigItemData::getCategories() const
{
    return categories;
}

/*
 * Sets the menu item's displayed title.
 */
void 
AppMenu::JSONResource::ConfigItemData::setTitle(const juce::String& title)
{
    this->title = title;
}

/*
 * Sets the name or path used to load the menu item's icon file.
 */
void AppMenu::JSONResource::ConfigItemData::setIconName
(const juce::String& iconName)
{
    this->iconName = iconName;
}

/*
 * Sets the menu item's application launch command.
 */
void AppMenu::JSONResource::ConfigItemData::setCommand
(const juce::String& command)
{
    this->command = command;
}

/*
 * Sets if this menu item runs its command in a new terminal window.
 */
void AppMenu::JSONResource::ConfigItemData::setLaunchedInTerm
(const bool launchInTerm)
{
    this->launchInTerm = launchInTerm;
}

/*
 * Sets the application categories used to load this item's desktop entry child 
 * folder items.
 */
void AppMenu::JSONResource::ConfigItemData::setCategories
(const juce::StringArray& categories)
{
    this->categories = categories;
}

/*
 * Gets the number of folder items held by this menu item that can be reordered.
 */
int 
AppMenu::JSONResource::ConfigItemData::getMovableChildCount() const
{
    return movableChildCount;
}

/*
 * Writes all changes to this menu item back to its data source.
 */
void AppMenu::JSONResource::ConfigItemData::saveChanges()
{
    JSONWriter jsonWriter;
    jsonWriter.writeChanges();
}

/*
 * Gets an appropriate title to use for a deletion confirmation window.
 */
juce::String 
AppMenu::JSONResource::ConfigItemData::getConfirmDeleteTitle() const
{
    return localeText(remove_APP) 
            + getTitle() + localeText(from_favorites);
}

/*
 * Gets appropriate descriptive text for a deletion confirmation window.
 */
juce::String 
AppMenu::JSONResource::ConfigItemData::getConfirmDeleteMessage() const
{
    return localeText(will_remove_link);
}

/*
 * Gets an appropriate title to use for a menu item editor.
 */
juce::String 
AppMenu::JSONResource::ConfigItemData::getEditorTitle() const
{
    return localeText(edit_app);
}

/*
 * Checks if a data field within this menu item can be edited.
 */
bool 
AppMenu::JSONResource::ConfigItemData::isEditable
(const DataField dataField) const
{
    switch(dataField)
    {
        case DataField::categories:
            return getFolderSize() > 0;
        case DataField::command:
        case DataField::termLaunchOption:
            return getFolderSize() == 0;
        case DataField::icon:
        case DataField::title:
            return true;
    }
    return false;
}

/*
 * Loads all desktop entry child menu items defined by the menu item's category
 * list.
 */
void AppMenu::JSONResource::ConfigItemData::loadDesktopEntryItems()
{
    using namespace juce;
    if(!categories.isEmpty() && pendingCallbackID.get() == 0
            && getFolderSize() <= getMovableChildCount())
    {
        DesktopEntryLoader entryLoader;
        pendingCallbackID = entryLoader.waitUntilLoaded([this]()
        {
            DesktopEntryLoader entryLoader;
            Array<DesktopEntry> entries 
                    = entryLoader.getCategoryListEntries(categories);
            for(const DesktopEntry& entry : entries)
            {
                MenuItemData::Ptr newItem = new DesktopEntryItemData(entry);
                insertChild(newItem, getFolderSize());
            }
            pendingCallbackID = 0;
        });
    }
}

/*
 * Removes this menu item from JSONResource's config file.
 */
void AppMenu::JSONResource::ConfigItemData::deleteFromSource()
{
    // This should only be called after a call to remove()
    jassert(getIndex() == -1 && getParentFolder() == nullptr);
    saveChanges();
}

/*
 * Writes all config-defined menu data back to the JSON file.
 */
void AppMenu::JSONResource::ConfigItemData::JSONWriter::writeChanges()
{
    auto appJSON = getWriteLockedResource();
    appJSON->writeDataToJSON();
}
