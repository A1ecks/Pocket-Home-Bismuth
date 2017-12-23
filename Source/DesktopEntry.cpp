/* 
 * File:   DesktopEntry.cpp
 * Author: anthony
 * 
 * Created on December 14, 2017, 1:46 PM
 */

#include "DesktopEntry.h"
#include "DesktopEntries.h"
#include "Utils.h"
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <regex>
#include <stdio.h>


//initialize string and bool maps with all valid keys

void DesktopEntry::mapInit() {
    std::vector<String> stringKeys = {
        "Version",
        "Name",
        "GenericName",
        "Comment",
        "Icon",
        "OnlyShowIn",
        "NotShowIn",
        "TryExec",
        "Exec",
        "Path",
        "Actions",
        "MimeType",
        "Categories",
        "Implements",
        "Keywords",
        "StartupWMClass",
        "URL"
    };
    std::vector<String> boolKeys{
        "NoDisplay",
        "Hidden",
        "DBusActivatable",
        "Terminal",
        "StartupNotify"};
    //populate maps and write default values:
    for (int i = 0; i < stringKeys.size(); i++) {
        appStrings[String(stringKeys[i])] = "";
    }
    for (int i = 0; i < boolKeys.size(); i++) {
        appBools[String(boolKeys[i])] = false;
    }
}

//Load DesktopEntry data from a .desktop or .directory file

DesktopEntry::DesktopEntry(String path, String localeName, PathRecord * pathRecords) :
pathRecord(pathRecords),
entrypath(path) {
    mapInit();
    try {
        std::ifstream desktopFile(path.toStdString());
        //capture "key[locale]=val" lines from the .Desktop/.Directory file
        std::regex varCapture("([A-Za-z]+)(\\[[a-zA-Z0-9_@]+\\])?\\=(.*)");
        std::smatch varMatch;
        for (std::string line; getline(desktopFile, line);) {
            if (line.substr(0, 1) == "#")continue; //skip comments
            if (std::regex_search(line, varMatch, varCapture)) {
                String key;
                String locale;
                String val;
                key = varMatch.str(1);
                if (varMatch.size() > 3) {
                    locale = varMatch.str(2);
                    val = varMatch.str(3);
                } else {
                    val = varMatch.str(2);
                }
                if (locale.isEmpty() || locale == localeName) {
                    std::map<String, String>::iterator strSearch =
                            appStrings.find(key);
                    if (key == "Type") {
                        if (val == "Application") {
                            type = APPLICATION;
                        } else if (val == "Link") {
                            type = LINK;
                        } else if (val == "Directory") {
                            type = DIRECTORY;
                        } else {
                            DBG("invalid type:" + val);
                        }
                    } else if (strSearch != appStrings.end()) {
                        appStrings[key] = val;
                    } else {
                        std::map < String, bool>::iterator boolSearch =
                                appBools.find(key);
                        if (boolSearch != appBools.end()) {
                            if (val == "true") {
                                appBools[key] = true;
                            } else if (val == "false") {
                                appBools[key] = false;
                            } else {
                                DBG("invalid boolean pair " + key + ":" + val);
                            }
                        } else DBG("unrecognized data:" + key + "=" + val);
                    }
                }
            } else DBG("no match:" + line);
        }
        desktopFile.close();
    } catch (std::ifstream::failure e) {
        DBG("couldn't open " + path);
    }
}

//Creates a DesktopEntry object representing an application category
DesktopEntry::DesktopEntry(String category, PathRecord * pathRecords) :
pathRecord(pathRecords) {
    mapInit();
    type = DIRECTORY;
    appStrings["Name"] = category;
    appStrings["Icon"] = category;
    appStrings["Exec"] = "OPEN:" + category;
}

DesktopEntry::DesktopEntry(const DesktopEntry& orig) {
    pathRecord = orig.pathRecord;
    entrypath = orig.entrypath;
    appStrings = orig.appStrings;
    appBools = orig.appBools;
    type = orig.type;
    iconPath = orig.iconPath;
}

DesktopEntry::~DesktopEntry() {
}

String DesktopEntry::getName() {
    return appStrings["Name"];
}

DesktopEntry::Type DesktopEntry::getType() {
    return type;
}

String DesktopEntry::getVersion() {
    return appStrings["Version"];
}

String DesktopEntry::getGenericName() {
    return appStrings["GenericName"];
}

String DesktopEntry::getComment() {
    return appStrings["Comment"];
}

String DesktopEntry::getIconPath() {
    if (!iconPath.isEmpty())return iconPath;
    return findIconPath();
}

String DesktopEntry::getExec() {
    return appStrings["Exec"];
}

String DesktopEntry::getPath() {
    return appStrings["Path"];
}

String DesktopEntry::getStartupWMClass() {
    return appStrings["StartupWm"];
}

String DesktopEntry::getURL() {
    return appStrings["Exec"];
}

std::vector<String> DesktopEntry::getActions() {
    return split(appStrings["Actions"], ";");
}

std::vector<String> DesktopEntry::getMimeType() {
    return split(appStrings["MimeType"], ";");
}

std::vector<String> DesktopEntry::getCategories() {
    String categories = appStrings["Categories"];
    if (categories.isEmpty())categories = "Other";
    return split(categories, ";");
}

std::vector<String> DesktopEntry::getImplements() {
    return split(appStrings["Implements"], ";");
}

std::vector<String> DesktopEntry::getKeywords() {
    return split(appStrings["Keywords"], ";");
}

bool DesktopEntry::onlyShowIn(String env) {
    String showIn = appStrings["OnlyShowIn"];
    return showIn.contains(env);
}

bool DesktopEntry::notShowIn(String env) {
    String showIn = appStrings["NotShowIn"];
    return showIn.contains(env);
}

bool DesktopEntry::tryExec() {
    return fileExists(appStrings["TryExec"]);
}

bool DesktopEntry::noDisplay() {
    return appBools["NoDisplay"];
}

bool DesktopEntry::hidden() {
    return appBools["Hidden"];
}

bool DesktopEntry::dBusActivatable() {
    return appBools["DBusActivatable"];
}

bool DesktopEntry::terminal() {
    return appBools["Terminal"];
}

bool DesktopEntry::startupNotify() {
    return appBools["StartupNotify"];
}

//Recursively search directories under path for an icon file

String DesktopEntry::searchIconPaths(String icon, String path) {
    std::vector<String> files = listFiles(path);
    String iconFile;
    std::regex iconMatch(icon.toStdString() + "(\\.(png|svg|xpm))?$", std::regex::ECMAScript | std::regex::icase);

    foreach(files, [path, &iconFile, iconMatch](String file)->bool {
        if (std::regex_search(file.toStdString(), iconMatch)) {
            iconFile = path + file;
            return true;
        }
        return false;
    });
    if (!iconFile.isEmpty())return iconFile;
    //check subdirectories if not already found
    std::vector<String> dirs = listDirectoryFiles(path);
    if (dirs.empty())return iconFile;
    //sort icon size directories, if found
    std::regex re("^([0-9]+)");
    std::smatch numMatch;
    std::sort(dirs.begin(), dirs.end(), [&numMatch, &re](const String& a, const String & b)->bool {
        std::string a_str = a.toStdString();
        std::string b_str = b.toStdString();
        if (a == b)return false;
                int aVal = 0;
                int bVal = 0;
            if (std::regex_search(a_str, numMatch, re)) {
                sscanf(numMatch.str(1).c_str(), "%d", &aVal);
            }
        if (std::regex_search(b_str, numMatch, re)) {
            sscanf(numMatch.str(1).c_str(), "%d", &bVal);
        }
        if (aVal != bVal) {
            //higher numbers first, until after 128px
            if (aVal > 128)aVal = 1 + 128 / aVal;
                if (bVal > 128)bVal = 1 + 128 / bVal;
                    return aVal > bVal;
                }
        return false;
    });

    foreach(dirs, [icon, path, this, &iconFile](String s)->bool {
        iconFile = searchIconPaths(icon, path + s + "/");
        return !iconFile.isEmpty();
    });
    return iconFile;
}

//look through likely directories to find an icon file's path.

String DesktopEntry::findIconPath() {
    std::vector<String> checkPaths;
    String icon;
    try {
        icon = appStrings.at("icon");
    } catch (std::out_of_range e) {//no icon defined
        return "";
    }

    //explicit path defined, return it
    if (icon.substring(0, 1) == "/") {
        iconPath = icon;
        return iconPath;
    }
    //check cached data
    if (pathRecord != NULL) {
        String path = pathRecord->getRecord(icon);
        if (path == PathRecord::noPath)return "";
        if (!path.isEmpty()) {
            iconPath = path;
            return iconPath;
        }
    }
    std::vector<String> basePaths = {
        getHomePath() + "/.icons/",
        "/usr/share/pocket-home/appIcons/",
        "/usr/share/icons/",
        "/usr/local/icons/",
        "/usr/share/pixmaps/"
    };

    //if no explicit path, find all directories to search
    //check under system theme first
    String theme = getTheme();
    if (!theme.isEmpty()) {
        for (int i = 0; i < basePaths.size(); i++) {
            checkPaths.push_back(basePaths[i] + theme + "/");
        }
    }
    //if it didn't turn up there, try the fallback theme
    String fallbackTheme = getFallbackTheme();
    if (!fallbackTheme.isEmpty()) {
        for (int i = 0; i < basePaths.size(); i++) {
            checkPaths.push_back(basePaths[i] + fallbackTheme + "/");
        }
    }
    //search all icon files if a theme-specific one doesn't show up
    for (int i = 0; i < basePaths.size(); i++) {
        checkPaths.push_back(basePaths[i]);
    }

    //recursively search directories until the icon file turns up
    for (int i = 0; i < checkPaths.size(); i++) {
        iconPath = searchIconPaths(icon, checkPaths[i]);
        if (!iconPath.isEmpty()) {
            if (pathRecord != NULL)pathRecord->addRecord(icon, iconPath);
            return iconPath;
        }
    }
    DBG("Missing icon " + icon);
    if (pathRecord != NULL)pathRecord->addRecord(icon, PathRecord::noPath);
    return iconPath;
}
