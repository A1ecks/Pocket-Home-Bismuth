#include "AdvancedSettingsPage.h"
#include "../PocketHomeApplication.h"

AdvancedSettingsPage::AdvancedSettingsPage() :
PageComponent("AdvancedSettingsPage",{},true),
titleLabel("settings", "Advanced Settings"),
setPasswordButton("Set your password"),
removePasswordButton("Remove your password"),
personalizeButton("Personalize your homepage"),
dateTimeButton("Date and time"),
inputOptionsButton("Input settings"),
personalizePage(),
prevArrow("pageUpIcon.svg"),
nextArrow("pageDownIcon.svg")
{
    setColour(backgroundColourId,Colour(0xffd23c6d));
    std::vector<Button*> allButtons = getButtonList(true);
    for (Button* button : allButtons)
    {
        button->addListener(this);
    }
    titleLabel.setJustificationType(Justification::centred);
    reloadLayout();
}

AdvancedSettingsPage::~AdvancedSettingsPage() { }

/**
 * Get pointers to all buttons on the page
 */
std::vector<Button*>
AdvancedSettingsPage::getButtonList(bool includeAll)
{
    std::vector<Button*> buttonList;
    buttonList.push_back(&personalizeButton);
    buttonList.push_back(&setPasswordButton);
    if (setPasswordPage.hasPassword() || includeAll)
    {
        buttonList.push_back(&removePasswordButton);
    }
    buttonList.push_back(&dateTimeButton);
    buttonList.push_back(&inputOptionsButton);
    if (includeAll)
    {
        buttonList.push_back(&prevArrow);
        buttonList.push_back(&nextArrow);
    }
    return buttonList;
}

/**
 * Reloads the page layout settings.
 */
void
AdvancedSettingsPage::reloadLayout()
{
    setPasswordButton.setButtonText(setPasswordPage.hasPassword() ?
            "Change your password" : "Set your password");
    std::vector<Button*> buttons = getButtonList();
    if (buttonIndex >= buttons.size() || buttonIndex < 0)
    {
        buttonIndex = 0;
    }
    std::vector<GridLayoutManager::RowLayoutParams> layout = {
        {4,
            {
                {&titleLabel, 1}
            }},
        {1,
            {
                {buttonIndex > 0 ? &prevArrow : nullptr, 1}
            }}
    };

    for (int i = buttonIndex; i < (buttonIndex + buttonsPerPage); i++)
    {
        layout.push_back({3,
            {
                {(i < buttons.size()) ? buttons[i] : nullptr, 1}
            }});
    }
    layout.push_back({1,
        {
            {buttonIndex + buttonsPerPage < buttons.size() ? &nextArrow : nullptr, 1}
        }});
    updateLayout(layout);
}

/**
 * Reloads page layout whenever the page becomes visible.
 */
void
AdvancedSettingsPage::visibilityChanged()
{
    if (isVisible())
    {
        reloadLayout();
    }
}

/**
 * Handle button clicks to open menu pages, close this page, or 
 * scroll the list of page buttons.
 */
void
AdvancedSettingsPage::pageButtonClicked(Button * button)
{
    PageStackComponent& mainStack = PocketHomeApplication::getInstance()
            ->getMainStack();
    if (button == &setPasswordButton)
    {
        mainStack.pushPage(&setPasswordPage,
                PageStackComponent::kTransitionTranslateHorizontal);
    }
    else if (button == &removePasswordButton
             && removePasswordPage.hasPassword())
    {
        mainStack.pushPage(&removePasswordPage,
                PageStackComponent::kTransitionTranslateHorizontal);
    }
    else if (button == &personalizeButton)
    {
        mainStack.pushPage(&personalizePage,
                PageStackComponent::kTransitionTranslateHorizontal);
    }
    else if (button == &nextArrow)
    {
        buttonIndex += buttonsPerPage;
        reloadLayout();
    }
    else if (button == &prevArrow)
    {
        buttonIndex -= buttonsPerPage;
        reloadLayout();
    }
    else if (button == &inputOptionsButton)
    {
        mainStack.pushPage(&inputPage,
                PageStackComponent::kTransitionTranslateHorizontal);
    }
    else if (button == &dateTimeButton)
    {
        mainStack.pushPage(&dateTimePage,
                PageStackComponent::kTransitionTranslateHorizontal);
    }
}






