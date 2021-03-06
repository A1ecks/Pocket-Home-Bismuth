#ifndef APPMENU_IMPLEMENTATION
    #error File included directly outside of AppMenu implementation.
#endif
#pragma once
/**
 * @file  AppMenu_CategoryEditor.h
 *
 * @brief  Provides a user interface for editing a list of application category
 *         strings.
 */

#include "Widgets_PopupEditor.h"
#include "Widgets_ListEditor.h"
#include "Locale_TextUser.h"

namespace AppMenu { class CategoryEditor; }

/**
 * @brief  Appears above other components when the user chooses the "Edit
 *         Categories" option in a PopupEditor, allowing the user to add,
 *         delete, or edit a menu item's list of categories.
 *
 *  Double-clicking on items in the list makes them editable, clicking the 'X'
 * button to the right of a list item deletes it, and clicking the '+' button
 * adds text from the text editor component at the bottom to the list. When
 * closing the editor, the user may either choose to confirm and save changes,
 * or cancel and discard changes.
 */
class AppMenu::CategoryEditor : public Widgets::PopupEditor,
    public Locale::TextUser
{
public:
    /**
     * @brief  Creates a new category editor, setting the initial categories
     *         and the confirmation callback function.
     *
     * @param categories  An initial list of editable category strings to show.
     *
     * @param onConfirm   A callback to run if the user presses the confirm
     *                    button. The final list of edited categories will be
     *                    passed to it as its only parameter.
     */
    CategoryEditor(juce::StringArray categories,
            std::function<void(juce::StringArray) > onConfirm);

    virtual ~CategoryEditor() { }

private:
    // Holds and allows updates to the list of categories:
    Widgets::ListEditor categoryList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CategoryEditor)
};
