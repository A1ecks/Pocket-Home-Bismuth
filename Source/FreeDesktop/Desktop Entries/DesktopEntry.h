#pragma once
#include <map>
#include "AppConfigFile.h"

/* 
 * @file DesktopEntry.h
 * 
 * DesktopEntry reads in standardized .Desktop file data
 * the Desktop entry format is very well documented at
 * https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-latest.html#recognized-keys
 */

class DesktopEntry
{
public:
    /**
     * Load DesktopEntry data from a .desktop or .directory file
     * 
     * @param entryFile
     */
    DesktopEntry(File entryFile);

    /**
     * Creates a new desktop entry from parameter data.
     * 
     * @param title            The application display title, which will also 
     *                          be used as the filename.
     * 
     * @param icon             The full path of an icon file, or the name of an
     *                          icon file in one of the standard icon 
     *                          directories.
     * 
     * @param command          The launch command for the desktop application.
     * 
     * @param categories       The list of categories that describe the 
     *                          application.
     * 
     * @param launchInTerminal  Marks if the application should run in a
     *                           terminal window.
     */
    DesktopEntry(String title, String icon, String command,
            StringArray categories,
            bool launchInTerminal);

    virtual ~DesktopEntry() { }

    /**
     * Defines all entry Type values. Only the application type is currently
     * handled correctly.
     */
    enum Type
    {
        application,
        link,
        directory
    };

    /**
     * Gets the file type for this entry.
     * 
     * @return the file type for this entry, or application if no valid type
     *          is found.
     */
    Type getType() const;

    /**
     * Defines all possible String values that can be stored in a DesktopEntry.
     */
    enum StringValue
    {
        version,
        name,
        genericName,
        comment,
        icon,
        tryExec,
        exec,
        path,
        startupWMClass,
        url
    };

    /**
     * Defines all possible boolean values that can be stored in a DesktopEntry.
     */
    enum BoolValue
    {
        noDisplay,
        hidden,
        dBusActivatable,
        terminal,
        startupNotify
    };

    /**
     * Defines all String list values that can be stored in a DesktopEntry.
     */
    enum ListValue
    {
        onlyShowIn,
        notShowIn,
        actions,
        mimeType,
        categories,
        implements,
        keywords
    };

    /**
     * Get stored string data.
     * 
     * @param valueType  The desired string variable.
     * 
     * @return the stored value, or empty string if not found.
     */
    String getValue(StringValue valueType) const;

    /**
     * Get stored boolean data.
     * 
     * @param valueType  The desired bool variable.
     * 
     * @return the stored value, or false if not found.
     */
    bool getValue(BoolValue valueType) const;

    /**
     * Get stored list data.
     * 
     * @param valueType  The desired list variable.
     * 
     * @return the stored value, or the empty list if not found.
     */
    StringArray getValue(ListValue valueType) const;

    /**
     * Changes the value of stored string data.
     * 
     * @param valueType  The string variable to change.
     * 
     * @param newValue   The new value for valueType
     */
    void setValue(StringValue valueType, String newValue);

    /**
     * Changes the value of stored boolean data.
     * 
     * @param valueType  The boolean variable to change.
     * 
     * @param newValue   The new value for valueType.
     */
    void setValue(BoolValue valueType, bool newValue);

    /**
     * Changes the value of stored list data.
     * 
     * @param valueType  The list variable to change.
     * 
     * @param newValue   The new value for valueType.
     */
    void setValue(ListValue valueType, StringArray newValue);

    /**
     * Exports this entry to a .Desktop file.  This creates or overwrites a 
     * file in ~/.local/share/applications, so the changes will only apply to 
     * this user.
     */
    void writeFile();


    //To prevent duplicates, two entries are equal as long as they have
    //the same file name.
    bool operator==(const DesktopEntry toCompare) const;
    
    //used for sorting entries by name, 
    bool operator<(const DesktopEntry toCompare) const;

private:
    /**
     * @param valueType any StringValue
     * 
     * @return that value's key String
     */
    String getKey(StringValue valueType) const;

    /**
     * @param valueType any BoolValue
     * 
     * @return that value's key String
     */
    String getKey(BoolValue valueType) const;

    /**
     * @param valueType any ListValue
     * 
     * @return that value's key String
     */
    String getKey(ListValue valueType) const;

    static constexpr const char* typeKey = "Type";

    struct LineValues
    {
        String key;
        String locale;
        String value;
    };

    /**
     * Parses desktop entry file lines.
     * 
     * @param line   A single line from a desktop entry file.
     * 
     * @return       the data extracted from that line.
     */
    LineValues getLineData(String line);

    //Stores all data values as strings, mapped to their respective key values
    std::map<String, String> dataStrings;

    //path of the .Desktop/.Directory file
    String entrypath;

    //local user directory for storing updated .Desktop files
    static const String localEntryPath;

};

