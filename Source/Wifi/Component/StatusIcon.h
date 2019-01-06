#pragma once
/**
 * @file  Wifi/Component/StatusIcon.h
 * 
 * @brief  Displays an icon indicating Wifi connection signal strength.
 */

#include "Theme_Image_Component.h"
#include "DrawableImageComponent.h"
#include "Wifi/Device/DeviceListener.h"
#include "Wifi/AccessPoint/SignalStrengthListener.h"
#include "Wifi/Connection/Listener.h"

namespace Wifi { class StatusIcon; }


class Wifi::StatusIcon : 
    public Theme::Image::Component<DrawableImageComponent>, 
    public SignalStrengthListener, 
    public Connection::Listener,
    public DeviceListener
{
public:
    StatusIcon();

    virtual ~StatusIcon() { }

private:
    /**
     * All wifi state icons.  Each should correspond to an image asset file 
     * defined in components.json
     */
    enum class APIcon
    {
        wifiOff,
        wifiStrength0,
        wifiStrength1,
        wifiStrength2,
        wifiStrength3
    };

    /**
     * @brief  Sets the displayed WiFi connection icon.
     * 
     * @param selectedIcon  The wifi icon that matches the current connection
     *                      state.
     */
    void setIcon(const APIcon selectedIcon);
    
    /**
     * @brief  Updates the selected icon when the active connection's signal
     *         strength changes.
     *
     * @param updatedAP  The active connection's AccessPoint.
     */
    virtual void signalStrengthUpdate(const AccessPoint updatedAP) 
        final override;

    /**
     * @brief  Changes the tracked AccessPoint and selected status icon when a
     *         new Wifi connection is activated.
     *
     * @param connectedAP  The new connection's access point object.
     */
    virtual void connected(const AccessPoint connectedAP) final override; 

    /**
     * @brief  Stops tracking signal strength changes and updates the status
     *         icon when the Wifi connection is lost.
     *
     * @param disconnectedAP  The disconnected access point object.
     */
    virtual void disconnected(const AccessPoint disconnectedAP) final override;

    /**
     * @brief  Updates the selected icon when Wifi is turned on.
     */
    virtual void wirelessEnabled() final override;

    /**
     * @brief  Updates the selected icon and stops tracking signal strength when
     *         Wifi is turned off.
     */
    virtual void wirelessDisabled() final override;
};
