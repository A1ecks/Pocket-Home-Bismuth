#include "Config_FileResource.h"
#include "AssetFiles.h"

#ifdef JUCE_DEBUG
/* Print the full class name before all debug output: */
static const constexpr char* dbgPrefix = "Config::FileResource::";
#endif
    
/* The directory where all config files will be located. */
static const constexpr char* configPath = "~/.pocket-home/";

/* The pocket-home asset folder subdirectory containing default config files. */
static const constexpr char* defaultAssetPath = "configuration/";

/*
 * Loads the resource's JSON data files.
 */
Config::FileResource::FileResource(
        const juce::Identifier& resourceKey,
        const juce::String& configFilename) :
SharedResource::Resource(resourceKey), 
filename(configFilename),
configJson(configPath + filename),
defaultJson(defaultAssetPath + filename) { }

/*
 * Writes any pending changes to the file before destruction.
 */
Config::FileResource::~FileResource()
{
    writeChanges();
}

/*
 * Sets a configuration data value back to its default setting.
 */
void Config::FileResource::restoreDefaultValue(const juce::Identifier& key)
{
    // Check key validity, find expected data type:
    const std::vector<DataKey>& keys = getConfigKeys();
    for(const DataKey& configKey : keys)
    {
        if(configKey.key == key)
        {
            restoreDefaultValue(configKey);
            return;
        }
    }
    DBG(dbgPrefix << __func__ << ": Key \"" << key 
                << "\" is not expected in " << filename);
    throw BadKeyException(key);
}

/*
 * Restores all values in the configuration file to their defaults.
 */
void Config::FileResource::restoreDefaultValues()
{
    const std::vector<DataKey>& keys = getConfigKeys();
    for(const DataKey& key : keys)
    {
        restoreDefaultValue(key);
    }
    writeChanges();
}

/*
 * Loads all initial configuration data from the JSON config file.
 */
void Config::FileResource::loadJSONData()
{
    const std::vector<DataKey>& keys = getConfigKeys();
    for (const DataKey& key : keys)
    {
        try
        {
            switch (key.dataType)
            {
                case DataKey::stringType:
                    initProperty<juce::String>(key);
                    break;
                case DataKey::intType:
                    initProperty<int>(key);
                    break;
                case DataKey::boolType:
                    initProperty<bool>(key);
                    break;
                case DataKey::doubleType:
                    initProperty<double>(key);
                    break;
                default:
                    DBG(dbgPrefix << __func__ << ": Unexpected type for key \"" 
                            << key.key << "\" in file " << filename);
            }
        }
        catch(JSONFile::FileException e)
        {
            DBG(dbgPrefix << __func__ << ": Caught FileException:" << e.what());
        }
        catch(JSONFile::TypeException e)
        {
            DBG(dbgPrefix << __func__ << ": Caught TypeException:" << e.what());
        }
    }
    writeChanges();
}
 
/*
 * Checks if a key string is valid for this FileResource.
 */
bool Config::FileResource::isValidKey(const juce::Identifier& key) const
{
    const std::vector<DataKey>& keys = getConfigKeys();
    for(const DataKey& configKey : keys)
    {
        if(configKey.key == key)
        {
            return true;
        }
    }
    return false;
}

/*
 * Re-writes all data back to the config file, as long as there are changes to 
 * write.
 */
void Config::FileResource::writeChanges()
{
    writeDataToJSON();
    try
    {
        configJson.writeChanges();
    }
    catch(JSONFile::FileException e)
    {
        DBG(dbgPrefix << __func__ << ": Caught FileException:" << e.what());
    }
    catch(JSONFile::TypeException e)
    {
        DBG(dbgPrefix << __func__ << ": Caught TypeException:" << e.what());
    }
}

/*
 * Sets a configuration data value back to its default setting, notifying 
 * listeners if the value changes.
 */
void Config::FileResource::restoreDefaultValue(const DataKey& key)
{
    try
    {
        switch(key.dataType)
        {
            case DataKey::stringType:
                setConfigValue<juce::String>(key,
                        defaultJson.getProperty<juce::String>(key));
                return;
            case DataKey::intType:
                setConfigValue<int>(key,
                        defaultJson.getProperty<int>(key));
                return;
            case DataKey::boolType:
                setConfigValue<bool>(key,
                        defaultJson.getProperty<bool>(key));
                return;
            case DataKey::doubleType:
                setConfigValue<double>(key,
                        defaultJson.getProperty<double>(key));
        }
    }
    catch(JSONFile::FileException e)
    {
        DBG(dbgPrefix << __func__ << ": Caught FileException:" << e.what());
    }
    catch(JSONFile::TypeException e)
    {
        DBG(dbgPrefix << __func__ << ": Caught TypeException:" << e.what());
    }
}
