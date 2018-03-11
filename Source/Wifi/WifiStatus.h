/**
 * @file WifiStatus.h
 * 
 * TODO: documentation
 */
#pragma once
#include <exception>
#include "../Utils.h"

struct WifiAccessPoint {
    String ssid;
    int signalStrength; // 0 to 100
    bool requiresAuth;
    String hash;

    /**
     * Represents an invalid or missing connection point
     */
    static const WifiAccessPoint null;

    bool operator==(const WifiAccessPoint rhs) const {
        return hash == rhs.hash;
    };

    bool operator!=(const WifiAccessPoint rhs) const {
        return hash != rhs.hash;
    };
};

class WifiStatus {
public:
    WifiStatus();
    virtual ~WifiStatus();

    class Listener {
    public:
        Listener();
        virtual ~Listener();

        virtual void handleWifiEnabled() {
        }

        virtual void handleWifiDisabled() {
        }

        virtual void handleWifiConnected() {
        }

        virtual void handleWifiDisconnected() {
        }

        virtual void handleWifiFailedConnect() {
        }

        virtual void handleWifiBusy() {
        }
    };

    virtual Array<WifiAccessPoint> nearbyAccessPoints() = 0;
    virtual WifiAccessPoint connectedAccessPoint() const = 0;
    virtual bool isEnabled() const = 0;
    virtual bool isConnected() const = 0;

    virtual void addListener(Listener* listener) = 0;
    virtual void clearListeners() = 0;

    virtual void enableWifi() = 0;
    virtual void disableWifi() = 0;
    virtual void setConnectedAccessPoint(const WifiAccessPoint& ap,
            String psk = String::empty) = 0;
    virtual void disconnect() = 0;
};


