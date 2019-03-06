# Config Module Documentation

The Config module defines an interface for reading and writing application configuration settings from user-accessible JSON files. Config allows easy, threadsafe access to configuration data, minimizing the amount of necessary file IO. Config also defines Listener objects, which are notified whenever configuration data they track is changed at any point.

Each configuration file has its own FileHandler class. Any object of that FileHandler class can be used to read or write all data values in the JSON file in a threadsafe manner.

FileHandlers also define their own listener classes, which receive notifications whenever data keys they select are changed through any FileHandler connected to the same JSON file.

## Public Interface

#### [Config\::FileResource](../../Source/Config/Config_FileResource.h)
FileResource is an abstract basis for JSON file resource classes. A new Config::FileResource subclass should be implemented for each JSON configuration file.

#### [Config\::FileHandler](../../Source/Config/Config_FileHandler.h)
FileHandler is an abstract basis for classes that access JSON file resources. Each Config::FileResource subclass should have at least one Config::FileHandler subclass defined to provide controlled access to the file resource.

#### [Config\::DataKey](../../Source/Config/Config_DataKey.h)
DataKey objects store the key and type of a basic data value stored in a FileResource. These should be used to declare all string, integer, double, and boolean values provided by a Config::FileResource subclass.

#### [Config\::Listener](../../Source/Config/Config_Listener.h)
Listener is an abstract basis for classes that listen for changes to JSON file resources. Listeners may track any number of value keys, and will receive notifications only when their tracked key values change in the JSON resource.

#### [Config\::ValueListener](../../Source/Config/Config_ValueListener.h)
ValueListener is a template wrapper for Listener subclasses. ValueListener tracks changes to a single value within a single JSON resource.

#### [Config\::MainFile](../../Source/Config/Config_MainFile.h)
MainFile is a fileHandler subclass that accesses the main.json file, where miscellaneous configuration data is stored that doesn't belong in a specialized JSON file.

#### [Config\::MainListener](../../Source/Config/Config_MainListener.h)
MainListener objects listen for changes to values defined in the main.json file.

## Private Implementation Classes

#### [Config\::MainResource](../../Source/Config/Implementation/Config_MainResource.h)
MainResource is the FileResource class that handles all access to the main.json configuration file.

#### [Config\::MainKeys](../../Source/Config/Config_MainKeys.h)
MainKeys provides the keys to all values accessed through MainFile and defined in main.json. 

#### [Config\::ListenerInterface](../../Source/Config/Implementation/Config_ListenerInterface.h)
ListenerInterface is the interface used by FileResource objects to send notifications to associated Listener objects.

#### [Config\::AlertWindow](../../Source/Config/Implementation/Config_AlertWindow.h)
AlertWindow objects notify the user when there are problems with reading or writing configuration files.