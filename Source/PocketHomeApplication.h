/**
 * @file PocketHomeApplication.h
 * 
 * TODO: documentation, organization, combine MainWindow with MainComponent.h
 */

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "MainComponent.h"
#include "Configuration/MainConfigFile.h"
#include "Configuration/ComponentConfigFile.h"
#include "Wifi/WifiStatus.h"
#include "Wifi/WifiStatusJson.h"
#include "Wifi/WifiStatusNM.h"
#include "BluetoothStatus.h"
#include "PokeLookAndFeel.h"
#include "Utils.h"



PageStackComponent &getMainStack();

class PocketHomeApplication : public JUCEApplication {
public:
    PocketHomeApplication();

    static PocketHomeApplication* getInstance();

    PageStackComponent& getMainStack();

    WifiStatus& getWifiStatus();

    BluetoothStatus& getBluetoothStatus();

    MainConfigFile& getConfig();
    
    ComponentConfigFile& getComponentConfig();

private:
    bool initAudio();
    MainConfigFile configFile;
    ComponentConfigFile componentConfig;
    PokeLookAndFeel lookAndFeel;
#ifdef LINUX
    WifiStatusNM wifiStatusNM;
#else
    WifiStatusJson wifiStatusNM;
#endif //LINUX

    WifiStatusJson wifiStatusJson;
    WifiStatus *wifiStatus;

    BluetoothStatus bluetoothStatus;
    const String getApplicationName() override;
    const String getApplicationVersion() override;
    bool moreThanOneInstanceAllowed() override;

    void initialise(const String &commandLine) override;
    void shutdown() override;

    void systemRequestedQuit() override;
    void anotherInstanceStarted(const String &commandLine) override;

    class MainWindow : public DocumentWindow {
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)

    public:
        MainWindow(String name);

        void activeWindowStatusChanged() override;
        void closeButtonPressed() override;
    };

private:
    ScopedPointer<MainWindow> mainWindow;
};