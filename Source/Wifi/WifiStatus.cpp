#include <map>

#include "WifiStatus.h"
#include "../JuceLibraryCode/JuceHeader.h"

WifiStatus::WifiStatus() { }

WifiStatus::~WifiStatus() { }

WifiStatus::Listener::Listener() { }

WifiStatus::Listener::~Listener() { }

const WifiAccessPoint WifiAccessPoint::null = {"", -1, false, ""};
