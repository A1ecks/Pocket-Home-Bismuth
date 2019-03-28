#include "Info_IPLabel.h"
#include "Config_MainFile.h"
#include "Wifi_AccessPoint.h"
#include "Util_Commands.h"

#ifdef JUCE_DEBUG
/* Print the full class name before all debug output: */
static const constexpr char* dbgPrefix = "Info::IPLabel::";
#endif

/* Localized text class key: */
static const juce::Identifier localeClassKey = "Info::IPLabel";

/* Localized text value keys: */
static const juce::Identifier localIPKey  = "localIP";
static const juce::Identifier publicIPKey = "publicIP";

/*
 * Loads appropriate label text on construction.
 */
Info::IPLabel::IPLabel() noexcept :
Locale::TextUser(localeClassKey)
{
    updateLabelText();
}

/*
 * Updates the IP label text to match the IPLabel settings in the main
 * configuration file and the current system IP address(es).
 */
void Info::IPLabel::updateLabelText() noexcept
{
    using juce::String;
    Config::MainFile mainConfig;
    String newText;
    if(mainConfig.getIPLabelPrintsLocal())
    {
        String localIP = commandLoader.runTextCommand(
                Util::CommandTypes::Text::getLocalIP);
        if(localIP.isNotEmpty())
        {
            newText = localeText(localIPKey) + localIP;
            
        }
    }
    if(mainConfig.getIPLabelPrintsPublic())
    {
        String publicIP = commandLoader.runTextCommand(
                Util::CommandTypes::Text::getPublicIP);
        if(publicIP.isNotEmpty())
        {
            if(newText.isNotEmpty())
            {
                newText += "\t";
            }
            newText += localeText(publicIPKey) + publicIP;
        }
    }
    setText(newText, juce::NotificationType::sendNotification);
}

/*
 * Updates the label text when the IPLabel gains visibility.
 */
void Info::IPLabel::visibilityChanged()
{
    if(isVisible())
    {
        updateLabelText();
    }
}

#ifdef WIFI_SUPPORTED
/*
 * Updates the label text whenever a new network connection becomes active.
 */
void Info::IPLabel::connected(const Wifi::AccessPoint connectedAP)
{
    DBG(dbgPrefix << __func__ << ": Connection opened, updating label text.");
    updateLabelText();
}

/*
 * Updates the label text whenever the active network connection is closed.
 */
void Info::IPLabel::disconnected(const Wifi::AccessPoint connectedAP)
{
    DBG(dbgPrefix << __func__ << ": Connection closed, updating label text.");
    updateLabelText();
}
#endif