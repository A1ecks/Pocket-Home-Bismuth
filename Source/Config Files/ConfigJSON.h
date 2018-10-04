#pragma once
#include <map>
#include "SharedResource.h"
#include "ResourceHandler.h"
#include "JSONFile.h"
#include "ConfigKey.h"
#include "JuceHeader.h"
#include "ConfigAlertWindows.h"

/**
 * @file ConfigJSON.h
 * 
 * @brief Reads and writes data from a JSON configuration file. 
 * 
 *  ConfigJSON provides an abstract base for classes that read and write JSON 
 * configuration files.  Each ConfigJSON subclass is responsible for a single
 * JSON file containing data that can be altered by the user.  Along with 
 * reading and writing data, ConfigJSON allows objects to be defined as listener
 * objects, which will receive notifications whenever data keys they select are
 * changed.
 * 
 * As an implementation of the SharedResource class, each ConfigJSON subclass
 * will have only one object instance at a time, to prevent concurrent access
 * to the JSON file, and limit the amount of file I/O necessary.  Each
 * ConfigJSON subclass should be accessed only through a ConfigFile subclass
 * specific to that ConfigJSON.
 *
 * A default version of each ConfigJSON's JSON resource file should be placed in
 * the configuration subdirectory of the asset folder.  Any missing or invalid 
 * parameters in config files will be replaced with values from the default 
 * file.
 *
 * ConfigJSON reads from each JSON file only once per program instance, so any 
 * external changes to the file that occur while the program is running will
 * most likely be ignored and may be overwritten.
 */
class ConfigJSON : public SharedResource
{
protected:
    /**
     * @brief Creates the ConfigJSON resource, and prepares to read JSON data.
     *
     * @param resourceKey     The SharedResource object key for the specific
     *                        ConfigJSON variant being created.
     * 
     * @param configFilename  The name of a JSON file to read or create in
     *                        the config file directory. There should be a file
     *                        with the same name in the asset folder filled with
     *                        default values.
     */
    ConfigJSON(const juce::Identifier& resourceKey,
            const juce::String& configFilename);

public:
    /**
     * @brief  Writes any pending changes to the file before destruction.
     */
    virtual ~ConfigJSON();

    /**
     * @brief  Gets one of the values stored in the JSON configuration file.
     * 
     * @param key                       The key string that maps to the desired 
     *                                  value.
     * 
     * @tparam ValueType                The value's data type.
     * 
     * @throws BadKeyException          If the key parameter was not a valid 
     *                                  key string for this ConfigJSON.
     * 
     * @throws JSONFile::TypeException  If the key does not map to a value of 
     *                                  type ValueType in this config file.
     * 
     * @return                          The value read from the config file.
     */
    template<typename ValueType >
    ValueType getConfigValue(const juce::Identifier& key)
    {      
        using namespace juce;
        if(!isValidKey(key))
        {
            throw BadKeyException(key);
        }
        try
        {
            return configJson.getProperty<ValueType>(key);
        }
        catch(JSONFile::TypeException e)
        {
            DBG("ConfigJSON::" << __func__ 
                    << ": Failed to load key \"" 
                    << e.getPropertyKey()
                    <<"\" , expected type:" << e.getExpectedType()
                    <<", actual type: "  << e.getFoundType()
                    << ", error = " << e.what());
            throw e;
        }
        catch (JSONFile::FileException e)
        {
            //Failed to access .json file
            DBG("ConfigJSON::" << __func__ << ": " << e.what());
            alertWindows.showPlaceholderError(e.what());
        }
        return ValueType();
    }

    /**
     * @brief  Sets one of this ConfigJSON's values, notifying listeners and 
     *         writing to the JSON file if the value is changed.
     * 
     * @param key               The key string that maps to the value being 
     *                          updated.
     * 
     * @param newValue          The new value to save to the file.
     * 
     * @tparam ValueType        The value data type.
     * 
     * @throws BadKeyException  If the key parameter was not a valid key string
     *                          for this ConfigJSON.
     *
     * @return                  True if the value changed, false if the new
     *                          value matched the old value.
     */
    template<typename ValueType>
    bool setConfigValue(const juce::Identifier& key, ValueType newValue)
    {
        using namespace juce;
        if(!isValidKey(key))
        {
            throw BadKeyException(key);
        }
        if(updateProperty<ValueType>(key, newValue))
        {
            configJson.writeChanges();
            notifyListeners(key);
            return true;
        }
        return false;
    }
     
    /**
     * @brief  Sets a configuration data value back to its default setting. 
     *
     * If this changes the value, listeners will be notified and changes will be
     * saved.
     * 
     * @param key                A key value defined in the config file.
     * 
     * @throws BadKeyException   If the key parameter was not a valid key string
     *                           for this ConfigJSON.
     */
    virtual void restoreDefaultValue(const juce::Identifier& key);

    /**
     * @brief  Restores all values in the configuration file to their defaults. 
     *
     * All updated values will notify their Listeners and be written to the JSON
     * file.
     */
    virtual void restoreDefaultValues();
     
    /**
     * Signals an attempt to access an invalid config value in a ConfigJSON.
     */
    struct BadKeyException : public std::exception
    {
    public:
        /**
         * @param invalidKey  The invalid key string that caused this exception.
         */
        BadKeyException(const juce::Identifier& invalidKey) : 
        invalidKey(invalidKey) { }
        
        virtual const char* what() const noexcept override
        {
            return juce::CharPointer_UTF8(invalidKey);
        }
        
        /**
         * @brief  Gets the invalid key that caused the exception.
         * 
         * @return  The unexpected key value.
         */
        const juce::Identifier& getInvalidKey()
        {
            return invalidKey;
        }
    private:
        const juce::Identifier& invalidKey;
    };
 
/**
 * Receives updates whenever tracked key values are updated in a JSON
 *        configuration file.
 */
class Listener : public SharedResource::Handler
{
public:
    //Allow the ConfigJSON class access to the list of tracked keys.
    friend ConfigJSON;
    
    /**
     * @brief Initialize a new listener to track ConfigJSON properties.
     *
     * @param resourceKey      SharedResource object key for the Listener's
     *                         ConfigJSON subclass.
     *
     * @param createResource   A function that creates the listener's ConfigJSON
     *                         object if necessary.
     */
    Listener(const juce::Identifier& resourceKey,
            const std::function<SharedResource*()> createResource) : 
    SharedResource::Handler(resourceKey, createResource) { }

    virtual ~Listener() { }

protected:
    /**
     * @brief Calls configValueChanged() for every key tracked by this listener.
     */
    virtual void loadAllConfigProperties();

    /**
     * @brief Adds a key to the list of keys tracked by this listener.
     *
     * @param keyToTrack  Whenever a value with this key is updated, the
     *                    Listener will be notified.
     */
    void addTrackedKey(const juce::Identifier& keyToTrack);

    /**
     * @brief Unsubscribes from updates to a ConfigJSON value.
     *
     * @param keyToRemove  This listener will no longer receive updates when
     *                     the value with this key is updated.
     */
    void removeTrackedKey(const juce::Identifier& keyToRemove);
    

    /**
     * @brief  Requests a stored value directly from this Listener's ConfigJSON.
     *
     * @tparam ValueType        The type of value requested.
     *
     * @param key               The key to the requested value.
     *
     * @throws BadKeyException  If the key does not map to a value with the 
     *                          correct type.
     *
     * @return                  The requested value.
     */
    template<typename ValueType>
    ValueType getConfigValue(const juce::Identifier& key)
    {
        ThreadLock& configLock = getResourceLock();
        configLock.takeReadLock();
        
        ConfigJSON* config = static_cast<ConfigJSON*>(getClassResource());
        ValueType value = config->getConfigValue<ValueType>(key);

        configLock.releaseLock();
        return value;
    }

private:
    /**
     * @brief  This method will be called whenever a key tracked by this 
     *         listener changes in the config file.
     * 
     * @param propertyKey   Passes in the updated value's key.
     */
    virtual void configValueChanged
    (const juce::Identifier& propertyKey) = 0;

    /* Tracks all keys this listener follows. */
    juce::Array<juce::Identifier, juce::CriticalSection> subscribedKeys;
};

protected:
    /**
     * @brief  Announces a changed configuration value to each Listener object.
     * 
     * @param key  The key of an updated configuration value. 
     */
    void notifyListeners(const juce::Identifier& key);
    
    /**
     * @brief  Checks if a single handler object is a Listener tracking updates
     *         of a single key value, and if so, notifies it that the tracked
     *         value has updated.
     *
     * @param listener   A Listener that might be tracking the updated value.
     *
     * @param key        The key to an updated configuration value.
     */
    virtual void notifyListener(Listener* listener,
            const juce::Identifier& key);  

    /**
     * @brief  Loads all initial configuration data from the JSON config file. 
     *
     * This checks for all expected data keys, and replaces any missing or 
     * invalid values with ones from the default config file. ConfigJSON 
     * subclasses should call this once, after they load any custom object or 
     * array data.
     */
    void loadJSONData();

    /**
     * @brief  Checks if a key string is valid for this ConfigJSON.
     * 
     * @param key  A key string value to check.
     * 
     * @return  True iff the key is valid for this file.
     */
    virtual bool isValidKey(const juce::Identifier& key) const;
    
    /**
     * Get the set of all basic (non-array, non-object) properties tracked by
     * this ConfigJSON.
     * 
     * @return  The keys to all variables tracked in this config file.
     */
    virtual const std::vector<ConfigKey>& getConfigKeys() const = 0;

    
    /**
     * Checks for an expected property value in the JSON config data.  If the
     * value is not found or has an invalid type, the property will be copied
     * from the default config file.
     * 
     * @param key  The key string of a parameter that should be defined in this
     *             config file.
     * 
     * @tparam T   The expected type of the JSON property.
     * 
     * @return  The initialized property value.
     * 
     * @throws JSONFile::TypeException  If any data value is missing or has an
     *                                  incorrect type in both the config file 
     *                                  and the default config file.
     */
    template<typename T> T initProperty(const juce::Identifier& key)
    {
        try
        {
            if(!configJson.propertyExists<T>(key))
            {
                DBG("ConfigJSON::" << __func__ << ": Key \"" << key
                        << "\" not found in " << filename 
                        << ", checking default config file");
                if(!defaultJson.propertyExists<T>(key))
                {
                    DBG("ConfigJSON::" << __func__ << ": Key \"" << key 
                            << "\" missing in config and default config!");
                }
                T property = defaultJson.getProperty<T>(key);
                configJson.setProperty<T>(key, property);
                return property;
            }
            return configJson.getProperty<T>(key);
        }
        catch (JSONFile::FileException e)
        {
            //Failed to read from .json file
            DBG("ConfigJSON::" << __func__ << ": " << e.what());
            alertWindows.showPlaceholderError(e.what());
        }
        catch(JSONFile::TypeException e)
        {
            DBG("ConfigJSON::" << __func__ 
                    << ": Failed to load default value for " 
                    << e.getPropertyKey()
                    <<", expected type:" << e.getExpectedType()
                    <<", actual type: "  << e.getFoundType()
                    << ", error = " << e.what());
            alertWindows.showPlaceholderError(e.what());
        }
        return T();
    }
    
    /**
     * Updates a property in the JSON config file.  This will not check
     * key validity, immediately write any changes, or notify listeners.
     * 
     * @param key        The key string that maps to the value being updated.
     * 
     * @param newValue   The new value to save to the file.
     * 
     * @tparam T         The value data type.
     * 
     * @return  True if JSON data changed, false if newValue was identical to
     *          the old value with the same key.
     */
    template<typename T>
    bool updateProperty(const juce::Identifier& key, T newValue)
    {
        try
        {
            return configJson.setProperty<T>(key, newValue);
        }
        catch (JSONFile::FileException e)
        {
            //Failed to write to .json file
            DBG("ConfigJSON::" << __func__ << ": " << e.what());
            alertWindows.showPlaceholderError(e.what());
        }
        catch(JSONFile::TypeException e)
        {
            DBG("ConfigJSON::" << __func__ << ": " << e.what());
            alertWindows.showPlaceholderError(e.what());
        }
        return false;
    }

    /**
     * Re-writes all data back to the config file, as long as there are
     * changes to write.
     * 
     * @pre Any code calling this function is expected to have already
     *      acquired the ConfigJSON's lock
     */
    void writeChanges();

private:
     
    /**
     * @brief  Sets a configuration data value back to its default setting, 
     *         notifying listeners if the value changes.
     *
     * This does not ensure that the key is valid for this ConfigFile.  An
     * AlertWindow will be shown if any problems occur while accessing JSON
     * data.
     *
     * @param key  The key of the value that will be restored.
     */
    void restoreDefaultValue(const ConfigKey& key);
    
    /**
     * @brief  Writes any custom object or array data back to the JSON file.
     *
     * ConfigJSON subclasses with custom object or array data must override this
     * method to write that data back to the file.
     */
    virtual void writeDataToJSON() { }
    
    /* The name of this JSON config file. */
    const juce::String filename;

    /* Holds configuration values read from the file. */
    JSONFile configJson;
    
    /* Holds default config file values. */
    JSONFile defaultJson; 
    
    /* Used to send error messages to the user. */
    ConfigAlertWindows alertWindows;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConfigJSON)
};