/* Only include this file directly in the AppMenu implementation! */
#ifdef APPMENU_IMPLEMENTATION_ONLY

#pragma once
#include "JuceHeader.h"
#include "OverlaySpinner.h"
#include "DesktopEntryLoader.h"
#include "AppMenu/AppMenu.h"
#include "AppMenu/Control/Initializer.h"
#include "AppMenu/Components/MenuComponent.h"
#include "AppMenu/Control/Controller.h"
#include "AppMenu/Data/ConfigFile.h"

/**
 * @file  MainComponent.h
 *
 * @brief  Creates and shows an AppMenu of any format.
 */

class AppMenu::MainComponent : public juce::Component, 
    public ConfigFile::Listener
{
public:
    /**
     * @brief  Creates and initializes the application menu.
     *
     * @param initializer  A struct that defines how to initialize a specific
     *                     menu format.
     */
    MainComponent(const Initializer* initializer);

    virtual ~MainComponent();

    /**
     * @brief  Initialize the menu as a new menu format, cleaning up any
     *         existing menu first.
     *
     * @param initializer  A struct that defines how to initialize the new menu
     *                     format.  If this matches the format of the existing
     *                     menu, no action will be taken.
     */
    void initMenu(const Initializer* initializer);

private:
    /**
     * @brief  Safely destroys all AppMenu objects held in the MainComponent.
     */
    void destroyMenu();

    /**
     * @brief  Notifies the MenuComponent that its parent's bounds have changed
     *         whenever the MainComponent is moved or resized.
     */
    virtual void resized() final override;

    /**
     * @brief  Updates the menu when the selected format changes.
     *
     * @param propertyKey  The menu format key.
     */
    virtual void configValueChanged(const juce::Identifier& propertyKey)
        final override;

    /* Tracks the current AppMenu format. */
    AppMenu::Format currentMenuFormat = Format::Invalid;

    /* Loads AppMenu shortcuts and folder definitions.  This resource should
       exist as long as the home page exists. */
    AppMenu::ConfigFile appConfig;

    /* The menu's loading spinner. */
    OverlaySpinner loadingSpinner;

    /* Handles the appearance and layout of the menu. */
    std::unique_ptr<MenuComponent> menuComponent;

    /* Allows the inputHandler to control menu behavior. */
    std::unique_ptr<Controller> controller;

    /* Listens to key, mouse, and window focus events, and responds by
       triggering appropriate menu events. */
    std::unique_ptr<InputHandler> inputHandler;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

/* Only include this file directly in the AppMenu implementation! */
#endif