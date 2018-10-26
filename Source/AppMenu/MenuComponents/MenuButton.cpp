#define APPMENU_IMPLEMENTATION_ONLY
#include "IconLoader.h"
#include "AssetFiles.h"
#include "ComponentConfigFile.h"
#include "MenuButton.h"

/* Extra characters applied when calculating title width, defining title padding
   space relative to the font size. */
static const constexpr char* titleBuffer = "     ";

/*
 * Creates a new MenuButton component for a menu item.
 */
AppMenu::MenuButton::MenuButton(MenuItem menuItem) : menuItem(menuItem),
juce::Button(menuItem.getTitle())
{
    setWantsKeyboardFocus(false);
} 

/*
 * Checks if this button is the selected button in its folder.
 */
bool AppMenu::MenuButton::isSelected() const
{
    return selected;
}

/*
 * Selects or deselects this button.
 */
void AppMenu::MenuButton::setSelected(const bool isSelected)
{
    selected = isSelected;
    repaint();
}

/*
 * Gets the MenuItem that defines this button.
 */
AppMenu::MenuItem AppMenu::MenuButton::getMenuItem() const
{
    return menuItem;
}

/*
 * Gets the width of the button's title string.
 */
int AppMenu::MenuButton::getTitleWidth() const
{
    return textWidth;
}

/*
 * Recalculates and saves the menu button title bounds.
 */
void AppMenu::MenuButton::updateTitleBounds()
{
    titleBounds = findTitleBounds();
}

/*
 * Recalculates and saves the menu button icon bounds.
 */
void AppMenu::MenuButton::updateIconBounds()
{
    iconBounds = findIconBounds();
}

/*
 * Recreates and saves the font used to draw the menu button's title.
 */
void AppMenu::MenuButton::updateFont()
{
    titleFont = findTitleFont(titleBounds);
    textWidth = findTitleBGWidth(titleBounds, titleFont);
}

/*
 * Updates the title font to fit the current title bounds.
 */
juce::Font AppMenu::MenuButton::findTitleFont
(const juce::Rectangle<float>& titleBounds) const
{
    ComponentConfigFile config;
    return juce::Font(config.getFontHeight(ComponentConfigFile::smallText));
}

/*
 * Finds the width of the background area that will be drawn behind the button's
 * title.
 */
int AppMenu::MenuButton::findTitleBGWidth
(const juce::Rectangle<float>& titleBounds, const juce::Font& titleFont) const
{
    const juce::String title = getMenuItem().getTitle();
    const int width = titleFont.getStringWidth(title + titleBuffer);
    return width;
}

/*
 * Updates the component if necessary whenever its menu data changes.
 */
void AppMenu::MenuButton::dataChanged(MenuItem::DataField changedField)
{
    using DataField = MenuItem::DataField;
    if(changedField == DataField::title)
    {
        updateTitleBounds();
        repaint();
    }
    else if(changedField == DataField::icon)
    {
        loadIcon();
    }
}


/*
 * Reloads the button icon image if necessary.
 */
void AppMenu::MenuButton::loadIcon()
{
    using juce::Image;
    if(!iconBounds.isEmpty())
    {
        icon = AssetFiles::loadImageAsset("appIcons/default.png");
        IconLoader iconThread;

        iconThread.loadIcon(getMenuItem().getIconName(),
                iconBounds.toNearestInt().getWidth(),
        [this](Image iconImg)
        {
            icon = iconImg;
            repaint();
        });
    }
}

/*
 * Calls AppMenu::MenuButton::menuButtonResized() and re-calculates title and 
 * icon layout whenever the button's bounds change.
 */
void AppMenu::MenuButton::resized()
{
    menuButtonResized();
    updateTitleBounds();
    updateIconBounds();
    updateFont();
    if(icon.isNull())
    {
        loadIcon();
    }
}

/*
 * Draws the button object.
 */
void AppMenu::MenuButton::paintButton
(juce::Graphics &g, bool isMouseOverButton, bool isButtonDown)
{
    using juce::Rectangle;
    using juce::RectanglePlacement;
    Rectangle<int> border = getLocalBounds();
    if ((iconBounds.isEmpty() || titleBounds.isEmpty()) && 
            !border.isEmpty())
    {
        resized();
    }
    g.setColour(findColour(isSelected() ?
            selectionColourId : backgroundColourId));
    if (shouldFillBackground())
    {
        g.fillRect(border);
    }
    else
    {
        Rectangle<float> textOval = titleBounds.withSizeKeepingCentre
                (textWidth, titleBounds.getHeight());
        g.fillRoundedRectangle(textOval, textOval.getHeight() / 6);
    }
    // Draw icon:
    g.setOpacity(1);
    g.drawImageWithin(icon, iconBounds.getX(), iconBounds.getY(),
            iconBounds.getWidth(), iconBounds.getHeight(),
            RectanglePlacement::xMid | RectanglePlacement::yTop, false);
    // Draw title:
    g.setColour(findColour(textColourId));
    g.setFont(titleFont);
    g.drawText(getMenuItem().getTitle(), titleBounds, 
            getTextJustification(), true);
    if (shouldDrawBorder())
    {
        g.setColour(findColour(borderColourId));
        g.drawRect(border, 2);
    }
}
