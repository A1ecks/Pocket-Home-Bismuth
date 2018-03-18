/**
 * @file WifiStateManager.h
 *
 * WifiStateManager tracks the current state of the Wifi network device,
 * provides information about nearby access points and any current connections,
 * notifies Listener objects whenever Wifi state changes, and provides an
 * interface for requesting Wifi connections or other changes in Wifi
 * state.
 *
 * WifiStateManager::WifiNetworkInterface is responsible for all direct
 * communication with the Wifi device.  In order for a WifiStateManager
 * object to function correctly, an object implementing WifiNetworkInterface
 * must be assigned to it with setNetworkInterface.
 */

#pragma once
#include "JuceHeader.h"
#include "WifiAccessPoint.h"

class WifiStateManager : private Timer
{
public:
    WifiStateManager();

    /**
     * On destruction, the WifiStateManager removes all references
     * to itself from its Listener objects, and destroys its
     * WifiNetworkInterface.
     * This method acquires the WifiStateManager's stateLock.
     */
    virtual ~WifiStateManager();

    enum WifiState
    {
        //Wifi state unknown, WifiNetworkInterface needs to be assigned
        //with setNetworkManager.
        missingNetworkInterface,
        //Wifi device is disabled.
        disabled,
        //Wifi device is being turned on.
        turningOn,
        //Wifi is turned on but not connected.
        enabled,
        //Wifi device is being turned on.
        turningOff,
        //Wifi device is connecting to an access point.
        connecting,
        //Wifi device is connected to an access point.
        connected,
        //Wifi device is closing its connection to an access point.
        disconnecting,
        //Wifi is disconnecting from one access point in
        //order to connect to another.
        switchingConnection,
        //WifiStateManager not available.
        noStateManager
    };

    /**
     * Assigns a WifiNetworkInterface to this WifiStateManager.
     * The state manager will take ownership of the network
     * interface, deleting it on destruction or when a new
     * interface is provided through setNetworkInterface.
     * This method acquires the WifiStateManager's stateLock.
     *
     * @param interface takes responsibility for communicating
     * with the Wifi device.
     */
    void setNetworkInterface(WifiNetworkInterface* interface);

    /**
     * @return the current state of the wifi device, or
     * missingNetworkInterface if the WifiNetworkInterface
     * hasn`t been set.
     * This method acquires the WifiStateManager's stateLock.
     */
    WifiState getWifiState();

    /**
     * Add a listener to the list of objects receiving updates
     * whenever Wifi state changes.
     * This method acquires the WifiStateManager's stateLock.
     *
     * If the listener had already been added to another
     * WifiStateManager, it will be removed from that state manager
     * before it's added to this one.  In debug builds this will
     * also print a warning and fail an assertion, as there really
     * shouldn`t be more than one WifiStateManager.
     *
     * @param listener will be subscribed to state updates
     */
    void addListener(Listener* listener);

    /**
     * Search the list of registered listeners for a particular one,
     * and remove it if it's found.
     *  This method acquires the WifiStateManager's stateLock.
     * @param listener will be unsubscribed from state updates.
     */
    void removeListener(Listener* listener);

    /**
     * @return the current connected Wifi access point, or
     * a null WifiAccessPoint if there is no connection.
     */
    WifiAccessPoint getConnectedAP();

    /**
     * @return the current connecting WifiAccessPoint if one
     * is found, or a null WifiAccessPoint if not connecting.
     */
    WifiAccessPoint getConnectingAP();

    /**
     * @return all wifi access points currently detected.
     * If wifi is disabled, this will return an empty list.
     */
    Array<WifiAccessPoint> getVisibleAPs();

    /**
     * Attempt to open a connection to a wifi access point.
     * This will fail if wifi is disabled, the access point is
     * invalid, or the psk is wrong.
     * This method acquires the WifiStateManager's stateLock.
     *
     * @param toConnect should be a valid nearby access point
     *
     * @param psk wifi key for toConnect, or the empty string
     * if toConnect isn't secured.
     */
    void connectToAccessPoint(WifiAccessPoint toConnect,
            String psk = String());

    /**
     * If currently connected to or trying to connect to a particular
     * access point, that connection will be closed or canceled.  This method
     * acquires the WifiStateManager's stateLock.
     * 
     * @param toDisconnect should be the connected or connecting access point.
     */
    void disconnect(WifiAccessPoint toDisconnect);

    /**
     * Listener objects can be added to a WifiStateManager to receive
     * wifi state updates.  Once added to a WifiStateManager, Listener
     * objects can get the current wifi state at any time until
     * they're removed from the state manager.
     */
    class Listener
    {
    public:
        friend class WifiStateManager;
        Listener();

        /**
         * On destruction, listeners will remove themselves from
         * their WifiStateManager if necessary. If this is necessary,
         * this method  indirectly acquires the WifiStateManager's stateLock.
         */
        virtual ~Listener();

    protected:
        /**
         * @return the current wifi state if the listener was added
         * to a WifiStateManager.  Otherwise, returns noStateManager.
         * If the listener was added to a state manager, this method 
         * indirectly acquires the WifiStateManager's stateLock.
         */
        WifiState getWifiState();

    private:
        /**
         * When added to a WifiStateManager, it will call this
         * method whenever the wifi state changes.  When this callback runs,
         * the state manager's stateLock will be locked.  This method is
         * not guaranteed to run on the message thread.
         *
         * @param state new wifi device state
         */
        virtual void wifiStateChanged(WifiState state) = 0;
        WifiStateManager * const stateManager = nullptr;
    };

    /**
     * WifiNetworkInterface is an abstract interface for 
     * communication between the WifiStateManager and whatever
     * object handles direct interaction with the wifi device.
     */
    class WifiNetworkInterface
    {
        friend class WifiStateManager
    protected:
        WifiNetworkInterface();

        /**
         * If the WifiNetworkInterface belongs to a WifiStateManager
         * and it's destroyed early, it will safely remove all
         * references to itself from the WifiStateManager.
         * This method acquires the WifiStateManager's stateLock.
         */
        virtual ~WifiNetworkInterface();

        /**
         * Access the wifi device to check if wifi is enabled.
         * 
         * @return true iff wifi is enabled
         */
        virtual bool isWifiDeviceEnabled() = 0;

        /**
         * Access the wifi device to check if an active wifi connection
         * exists.
         * 
         * @return true iff a connection is found.
         */
        virtual bool isWifiConnected() = 0;

        /**
         * @return the current connected WifiAccessPoint if one
         * is found, or a null WifiAccessPoint if not connected.
         */
        virtual WifiAccessPoint getConnectedAP() = 0;

        /**
         * @return the current connecting WifiAccessPoint if one
         * is found, or a null WifiAccessPoint if not connecting.
         */
        virtual WifiAccessPoint getConnectingAP() = 0;

        /**
         * Scan for and return nearby wifi access points.
         * 
         * @return the list of all wifi access points close enough
         * to detect.
         */
        virtual Array<WifiAccessPoint> getVisibleAPs() = 0;


        /**
         * Attempt to open a connection to a wifi access point.
         * This will fail if wifi is disabled, the access point is
         * invalid, or the psk is wrong.
         * @param toConnect should be a valid nearby access point
         *
         * @param psk wifi key for toConnect, or the empty string
         * if toConnect isn't secured.
         */
        virtual void connectToAccessPoint(WifiAccessPoint toConnect,
                String psk = String()) = 0;

        /**
         * If a wifi access point is currently connected, attempt to disconnect
         * from that access point.
         */
        virtual void disconnect() = 0;

        /**
         * This method queries the wifi device to ensure that the current 
         * tracked wifi state actually matches the device state.  If necessary,
         * this will update the WifiStateManager's current state.  The
         * WifiNetworkInterface should call this method whenever it has reason
         * to believe the wifi state may be out of date.  
         * 
         * This method acquires the WifiStateManager's stateLock.
         */
        void confirmWifiState();

        /**
         * Whenever the wifi device establishes a new connection,
         * the WifiNetworkInterface should call this to notify
         * its WifiStateManager.
         * This method acquires the WifiStateManager's stateLock.
         *
         * @param connected the newly connected access point
         */
        void signalWifiConnected(WifiAccessPoint connected);

        /**
         * Whenever the wifi device fails to connect to an access
         * point, the WifiNetworkInterface should call this to
         * notify its WifiStateManager.
         * This method acquires the WifiStateManager's stateLock.
         */
        void signalConnectionFailed();

        /**
         * Whenever the wifi device disconnects from a wifi access
         * point, the WifiNetworkInterface should call this to
         * notify its WifiStateManager.
         * This method acquires the WifiStateManager's stateLock.
         */
        void signalWifiDisconnected();

        /**
         * Whenever the wifi device is turned on, the 
         * WifiNetworkInterface should call this to
         * notify its WifiStateManager.
         * This method acquires the WifiStateManager's stateLock.
         */
        void signalWifiEnabled();

        /**
         * Whenever the wifi device is turned off, the 
         * WifiNetworkInterface should call this to
         * notify its WifiStateManager.
         * This method acquires the WifiStateManager's stateLock.
         */
        void signalWifiDisabled();
    private:
        WifiStateManager* stateManager = nullptr;
    }
private:
    //Milliseconds to wait before assuming that enabling or
    //disabling wifi has failed.
    const int wifiEnabledChangeTimeout = 10000;

    //Milliseconds to wait before assuming that connecting to
    //or disconnecting from a wifi access point has failed.
    const int wifiConnectionTimeout = 35000;

    /**
     * Whenever a wifi operation is attempted, the timer is set to
     * the appropriate timeout period. If the timer goes off before 
     * the WifiNetworkInterface responds, the WifiStateManager will
     * assume that the wifi operation failed.
     * This method acquires the WifiStateManager's stateLock.

     */
    void timerCallback() override;

    /**
     * Update the current wifi state and notify all listeners.
     * This method should only be called after acquiring the WifiStateManager's
     * stateLock
     */
    void setWifiState(WifiState state);

    /**
     * Checks the validity of the current wifi state, possibly changing the
     * state to missingNetworkInterface if necessary.  This method should only 
     * be called after acquiring the WifiStateManager's stateLock.
     * 
     * @return true iff the current wifi state is noStateManager or 
     * missingNetworkInterface
     */
    bool invalidWifiState();

    //holds any pending connection data

    struct
    {
        WifiAccessPoint ap;
        String psk;
    } pendingConnection;
    //Holds the last canceled connection, so the state manager knows to 
    //immediately disconnect if the networkInterface announces a connection to
    //this access point.
    WifiAccessPoint cancelledConnection;

    WifiState wifiState = missingNetworkInterface;
    ScopedPointer<WifiNetworkInterface> networkInterface = nullptr;
    Array<Listener> wifiListeners;
    CriticalSection stateLock;
};

