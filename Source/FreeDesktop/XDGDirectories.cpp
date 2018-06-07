#include <cstdlib>
#include "XDGDirectories.h"

//###############   XDG Path environment variables   ##########################

//Stores the default user data directory:
static const constexpr char* dataDirVar     = "XDG_DATA_HOME";
//Stores the default user config directory:
static const constexpr char* configDirVar   = "XDG_CONFIG_HOME";
//Stores the default user cache directory:
static const constexpr char* cacheDirVar    = "XDG_CACHE_HOME";
//Stores the default user runtime file directory:
static const constexpr char* runtimeDirVar  = "XDG_RUNTIME_DIR";
//Stores the list of paths to search for user data:
static const constexpr char* dataPathsVar   = "XDG_DATA_DIRS";
//Stores the list of paths to search for user config files:
static const constexpr char* configPathsVar = "XDG_CONFIG_DIRS";

//######################   Default Path Values   ##############################
// Default paths to use if the XDG environment variables are not set.

//Default user data directory, relative to $HOME
static const String defaultDataDir   = "/.local/share";
//Default user config directory, relative to $HOME
static const String defaultConfigDir = "/.config";
//Default user cache directory, relative to $HOME
static const String defaultCacheDir = "/.cache";
//Default data paths to search after defaultDataDir
static const String defaultDataPaths = "/usr/local/share:/usr/share";
//Default config paths to search after defaultConfigDir
static const String defaultConfigPaths = "/etc/xdg";

/*
 * Returns a string from an environment variable, or a default string if the
 * environment variable is unset or empty.  
 */
static String getEnvOrDefaultString(const char* envVar, String defaultStr)
{
    const char* envString = std::getenv(envVar);
    if(envString == nullptr || envString[0] == '\0')
    {
        return String(defaultStr);
    }
    return String(envString);
}

/*
 * Gets the user's home directory.
 */
static String homePath()
{
    return String(getenv("HOME"));
}

/*
 * Gets the path of the single base directory where user-specific data files
 * should be written.
 */
String XDGDirectories::getUserDataPath()
{
    return getEnvOrDefaultString(dataDirVar, homePath() + defaultDataDir);
}

/*
 * Gets the path of the single base directory where user-specific 
 * configuration files should be written.
 */
String XDGDirectories::getUserConfigPath()
{
    return getEnvOrDefaultString(configDirVar, homePath() + defaultConfigDir);
} 

/*
 * Gets the path of the single base directory where user-specific 
 * cache files should be written.
 */
String XDGDirectories::getUserCachePath()
{
    return getEnvOrDefaultString(cacheDirVar, homePath() + defaultCacheDir);
}

/*
 * Gets the path of the single base directory where user-specific 
 * runtime files should be written.
 */
String XDGDirectories::getUserRuntimePath()
{
    String runtimePath = getEnvOrDefaultString(runtimeDirVar, "");
    if(runtimePath.isEmpty())
    {
        DBG("XDGDirectories::" << __func__ << ": " << runtimeDirVar
                << " is not defined!");
        jassertfalse;
    }
    return runtimePath;
}

/*
 * Gets the ordered list of directories to search for user data files.
 */
StringArray XDGDirectories::getDataSearchPaths()
{
    String paths = getUserDataPath() + ":";
    paths += getEnvOrDefaultString(dataPathsVar, defaultDataPaths);
    return StringArray::fromTokens(paths, ":", "");
}

/*
 * Gets the ordered list of directories to search for user configuration
 * files.
 */
StringArray XDGDirectories::getConfigSearchPaths()
{
    String paths = getUserConfigPath() + ":";
    paths += getEnvOrDefaultString(configPathsVar, defaultConfigPaths);
    return StringArray::fromTokens(paths, ":", "");
}