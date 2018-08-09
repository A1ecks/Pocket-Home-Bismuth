#pragma once

#include "IconSliderComponent.h"
#include "Localized.h"
#include "SwitchComponent.h"
#include "ConfigurableImageButton.h"
#include "PageComponent.h"
#include "WindowFocusedTimer.h"
#include "WifiSettingsComponent.h"
#include "ScalingTextButton.h"
//#include "BluetoothSettingsComponent.h"

/**
 * @file QuickSettingsPage.h
 * 
 * @brief  Provides wireless device controls, brightness and volume sliders, and 
 *         a button to open additional settings pages.
 */

class QuickSettingsPage : public PageComponent, public WindowFocusedTimer,
        private juce::Slider::Listener, private Localized
{
public:
    QuickSettingsPage();

    virtual ~QuickSettingsPage() { }

    private:    
    /**
     * Cancels the slider timer when visibility is lost.
     */
    virtual void visibilityChanged() override;
    
    /**
     * Used to update the sliders while they're being dragged.
     */
    virtual void timerCallback() override;
    
    /**
     * Opens the advanced settings page when its button is clicked.
     * 
     * @param b
     */
    void pageButtonClicked(juce::Button *b) override;

    /**
     * Updates the advanced settings button when the page is resized.
     */
    void pageResized() override;

    /**
     * Slider::Listener requires this method to be implemented, but it's not 
     * actually needed.
     */
    void sliderValueChanged(juce::Slider* slider) { };
    
    /**
     * Starts a timer to update the slider values as its being dragged.
     * 
     * @param slider
     */
    void sliderDragStarted(juce::Slider* slider);
    
    /**
     * Stops the timer and immediately updates slider values.
     * 
     * @param slider
     */
    void sliderDragEnded(juce::Slider* slider);
    
    //Tracks the slider currently being dragged so the timer callback knows
    //whether it should update brightness or volume
    juce::Slider* changingSlider;

    //Turns wifi on or off, shows connection state, and opens the wifi page.
    WifiSettingsComponent wifiComponent;
    //Turns bluetooth on or of, shows connection state, and opens the bluetooth
    //page.
    //BluetoothSettingsComponent bluetoothComponent;
    //sets the display brightness
    IconSliderComponent screenBrightnessSlider;
    //sets system volume levels
    IconSliderComponent volumeSlider;
    //opens the settings list page
    ConfigurableImageButton settingsListBtn;
    
    //localized text keys
    static const constexpr char * advanced_settings = "advanced_settings";
};