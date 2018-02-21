/**
 * @file PowerPageComponent.h
 * 
 * PowerPageComponent is a UI menu page that shows buttons that perform actions
 * related to system power state.  From this page, the user can shut down the
 * system, reboot the system, turn off the display, enter PocketCHIP flashing
 * mode, or close the page.
 */
#pragma once
#include "../PageComponent.h"
#include "../Configuration/Configurables/ConfigurableImageButton.h"
#include "../Basic Components/OverlaySpinner.h"
#include "../Basic Components/SwitchComponent.h"
#include "../MainComponent.h"
#include "LoginPage.h"
#include "FelPage.h"

/**
 * TODO:
 * 
 * -Set the sleep mode command in config.json
 * -Restore build/version labels.
 * -Make background color configurable.
 */

class PowerPage : public PageComponent {
public:
    PowerPage();
    ~PowerPage();
private:
    /**
     * Turns off the display until key or mouse input is detected.
     * The lock screen will be visible when the display turns on again.
     */
    void startSleepMode();

    /**
     * If the lock screen is visible, this will remove it from the screen.
     */
    void hideLockscreen();

    /**
     * Show the power spinner to indicate to the user that the system is
     * restarting or shutting down.
     */
    void showPowerSpinner();
    /**
     * Handles all button clicks.
     */
    void pageButtonClicked(Button*) override;

    void pageResized() override;
    //Turns off the system using the shutdown command in the MainConfigFile.
    TextButton powerOffButton;
    //Starts sleep mode with startSleepMode()
    TextButton sleepButton;
    //Restarts the system using the reboot command in the MainConfigFile.
    TextButton rebootButton;
    //Shows a page that gives the user the option to enter flashing mode.
    TextButton felButton;
    //Spinner to indicate that the system is rebooting/shutting down
    OverlaySpinner overlaySpinner;
    //Page for showing flashing options, opened by felButton.
    FelPage felPage;
    //Used to run shutdown/restart/sleep commands
    //ChildProcess commandProcess;
    //The lock screen is displayed after entering sleep mode.
    LoginPage lockscreen;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PowerPage)
};
