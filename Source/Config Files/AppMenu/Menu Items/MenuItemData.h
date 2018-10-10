#pragma once
#include "JuceHeader.h"
#include "MenuIndex.h"

/**
 * @file  MenuItemData.h
 *
 * @brief  Reads and writes properties of a menu item. 
 */
class MenuItemData : public juce::ReferenceCountedObject
{
public:
    /* Allow AppJSON to update stored indices when moving menu items. */
    friend class AppJSON;
    
    /* Custom reference-counting pointer object type. */
    typedef juce::ReferenceCountedObjectPtr<MenuItemData> Ptr;

    /**
     * @brief  Creates a menu data object for an item in the application menu.
     *
     * @param index  The object's index within the menu.
     */
    MenuItemData(const MenuIndex& index);
    
    virtual ~MenuItemData() { }

    /**
     * @brief  Gets the menu item's index within the menu.
     *
     * @return  The menu item's index.
     */
    const MenuIndex& getIndex() const;

    /**
     * @brief  Accesses the menu data lock.
     *
     * @return  The lock used to control access to menu item data.
     */
    const juce::ReadWriteLock& getLock();

    /**
     * @brief  Checks if this menu item represents a folder within the menu.
     *
     * @return  Whether this menu item opens a new menu folder.
     */
    virtual bool isFolder() const = 0;

    /**
     * @brief  Gets the menu item's displayed title.
     *
     * @return  The displayed title string.
     */
    virtual juce::String getTitle() const = 0;

    /**
     * @brief  Sets the menu item's displayed title.
     *
     * @param title  The new title string to display.
     */
    virtual void setTitle(const juce::String& title) = 0;

    /**
     * @brief  Gets the name or path use to load the menu item's icon file.
     *
     * @return  The name or path of the icon.
     */
    virtual juce::String getIconName() const = 0;

    /**
     * @brief  Sets the name or path used to load the menu item's icon file.
     *
     * @param iconName  The new icon name or path.
     */
    virtual void setIconName(const juce::String& iconName) = 0;

    /**
     * @brief  Gets the application categories connected to this menu item.
     *
     * @return  Any category strings assigned to this menu item.
     */
    virtual juce::StringArray getCategories() const = 0;

    /**
     * @brief  Sets the application categories connected to this menu item.
     *
     * @param categories  The new set of category strings to assign to this menu
     *                    item.
     */
    virtual void setCategories(const juce::StringArray& categories) = 0;

    /**
     * @brief  Gets the menu item's application launch command.
     *
     * @return  The launch command string, or the empty string if the menu item
     *          does not launch an application.
     */
    virtual juce::String getCommand() const = 0;

    /**
     * @brief  Sets the menu item's application launch command.
     *
     * @param newCommand  The new command string to run when this menu item is
     *                    clicked.
     */
    virtual void setCommand(const juce::String& newCommand) = 0;

    /**
     * @brief  Checks if this menu item launches an application in a new
     *         terminal window.
     *
     * @return  True if and only if the menu item has a launch command it should
     *          run in a new terminal window
     */
    virtual bool getLaunchedInTerm() const = 0;

    /**
     * @brief  Sets if this menu item runs its command in a new terminal window.
     *
     * @param termLaunch  True to run any launch command assigned to this
     *                    menu item within a new terminal window.
     */
    virtual void setLaunchedInTerm(const bool termLaunch) = 0;

    /**
     * @brief  Deletes this menu item data from its source.
     */
    virtual void deleteFromSource() = 0;

    /**
     * @brief  Writes all changes to this menu item back to its data source.
     */
    virtual void updateSource() = 0;

    /**
     * @brief  Checks if this menu item can be moved within its parent folder.
     *
     * @param offset  The amount to offset the menu item index at its greatest
     *                depth.
     *
     * @return        True if and only if the menu item can be moved, and the 
     *                offset is valid.
     */
    virtual bool canMoveIndex(const int offset) = 0;

    /**
     * @brief  Attempts to move this menu item within its parent folder.
     *
     * @param offset  The amount to offset the menu item index.
     *
     * @return        True if the menu item was moved, false if it couldn't be
     *                moved by the given offset value.
     */
    virtual bool moveIndex(const int offset) = 0;
    
    /**
     * @brief  Gets an appropriate title to use for a deletion confirmation 
     *         window.
     *
     * @return  A localized confirmation title string.
     */
    virtual juce::String getConfirmDeleteTitle() const = 0;

    /**
     * @brief  Gets appropriate descriptive text for a deletion confirmation 
     *         window.
     *
     * @return  A localized confirmation description string.
     */
    virtual juce::String getConfirmDeleteMessage() const = 0;

    /**
     * @brief  Gets an appropriate title to use for a menu item editor.
     *
     * @return  A localized editor title string.
     */
    virtual juce::String getEditorTitle() const = 0;

    /**
     * @brief  Menu item data fields that may or may not be editable.
     */
    enum class DataField
    {
        title,
        icon,
        command,
        categories,
        termLaunchOption
    };

    /**
     * @brief  Checks if a data field within this menu item can be edited.
     *
     * @param dataField  The type of data being checked.
     *
     * @return           True if and only if the data field is editable.
     */
    virtual bool isEditable(const DataField dataField) = 0;

    /**
     * @brief  Gets the number of menu items in the folder opened by this menu
     *         item.
     *
     * @return  The number of folder items, or zero if this menu item does not
     *          open a folder.
     */
    virtual int getFolderSize() = 0;

protected:
    /**
     * @brief  Updates the menu item data's saved index value.
     *
     * Use this to update the index value when menu items are moved.
     *
     * @param depth   The level in the menu tree where the index should be
     *                moved.
     *
     * @param offset  The amount to offset the menu item index.
     */
    void updateIndex(const int depth, const int offset);

private:
    MenuIndex index;

    /* Prevents unsafe concurrent access to menu item data. */
    const juce::ReadWriteLock dataLock;
    
    JUCE_DECLARE_NON_COPYABLE(MenuItemData);
};
