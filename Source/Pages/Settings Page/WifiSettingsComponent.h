/**
 * @file WifiSettingsComponent.h
 * 
 *TODO:document, get rid of all the unnecessary redundancy.
 */
#pragma once

#include "../../Basic Components/Spinner.h"
#include "../../Wifi/WifiStatus.h"
#include "ConnectionSettingsComponent.h"
class WifiSettingsComponent : public ConnectionSettingsComponent, 
        public WifiStatus::Listener {
public:
    WifiSettingsComponent();
    virtual ~WifiSettingsComponent();

    void resized() override;
    void enabledStateChanged(bool enabled) override;
    void updateButtonText() override;

    void handleWifiEnabled() override;
    void handleWifiDisabled() override;
    void handleWifiConnected() override;
    void handleWifiDisconnected() override;
    void handleWifiFailedConnect() override;
    void handleWifiBusy() override;

private:
    void enableWifiActions();
    void disableWifiActions();
    Spinner spinner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WifiSettingsComponent)
};