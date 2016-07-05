#ifdef LINUX

#include <map>
#include <nm-remote-connection.h>
#include <nm-utils.h>

#include "WifiStatus.h"
#include "../JuceLibraryCode/JuceHeader.h"

#define LIBNM_ITERATION_PERIOD 100 // milliseconds

WifiStatusNM::WifiStatusNM() : listeners() {}
WifiStatusNM::~WifiStatusNM() {}

NMClient* WifiStatusNM::connectToNetworkManager() {
  if (!nmclient || !NM_IS_CLIENT(nmclient))
    nmclient = nm_client_new();

  if (!nmclient || !NM_IS_CLIENT(nmclient))
    DBG("WifiStatusNM: failed to connect to nmclient over dbus");

  if (!nmdevice || !NM_IS_DEVICE(nmdevice))
    nmdevice = nm_client_get_device_by_iface(nmclient, "wlan0");

  if (!nmdevice || !NM_IS_DEVICE(nmdevice))
    DBG("WifiStatusNM: failed to connect to nmdevice wlan0 over dbus");

  return nmclient;
}

/* Borrowed from network-manager-applet src/utils/utils.c */
char *
utils_hash_ap (const GByteArray *ssid,
               NM80211Mode mode,
               guint32 flags,
               guint32 wpa_flags,
               guint32 rsn_flags)
{
        unsigned char input[66];

        memset (&input[0], 0, sizeof (input));

        if (ssid)
                memcpy (input, ssid->data, ssid->len);

        if (mode == NM_802_11_MODE_INFRA)
                input[32] |= (1 << 0);
        else if (mode == NM_802_11_MODE_ADHOC)
                input[32] |= (1 << 1);
        else
                input[32] |= (1 << 2);

        /* Separate out no encryption, WEP-only, and WPA-capable */
        if (  !(flags & NM_802_11_AP_FLAGS_PRIVACY)
            && (wpa_flags == NM_802_11_AP_SEC_NONE)
            && (rsn_flags == NM_802_11_AP_SEC_NONE))
                input[32] |= (1 << 3);
        else if (   (flags & NM_802_11_AP_FLAGS_PRIVACY)
                 && (wpa_flags == NM_802_11_AP_SEC_NONE)
                 && (rsn_flags == NM_802_11_AP_SEC_NONE))
                input[32] |= (1 << 4);
        else if (   !(flags & NM_802_11_AP_FLAGS_PRIVACY)
                 &&  (wpa_flags != NM_802_11_AP_SEC_NONE)
                 &&  (rsn_flags != NM_802_11_AP_SEC_NONE))
                input[32] |= (1 << 5);
        else
                input[32] |= (1 << 6);

        /* duplicate it */
        memcpy (&input[33], &input[0], 32);
        return g_compute_checksum_for_data (G_CHECKSUM_MD5, input, sizeof (input));
}

bool resolveAPSecurity(NMAccessPoint *ap) {
  //FIXME: Assumes all security types equal
  return (
    nm_access_point_get_flags(ap) == NM_802_11_AP_FLAGS_PRIVACY ||
    nm_access_point_get_wpa_flags(ap) != NM_802_11_AP_SEC_NONE ||
    nm_access_point_get_rsn_flags(ap) != NM_802_11_AP_SEC_NONE
  );
}

WifiAccessPoint *createNMWifiAccessPoint(NMAccessPoint *ap) {
  const GByteArray *ssid = nm_access_point_get_ssid(ap);
  //GBytes *ssid = nm_access_point_get_ssid(ap);
  char *ssid_str = NULL, *ssid_hex_str = NULL;
  bool security = resolveAPSecurity(ap);

  /* Convert to strings */
  if (ssid) {
    const guint8 *ssid_data = ssid->data;
    gsize ssid_len = ssid->len;

    //ssid_data = (const guint8 *) g_bytes_get_data(ssid, &ssid_len);
    ssid_str = nm_utils_ssid_to_utf8(ssid);
  }

  if (!ssid_str || !ssid)
    DBG("libnm conversion of ssid to utf8 failed, using empty string");

  return new WifiAccessPoint {
    !ssid_str ? "" : ssid_str,
    nm_access_point_get_strength(ap),
    security,
    utils_hash_ap(nm_access_point_get_ssid(ap),
                  nm_access_point_get_mode(ap),
                  nm_access_point_get_flags(ap),
                  nm_access_point_get_wpa_flags(ap),
                  nm_access_point_get_rsn_flags(ap)),
  };
}

ScopedPointer<WifiAccessPoint> getNMConnectedAP(NMDeviceWifi *wdev) {
  NMAccessPoint *ap = nm_device_wifi_get_active_access_point(wdev);

  if (!wdev || !ap) {
    DBG(__func__ << ": no NMAccessPoint found!");
    return nullptr;
  }

  return createNMWifiAccessPoint(ap);
}

NMListener::NMListener() : Thread("NMListener Thread") {}

NMListener::~NMListener() {
  DBG(__func__ << ": cleanup thread");
  if (isThreadRunning()) {
    signalThreadShouldExit();
    notify();
    stopThread(2000);
  }
}

static void handle_wireless_enabled(WifiStatusNM *wifiStatus) {
  DBG("SIGNAL: " << NM_CLIENT_WIRELESS_ENABLED << ": changed! ");
  if (wifiStatus->isEnabled())
    wifiStatus->handleWirelessEnabled();
}

static void handle_wireless_connected(WifiStatusNM *wifiStatus) {
  DBG("SIGNAL: " << NM_DEVICE_STATE << ": changed! ");
  wifiStatus->handleWirelessConnected();
}

static void handle_active_access_point(WifiStatusNM *wifiStatus) {
  DBG("SIGNAL: " << NM_DEVICE_WIFI_ACTIVE_ACCESS_POINT << ": changed! ");
  wifiStatus->handleConnectedAccessPoint();
}

static void handle_changed_access_points(WifiStatusNM *wifiStatus) {
  DBG("SIGNAL: access-point-added | access-point-removed: changed! ");
  if (!wifiStatus->isEnabled())
    wifiStatus->handleWirelessEnabled();
}

static void handle_add_and_activate_finish(NMClient *client,
                                           NMActiveConnection *active,
                                           const char* path,
                                           GError *err,
                                           gpointer user_data) {
  WifiStatusNM *wifiStatus = (WifiStatusNM *) user_data;
  /*
  NMActiveConnection *active;
  GError *err = NULL;

  active = nm_client_add_and_activate_connection_finish(NM_CLIENT(client), result, &err);
  */
  if (err) {
    DBG("WifiStatusNM: failed to add/activate connection!");
    DBG("WifiStatusNM::" << __func__ << ": " << err->message);
    wifiStatus->handleWirelessConnected();
    //g_error_free(err);
  }
}


void NMListener::initialize(WifiStatusNM *status, NMClient *client) {
  nm = client;
  wifiStatus = status;
}

void NMListener::run() {
  NMDevice *dev = nm_client_get_device_by_iface(nm, "wlan0");
  context = g_main_context_default();
  //context = g_main_context_new();
  loop = g_main_loop_new(context, false);
  //g_main_context_invoke(context, initialize_in_context, status);

  g_signal_connect_swapped(nm, "notify::" NM_CLIENT_WIRELESS_ENABLED,
    G_CALLBACK(handle_wireless_enabled), wifiStatus);

  g_signal_connect_swapped(dev, "notify::" NM_DEVICE_STATE,
    G_CALLBACK(handle_wireless_connected), wifiStatus);

  g_signal_connect_swapped(NM_DEVICE_WIFI(dev), "notify::" NM_DEVICE_WIFI_ACTIVE_ACCESS_POINT,
    G_CALLBACK(handle_active_access_point), wifiStatus);

  g_signal_connect_swapped(NM_DEVICE_WIFI(dev), "access-point-added",
    G_CALLBACK(handle_changed_access_points), wifiStatus);

  g_signal_connect_swapped(NM_DEVICE_WIFI(dev), "access-point-removed",
    G_CALLBACK(handle_changed_access_points), wifiStatus);

  while (!threadShouldExit()) {
    {
      const MessageManagerLock mmLock;
      bool dispatched = g_main_context_iteration(context, false);
    }
    wait(LIBNM_ITERATION_PERIOD);
  }

  g_main_loop_unref(loop);
  g_main_context_unref(context);
}

OwnedArray<WifiAccessPoint> WifiStatusNM::nearbyAccessPoints() {
  NMDeviceWifi *wdev;
  const GPtrArray *ap_list;
  OwnedArray<WifiAccessPoint> accessPoints;

  wdev = NM_DEVICE_WIFI(nmdevice);
  //nm_device_wifi_request_scan(wdev, NULL, NULL);

  ap_list =  nm_device_wifi_get_access_points(wdev);
  if (ap_list != NULL) {
    std::map<String, WifiAccessPoint *> uniqueAPs;
    for (int i = 0; i < ap_list->len; i++) {
      NMAccessPoint *ap = (NMAccessPoint *) g_ptr_array_index(ap_list, i);
      auto created_ap = createNMWifiAccessPoint(ap);

      /*FIXME: dropping hidden (no ssid) networks until gui supports it*/
      if (created_ap->ssid.length() == 0)
        continue;

      if (uniqueAPs.find(created_ap->hash) == uniqueAPs.end())
        uniqueAPs[created_ap->hash] = created_ap;
      else
        if (uniqueAPs[created_ap->hash]->signalStrength < created_ap->signalStrength)
	  uniqueAPs[created_ap->hash] = created_ap;
    }
    for (const auto entry : uniqueAPs)
      accessPoints.add(entry.second);
  }

  DBG(__func__ << ": found " << accessPoints.size() << " AccessPoints");
  return accessPoints;
}

ScopedPointer<WifiAccessPoint> WifiStatusNM::connectedAccessPoint() const {
  if (connectedAP == nullptr)
    return nullptr;

  return ScopedPointer<WifiAccessPoint>(new WifiAccessPoint(*connectedAP));
}

bool WifiStatusNM::isEnabled() const {
  return enabled;
}

bool WifiStatusNM::isConnected() const {
  return connected;
}

void WifiStatusNM::addListener(Listener* listener) {
  listeners.add(listener);
  DBG("WifiStatusNM::" << __func__ << " numListeners = " << listeners.size());
}

void WifiStatusNM::clearListeners() {
  listeners.clear();
  DBG("WifiStatusNM::" << __func__ << " numListeners = " << listeners.size());
}

// TODO: direct action should not be named set, e.g. enable/disable/disconnect
// otherwise easily confused with setters thats wrap members, which are slightly different idiom
void WifiStatusNM::setEnabled() {
  if (!enabled) {
    for (const auto& listener : listeners)
      listener->handleWifiBusy();
    nm_client_wireless_set_enabled(nmclient, true);
  }
}

void WifiStatusNM::setDisabled() {
  if (enabled) {
    for (const auto& listener : listeners)
      listener->handleWifiBusy();
    nm_client_wireless_set_enabled(nmclient, false);
  }
}

void WifiStatusNM::handleWirelessEnabled() {
  enabled = nm_client_wireless_get_enabled(nmclient);
  DBG("WifiStatusNM::" << __func__ << " changed to " << enabled);

  //FIXME: Force and wait for a scan after enable
  if (enabled)
    for (const auto& listener : listeners)
      listener->handleWifiEnabled();
  else
    for (const auto& listener : listeners)
      listener->handleWifiDisabled();
}

void removeNMConnection(NMDevice *nmdevice, NMActiveConnection *conn) {
  if (conn) {
    const char *ac_uuid = nm_active_connection_get_uuid(conn);
    const GPtrArray *avail_cons = nm_device_get_available_connections(nmdevice);

    for (int i = 0; avail_cons && (i < avail_cons->len); i++) {
      NMRemoteConnection *candidate = (NMRemoteConnection *) g_ptr_array_index(avail_cons, i);
      const char *test_uuid = nm_connection_get_uuid(NM_CONNECTION(candidate));

      if (g_strcmp0(ac_uuid, test_uuid) == 0) {
        GError *err = NULL;
        nm_remote_connection_delete(candidate, NULL, &err);
        if (err) {
          DBG("WifiStatusNM: failed to remove active connection!");
          DBG("WifiStatusNM::" << __func__ << ": " << err->message);
          g_error_free(err);
        }
        break;
      }
    }
  }
}

void WifiStatusNM::handleWirelessConnected() {
  NMDeviceState state = nm_device_get_state(nmdevice);
  DBG("WifiStatusNM::" << __func__ << " changed to " << state
      << " while connecting = " << connecting);

  switch (state) {
    case NM_DEVICE_STATE_ACTIVATED:
      if (connected)
        break;

      handle_active_access_point(this);
      connected = true;
      connecting = false;
      DBG("WifiStatus::" << __func__ << " - connected");
      for(const auto& listener : listeners)
        listener->handleWifiConnected();
      break;

    case NM_DEVICE_STATE_PREPARE:
    case NM_DEVICE_STATE_CONFIG:
    case NM_DEVICE_STATE_IP_CONFIG:
    case NM_DEVICE_STATE_IP_CHECK:
    case NM_DEVICE_STATE_SECONDARIES:
      /* No state change for now, wait for connection to complete/fail */
      break;

    case NM_DEVICE_STATE_NEED_AUTH:
      if (connecting) {
	NMActiveConnection *conn = nm_client_get_activating_connection(nmclient);
        removeNMConnection(nmdevice, conn);
      }
      /* FIXME: let this drop into the general failed case for now
       *        eventually this should prompt the user
       */
    case NM_DEVICE_STATE_DISCONNECTED:
    case NM_DEVICE_STATE_DEACTIVATING:
    case NM_DEVICE_STATE_FAILED:
      if (connecting) {
        connected = false;
        connecting = false;
        DBG("WifiStatus::" << __func__ << " - failed");
        for(const auto& listener : listeners)
          listener->handleWifiFailedConnect();
        break;
      }

      if (!connected)
        break;

      handle_active_access_point(this);
      connected = false;

      for(const auto& listener : listeners)
        listener->handleWifiDisconnected();
      break;

    case NM_DEVICE_STATE_UNKNOWN:
    case NM_DEVICE_STATE_UNMANAGED:
    case NM_DEVICE_STATE_UNAVAILABLE:
    default:
      if (connecting || connected) {
        std::cerr << "WifiStatusNM::" << __func__
                  << ": wlan0 device entered unmanaged state: " << state << std::endl;
        handle_active_access_point(this);
        connected = false;
        connecting = false;
        for(const auto& listener : listeners)
          listener->handleWifiDisconnected();
      }
  }
}

void WifiStatusNM::handleConnectedAccessPoint() {
  DBG("WifiStatusNM::" << __func__ << " changed active AP");
  connectedAP = getNMConnectedAP(NM_DEVICE_WIFI(nmdevice));
  if (connectedAP)
    DBG("WifiStatusNM::" << __func__ << " ssid = " << connectedAP->ssid);
  else
    DBG("WifiStatusNM::" << __func__ << " connectedAP = NULL");
}

bool isValidWEPKeyFormat(String key) {
  return (key.length() == 10) || (key.length() == 26);
}

bool isValidWEPPassphraseFormat(String phrase) {
  return (phrase.length() == 5) || (phrase.length() == 13);
}

void WifiStatusNM::setConnectedAccessPoint(WifiAccessPoint *ap, String psk) {
  ScopedPointer<StringArray> cmd;

  for (const auto& listener : listeners)
    listener->handleWifiBusy();

  // disconnect if no ap provided
  if (ap == nullptr) {
    NMActiveConnection *conn = nm_device_get_active_connection(nmdevice);
    removeNMConnection(nmdevice, conn);

    return;
  }
  // try to connect to ap, dispatch events on success and failure
  else {
    NMConnection *connection = NULL;
    NMSettingWireless *s_wifi = NULL;
    NMSettingWirelessSecurity *s_wsec = NULL;
    const char *nm_ap_path = NULL;
    const GPtrArray *ap_list;
    NMAccessPoint *candidate_ap;

    //FIXME: expand WifiAccessPoint struct to know which NMAccessPoint it is
    ap_list = nm_device_wifi_get_access_points(NM_DEVICE_WIFI(nmdevice));
    if (ap_list == NULL)
      return;

    for (int i = 0; i < ap_list->len; i++) {
      const char *candidate_hash;
      candidate_ap = (NMAccessPoint *) g_ptr_array_index(ap_list, i);
      candidate_hash = utils_hash_ap(nm_access_point_get_ssid(candidate_ap),
                                     nm_access_point_get_mode(candidate_ap),
                                     nm_access_point_get_flags(candidate_ap),
                                     nm_access_point_get_wpa_flags(candidate_ap),
                                     nm_access_point_get_rsn_flags(candidate_ap));

      if (ap->hash == candidate_hash) {
        nm_ap_path = nm_object_get_path(NM_OBJECT(candidate_ap));
        break;
      }
    }

    if (!nm_ap_path)
      return;

    connecting = true;

    connection = nm_connection_new();
    s_wifi = (NMSettingWireless *) nm_setting_wireless_new();
    nm_connection_add_setting(connection, NM_SETTING(s_wifi));
    g_object_set(s_wifi,
                 NM_SETTING_WIRELESS_SSID, nm_access_point_get_ssid(candidate_ap),
                 NM_SETTING_WIRELESS_HIDDEN, false,
                 NULL);

    if (!psk.isEmpty()) {
      s_wsec = (NMSettingWirelessSecurity *) nm_setting_wireless_security_new();
      nm_connection_add_setting(connection, NM_SETTING(s_wsec));

      if (nm_access_point_get_wpa_flags(candidate_ap) == NM_802_11_AP_SEC_NONE &&
          nm_access_point_get_rsn_flags(candidate_ap) == NM_802_11_AP_SEC_NONE) {
        /* WEP */
        nm_setting_wireless_security_set_wep_key(s_wsec, 0, psk.toRawUTF8());
	if (isValidWEPKeyFormat(psk))
          g_object_set(G_OBJECT(s_wsec), NM_SETTING_WIRELESS_SECURITY_WEP_KEY_TYPE,
                       NM_WEP_KEY_TYPE_KEY, NULL);
	else if (isValidWEPPassphraseFormat(psk))
          g_object_set(G_OBJECT(s_wsec), NM_SETTING_WIRELESS_SECURITY_WEP_KEY_TYPE,
                       NM_WEP_KEY_TYPE_PASSPHRASE, NULL);
	else
	  DBG("User input invalid WEP Key type, psk.length() = " << psk.length()
              << ", not in [5,10,13,26]");
      } else {
        g_object_set(s_wsec, NM_SETTING_WIRELESS_SECURITY_PSK, psk.toRawUTF8(), NULL);
      }
    }

    nm_client_add_and_activate_connection(nmclient,
                                                connection,
                                                nmdevice,
                                                nm_ap_path,
                                                handle_add_and_activate_finish,
                                                this);
  }
}

void WifiStatusNM::setDisconnected() {
  setConnectedAccessPoint(nullptr);
}

void WifiStatusNM::initializeStatus() {
  connectedAP = nullptr;
  connected = false;

  if (!this->connectToNetworkManager())
    DBG("WifiStatusNM: failed to connect to nmclient over dbus");

  nmlistener = new NMListener();
  nmlistener->initialize(this, nmclient);
  nmlistener->startThread();

  enabled = nm_client_wireless_get_enabled(nmclient);

  if (!enabled)
    return;

  connected = nm_device_get_state(nmdevice) == NM_DEVICE_STATE_ACTIVATED;

  if (connected)
    connectedAP = getNMConnectedAP(NM_DEVICE_WIFI(nmdevice));
}

#endif // LINUX
