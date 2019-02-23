#include "FocusingPagedList.h"

#ifdef JUCE_DEBUG
/* Print the full class name before all debug output: */
static const constexpr char* dbgPrefix = "FocusingPagedList::";
#endif

/* Default number of list items per page: */
static const constexpr unsigned int defaultItemsPerPage = 5;

/* Default list padding fraction: */
static const constexpr float defaultPaddingFraction = 0.02;

/* Default milliseconds to take when (un)focusing list content: */
static const constexpr unsigned int focusDuration = 300;

/* List item border width in pixels: */
static const constexpr unsigned int borderWidth = 4;

FocusingPagedList::FocusingPagedList() : listButtonHandler(*this) 
{
    setItemsPerPage(defaultItemsPerPage);
    setYPaddingFraction(defaultPaddingFraction);
}

/*
 * Gets the index of the selected list item.
 */
int FocusingPagedList::getSelectedIndex() const
{
    return selectedIndex;
}

/*
 * Sets the selected list index.
 */
void FocusingPagedList::setSelectedIndex(const int index)
{
    if(index >= 0 && index < getListSize() && index != selectedIndex)
    {
        selectedIndex = index;
        selectionChanged();
        refreshListContent(Layout::Transition::Type::toDestination,
                focusDuration);
        updateNavButtonVisibility(false);
    }
}

/*
 * Deselects the current selected list item, updating the component unless no 
 * list item was selected.
 */
void FocusingPagedList::deselect() 
{
    if(selectedIndex >= 0)
    {
        selectedIndex = -1;
        updateNavButtonVisibility(true);
        refreshListContent(Layout::Transition::Type::toDestination,
                focusDuration);
    }
}

/*
 * Creates or updates a list component, selecting how to update the component 
 * based on whether it is selected.
 */
juce::Component* FocusingPagedList::updateListItem(juce::Component* listItem,
        const unsigned int index)
{
    juce::Button* listButton = static_cast<juce::Button*>(listItem);

    if(selectedIndex == index)
    {
        listButton = updateSelectedListItem(listButton);
    }
    else
    {
        listButton = updateUnselectedListItem(listButton, index);
    }
    if(listItem == nullptr)
    {
        listButton->addListener(&listButtonHandler);
    }
    return static_cast<juce::Component*>(listButton);
}

/*
 * Gets the weight value used to determine the height of a particular list item.
 */ 
unsigned int FocusingPagedList::getListItemWeight
(const unsigned int index) const
{
    if(selectedIndex == index || selectedIndex < 0)
    {
        return 1;
    }
    return 0;
} 

/*
 * Handles list item selection.
 */
void FocusingPagedList::ListButtonHandler::buttonClicked(juce::Button* button)
{
    const int buttonIndex 
            = list.getListItemIndex(static_cast<juce::Component*>(button));
    /* The button must be a valid list item. */
    jassert(buttonIndex >= 0);

    if(buttonIndex != list.getSelectedIndex())
    {
        list.setSelectedIndex(buttonIndex);
    }
    else
    {
        list.deselect();
    }
}

