#pragma once
#include "ConfigurableImageButton.h"
#include "AppMenuComponent.h"

/**
 * @file PagedAppMenu.h
 * 
 * @brief PagedAppMenu displays all applications in a grid, spreading them across
 * multiple pages.  
 * 
 * Only one page of one folder is visible at once. Touch
 * navigation buttons or hardware keys can be used to open or close folders,
 * and switch between folder pages.
 */

class PagedAppMenu : public AppMenuComponent, public Button::Listener
{
public:
    /**
     * @param loadingSpinner   Reference to an overlay spinner that sits over
     *                         the PageComponent holding this AppMenuComponent.
     */
    PagedAppMenu(OverlaySpinner& loadingSpinner);

    virtual ~PagedAppMenu() { }

    /**
     * Update navigation buttons when the menu changes size.
     */
    void menuResized() override;

private:
    /**
     * Uses key events to navigate through the menu.
     * @param key
     * @param activeFolder
     * @return true iff the key press was used.
     */
    bool folderKeyPressed
    (const KeyPress& key, AppMenuFolder* activeFolder) override;

    /**
     * Create a folder component object from a folder menu item.
     * @param folderItem provides folder menu items
     * @param buttonMap is used by the folder to recycle menu buttons
     */
    AppMenuFolder* createFolderObject(
            AppMenuItem::Ptr folderItem,
            std::map<String, AppMenuButton::Ptr>& buttonMap) override;

    /**
     * Return the bounds where the given folder should be placed in the menu.
     * @param folder
     * @param folderIndex
     * @return 
     */
    virtual Rectangle<int> updateFolderBounds
    (const AppMenuFolder* folder, int folderIndex) override;

    /**
     * Handles navigation button controls
     * @param button should be pageLeft, pageRight, or closeFolderBtn
     */
    void buttonClicked(Button* button) override;

    //folder navigation key bindings
    static const String pageLeftBinding;
    static const String pageRightBinding;
    //navigation buttons
    ConfigurableImageButton closeFolderBtn;
    ConfigurableImageButton pageLeft;
    ConfigurableImageButton pageRight;
};