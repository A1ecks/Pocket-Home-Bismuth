
#include "HomePage.h"
#include "InputSettingsPage.h"
#include "SetPasswordPage.h"
#include "RemovePasswordPage.h"
#include "PowerPage.h"
#include "FelPage.h"
#include "QuickSettingsPage.h"
#include "Bluetooth_SettingsPage.h"
#include "Wifi/Component/SettingsPage.h"
#include "Theme_Colour_ConfigPage.h"
#include "SettingsListPage.h"
#include "DateTimePage.h"
#include "HomeSettingsPage.h"
#include "LoginPage.h"
#include "PageFactory.h"


/**
 * Initializes a HomePage instance to use as the root page of the page 
 * stack.
 */
PageComponent* PageFactory::createHomePage()
{
    return setPageFactory(new HomePage());
}

/**
 * Initializes a login page instance.
 */
PageComponent* PageFactory::createLoginPage
(std::function<void () > loginCallback)
{
    return new LoginPage(loginCallback);
}

/**
 * Create a new page to push on top of the page stack.
 */
PageComponent* PageFactory::createPage(PageComponent::PageType type)
{
    switch (type)
    {
        case PageComponent::InputSettings:
            return setPageFactory(new InputSettingsPage());
        case PageComponent::Keybinding:
            return nullptr;
        case PageComponent::SetPassword:
            return new SetPasswordPage();
        case PageComponent::RemovePassword:
            return new RemovePasswordPage();
        case PageComponent::Power:
            return setPageFactory(new PowerPage());
        case PageComponent::Fel:
            return new FelPage();
        case PageComponent::QuickSettings:
            return setPageFactory(new QuickSettingsPage());
        case PageComponent::BluetoothSettings:
            return new Bluetooth::SettingsPage();
        case PageComponent::WifiSettings:
            return new Wifi::SettingsPage();
        case PageComponent::UI:
            return nullptr;
        case PageComponent::ColourSettings:
            return new Theme::Colour::ConfigPage();
        case PageComponent::SettingsList:
            return setPageFactory(new SettingsListPage());
        case PageComponent::DateTime:
            return new DateTimePage();
        case PageComponent::HomeSettings:
            return new HomeSettingsPage();
    }
    return nullptr;
}
