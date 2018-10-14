#pragma once
#include "AppConfigFile.h"
#include "AppMenuPopupEditor.h"

/** 
 * @file  NewConfigItemEditor.h
 * 
 * @brief  Creates a pop-up editor component that allows the user to add a new 
 *         application shortcut menu item to the AppConfigFile.  
 * 
 * After entering data and pressing the confirm button, the new favorite 
 * application link is added to the menu and saved by AppConfigFile.
 *
*/

class NewConfigItemEditor : public AppMenuPopupEditor {
public:
    /**
     * @brief  Creates the new editor component.
     *
     * @param parentFolder  The folder menu item the new component will be
     *                      inserted into.
     *
     * @param isFolder      If true, the editor creates a folder item, if false,
     *                      it creates an application shortcut item.
     *
     * @param insertIndex   An optional index value where the new item will be
     *                      inserted, By default it will be added to the start
     *                      of the folder.
     *
     * @param onConfirm     An optional callback routine to run after creating 
     *                      the new application entry.
     */
    NewConfigItemEditor(const AppMenuItem& parentFolder,
            const bool isFolder,
            const int insertIndex = 0,
            const std::function<void() > onConfirm = [](){});
protected:
    /**
     * @brief  Creates the new application menu item and inserts it into the
     *         parent folder item.
     */
    virtual void commitEdits();

private:
    /* Folder where the new menu item will be inserted. */
    AppMenuItem parentFolder;

    /* Index in the folder where the new menu item will be inserted. */
    int insertIndex;
};