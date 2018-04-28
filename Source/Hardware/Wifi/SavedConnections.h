#pragma once
#include "GPPDBusProxy.h"
#include "SavedConnection.h"

/**
 * @file SavedConnections.h
 * 
 * @brief Connects to NetworkManager to read saved network connections.
 * 
 * Only wifi connections are supported at this time.  The only role of this
 * class is to read in saved connections as SavedConnection objects, and return
 * all wifi connections.
 */

class SavedConnections : public GPPDBusProxy
{
public:
    SavedConnections();
    
    virtual ~SavedConnections() { }
    
    /**
     * Reads all connection paths from NetworkManager, and returns all the wifi
     * connections as SavedConnection objects.
     * 
     * @return all saved wifi connections.
     */
    Array<SavedConnection> getWifiConnections();
    
    /**
     * Checks saved connection paths to see if one exists at the given path.
     * 
     * @param connectionPath  A DBus path to check for a connection.
     * 
     * @return  true iff connectionPath is a valid path to a saved connection. 
     */
    bool connectionExists(const String& connectionPath);
    
private:
    /**
     * Check the list of saved connections against an updated connection path
     * list, adding any new connections and removing any deleted connections.
     */
    void updateSavedConnections();
    
    /**
     * Get the list of all available connection paths
     * 
     * @return the list of paths, freshly updated over the DBus interface.
     */
    inline StringArray getConnectionPaths();
    
    Array<SavedConnection> connectionList;    
};