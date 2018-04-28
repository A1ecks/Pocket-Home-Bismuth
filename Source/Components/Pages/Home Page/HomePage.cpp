#include "ConfigurableImageComponent.h"
#include "MainConfigFile.h"
#include "PokeLookAndFeel.h"
#include "AssetFiles.h"
#include "PagedAppMenu.h"
#include "ScrollingAppMenu.h"
#include "HomePage.h"

HomePage::HomePage() :
PageComponent("HomePage",{}, false),
frame(ComponentConfigFile::menuFrameKey, 0, RectanglePlacement::stretchToFit),
powerButton(ComponentConfigFile::powerButtonKey),
settingsButton(ComponentConfigFile::settingsButtonKey)
{
#    if JUCE_DEBUG
    setName("HomePage");
#    endif
    MainConfigFile mainConfig;
    mainConfig.addListener(this,{
        MainConfigFile::backgroundKey,
        MainConfigFile::menuTypeKey
    });

    setWantsKeyboardFocus(true);
    addAndMakeVisible(frame);
    addAndMakeVisible(clock);

    addAndMakeVisible(batteryIcon);
    addAndMakeVisible(wifiIcon);

    powerButton.addListener(this);
    powerButton.setWantsKeyboardFocus(false);
    addAndMakeVisible(powerButton);

    settingsButton.addListener(this);
    settingsButton.setWantsKeyboardFocus(false);
    addAndMakeVisible(settingsButton);

    addChildComponent(loadingSpinner);
    loadingSpinner.setAlwaysOnTop(true);

    loadAllConfigProperties();
}

/**
 * Tracks page background and menu type changes. Only the MainConfigFile 
 * should be calling this.  Depending on the key provided, this will update
 * the page background or recreate the AppMenu.
 */
void HomePage::configValueChanged(String key)
{
    MainConfigFile mainConfig;
    if (key == MainConfigFile::backgroundKey)
    {
        String background = mainConfig.getConfigValue<String>
                (MainConfigFile::backgroundKey);
        if (background.containsOnly("0123456789ABCDEFXabcdefx"))
        {
            setBackgroundImage(Image());
            Colour bgFill(background.getHexValue32());
            setColour(backgroundColourId, bgFill.withAlpha(1.0f));
        }
        else
        {
            setBackgroundImage(AssetFiles::loadImageAsset(background));
        }
    }
    else if (key == MainConfigFile::menuTypeKey)
    {
        String menuType = mainConfig.getConfigValue<String>
                (MainConfigFile::menuTypeKey);
        if (!MainConfigFile::menuTypes.contains(menuType))
        {
            DBG("HomePage::" << __func__ << ": Invalid menu type!");
            return;
        }
        if (appMenu != nullptr)
        {
            removeChildComponent(appMenu);
            appMenu = nullptr;
        }

        if (menuType == "Scrolling menu")
        {
            if (!isClass<AppMenuComponent, ScrollingAppMenu>(appMenu.get()))
            {
                DBG("HomePage::" << __func__
                        << ": Initializing scrolling menu");
                appMenu = new ScrollingAppMenu(loadingSpinner);
            }
            else
            {
                DBG("HomePage::" << __func__
                        << ": Menu was already scrolling, don't recreate");
            }
        }
        else//menuType == "pagedMenu"
        {
            if (!isClass<AppMenuComponent, PagedAppMenu>(appMenu.get()))
            {
                DBG("HomePage::" << __func__
                        << ": Initializing paged menu");
                appMenu = new PagedAppMenu(loadingSpinner);
            }
            else
            {
                DBG("HomePage::" << __func__ <<
                        ": Menu was already paged, don't recreate");
            }
        }
        addAndMakeVisible(appMenu);
        appMenu->toBack();
        pageResized();

    }
}

/**
 * Forward all clicks (except button clicks) to the appMenu so that it can 
 * potentially create a pop-up menu
 */
void HomePage::mouseDown(const MouseEvent &event)
{
    if (event.mods.isPopupMenu() || event.mods.isCtrlDown())
    {
        appMenu->openPopupMenu(nullptr);
    }
}

/**
 * Opens the power page or the settings page, depending on which button
 * was clicked.
 * 
 * @param button
 */
void HomePage::pageButtonClicked(Button * button)
{
    if (button == &settingsButton)
    {
        pushPageToStack(PageComponent::PageType::Settings);
    }
    else if (button == &powerButton)
    {
        pushPageToStack(PageComponent::PageType::Power,
                PageComponent::Animation::slideInFromLeft);
    }
}

/**
 * Forwards all key events to the AppMenu.
 */
bool HomePage::keyPressed(const KeyPress& key)
{
    //don't interrupt animation or loading
    if (Desktop::getInstance().getAnimator().isAnimating(appMenu)
        || appMenu->isLoading())
    {
        return true;
    }
    else return appMenu->keyPressed(key);
}

/**
 * Grab keyboard focus when the page becomes visible.
 */
void HomePage::visibilityChanged()
{
    if (isShowing())
    {
        MessageManager::callAsync([this]()
        {
            grabKeyboardFocus();
        });
    }
}

/**
 * Update all child component bounds when the page is resized.
 */
void HomePage::pageResized()
{
    if (appMenu != nullptr)
    {
        appMenu->applyConfigBounds();
    }
    loadingSpinner.setBounds(getLocalBounds());
    frame.applyConfigBounds();
    clock.applyConfigBounds();
    batteryIcon.applyConfigBounds();
    wifiIcon.applyConfigBounds();
    powerButton.applyConfigBounds();
    settingsButton.applyConfigBounds();
}