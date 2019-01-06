#include "PowerPage.h"
#include "Layout_Transition_Animator.h"
#include "SystemCommands.h"
#include "Utils.h"
#include <map>


/* Localized object class key: */
static const juce::Identifier localeClassKey = "PowerPage";

/* Localized text keys: */
static const juce::Identifier shutdownTextKey      = "shutdown";
static const juce::Identifier rebootTextKey        = "reboot";
static const juce::Identifier sleepTextKey         = "sleep";
static const juce::Identifier buildTextKey         = "build";
static const juce::Identifier versionTextKey       = "version";
static const juce::Identifier flashSoftwareTextKey = "flashSoftware";

PowerPage::PowerPage() : Locale::TextUser(localeClassKey),
PageComponent("PowerPage"),
powerOffButton(localeText(shutdownTextKey)),
rebootButton(localeText(rebootTextKey)),
sleepButton(localeText(sleepTextKey)),
felButton(localeText(flashSoftwareTextKey)),
versionLabel(localeText(versionTextKey) 
+ juce::JUCEApplication::getInstance()->getApplicationVersion()),
lockscreen([this]()
{
    hideLockscreen();
})
{
    using namespace Layout::Group;
    RelativeLayout layout({
        Row(6, { RowItem(&versionLabel) }),
        Row(10, { RowItem(&powerOffButton) }),
        Row(10, { RowItem(&sleepButton) }),
        Row(10, { RowItem(&rebootButton) }),
        Row(10, { RowItem(&felButton) }),
        Row(6, { RowItem(&buildLabel) })
    });
    layout.setYMarginFraction(0.1);
    layout.setYPaddingWeight(4);
    setBackButton(PageComponent::rightBackButton);
    setLayout(layout);
 
    versionLabel.setJustificationType(juce::Justification::centred);
    versionLabel.setText(localeText(versionTextKey) 
        + juce::JUCEApplication::getInstance()->getApplicationVersion(),
            juce::NotificationType::dontSendNotification);
    
    // Determine release label contents
    juce::String buildText = localeText(buildTextKey) + " ";
#ifdef BUILD_NAME
    buildText += juce::String(BUILD_NAME);
#else
    buildText += juce::String("unset");
#endif
    buildLabel.setJustificationType(juce::Justification::centred);
    buildLabel.setText(buildText, juce::NotificationType::dontSendNotification);

    powerOffButton.addListener(this);
    sleepButton.addListener(this);
    rebootButton.addListener(this);
    felButton.addListener(this);
    addChildComponent(overlaySpinner);
    addAndShowLayoutComponents();
}

/*
 * Turns off the display until key or mouse input is detected.
 */
void PowerPage::startSleepMode()
{
    SystemCommands systemCommands;
    if(systemCommands.runIntCommand(SystemCommands::IntCommand::sleepCheck)
            == 0)
    {
        systemCommands.runActionCommand(SystemCommands::ActionCommand::wake);
    }
    else
    {
        addAndMakeVisible(lockscreen);
        lockscreen.setBounds(getLocalBounds());
        lockscreen.setAlwaysOnTop(true);
        systemCommands.runActionCommand(SystemCommands::ActionCommand::sleep);
    }
}

/*
 * If the lock screen is visible, this will remove it from the screen.
 */
void PowerPage::hideLockscreen()
{
    if (lockscreen.isShowing())
    {
        removeChildComponent(&lockscreen);
        removeFromStack(Layout::Transition::Type::moveRight);
    }
}

/*
 * Shows the power spinner to indicate to the user that the system is
 * restarting or shutting down.
 */
void PowerPage::showPowerSpinner()
{
    powerOffButton.setVisible(false);
    sleepButton.setVisible(false);
    rebootButton.setVisible(false);
    felButton.setVisible(false);
    overlaySpinner.setVisible(true);
}

/*
 * Resizes the lock screen and overlay spinner to fit the page.
 */
void PowerPage::pageResized()
{
    lockscreen.setBounds(getLocalBounds());
    overlaySpinner.setBounds(getLocalBounds());
}

/*
 * Handles all button clicks.
 */
void PowerPage::pageButtonClicked(juce::Button *button)
{
    using namespace juce;
    if (button == &felButton)
    {
        pushPageToStack(PageType::Fel, Layout::Transition::Type::moveRight);
        return;
    }
    if (button == &sleepButton)
    {
        startSleepMode();
        return;
    }

    SystemCommands systemCommands;
    if (button == &powerOffButton)
    {
        showPowerSpinner();
        systemCommands.runActionCommand
            (SystemCommands::ActionCommand::shutdown);
    }
    else if (button == &rebootButton)
    {
        showPowerSpinner();
        systemCommands.runActionCommand
            (SystemCommands::ActionCommand::restart);
    }
}
