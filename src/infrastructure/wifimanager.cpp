#include "wifimanager.h"
#include "reconnectpolicy.h"

// Initialize the static instance pointer
WifiManager *WifiManager::_instance = nullptr;

WifiManager::WifiManager()
{
  // WiFi event API requires a static callback function.
  // We bridge that static callback to this instance via _instance.
  // Therefore only one active WifiManager instance is supported.
  if (_instance != nullptr)
  {
    Trace::log(TraceLevel::ERROR,
               "WifiManager instance already exists; replacing instance");
  }
  _instance = this;
}

WifiManager::~WifiManager()
{
  // Defensive cleanup: if this object is destroyed, clear static forwarding
  // target so callback invocations never dereference an invalid instance.
  if (_instance == this)
  {
    _instance = nullptr;
  }
}

void WifiManager::setup(String ssid, String password, String clientName)
{
  // Store credentials/identity for reconnect attempts and diagnostics.
  _ssid = ssid;
  _password = password;
  _clientName = clientName;

  // Configure station mode and hostname before connecting.
  // Hostname helps identify this node in router UI and mDNS environments.
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(clientName.c_str());
  // Re-register handler idempotently to avoid duplicate callback registrations
  // after repeated setup calls.
  WiFi.removeEvent(staticWifiEventHandler);
  WiFi.onEvent(staticWifiEventHandler);

  // Kick off first connect explicitly.
  // We do not rely on implicit connect behavior or event ordering side effects.
  WiFi.begin(_ssid.c_str(), _password.c_str());
  _wifiConnectStartTime = millis();
  _wifiState = WIFI_CONNECTING;
  _nextReconnectAttemptTime = _wifiConnectStartTime;

  Trace::log(TraceLevel::DEBUG, "WiFi setup complete.");
}

// Static WiFi event handler
void WifiManager::staticWifiEventHandler(WiFiEvent_t event)
{
  // Forward to the instance method if instance exists
  if (_instance)
  {
    _instance->wifiEvent(event);
  }
}

void WifiManager::wifiEvent(WiFiEvent_t event)
{
  // Keep event handler lightweight: only update state and schedule work.
  // Actual reconnect actions happen in loop()/manageConnection().
  switch (event)
  {
  case SYSTEM_EVENT_STA_START:
    Trace::log(TraceLevel::INFO, "WiFi started");
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
  {
    const IPAddress ip = WiFi.localIP();
    Trace::logf(TraceLevel::INFO, "WiFi connected, IP: %u.%u.%u.%u", ip[0],
                ip[1], ip[2], ip[3]);
    _wifiState = WIFI_CONNECTED;
    _connectedEventPending = true;
    _disconnectedEventPending = false;
    _reconnectAttempt = 0;
    break;
  }
  case SYSTEM_EVENT_STA_DISCONNECTED:
    Trace::log(TraceLevel::INFO,
               "WiFi disconnected, attempting to reconnect...");
    _wifiState = WIFI_DISCONNECTED;
    _disconnectedEventPending = true;
    _connectedEventPending = false;
    // Schedule immediate reconnect attempt; subsequent attempts use backoff.
    _nextReconnectAttemptTime = millis();
    break;
  default:
    break;
  }
}

bool WifiManager::consumeConnectedEvent()
{
  bool hadEvent = _connectedEventPending;
  _connectedEventPending = false;
  return hadEvent;
}

bool WifiManager::consumeDisconnectedEvent()
{
  bool hadEvent = _disconnectedEventPending;
  _disconnectedEventPending = false;
  return hadEvent;
}

bool WifiManager::loop()
{
  // Safety net:
  // If event delivery misses a disconnect, still detect link loss by polling
  // WiFi.status() and transition to DISCONNECTED state.
  if (_wifiState == WIFI_CONNECTED && WiFi.status() != WL_CONNECTED)
  {
    Trace::log(TraceLevel::WARNING,
               "WiFi link lost (status poll), scheduling reconnect");
    _wifiState = WIFI_DISCONNECTED;
    _disconnectedEventPending = true;
    _connectedEventPending = false;
    _nextReconnectAttemptTime = millis();
  }

  // Fast-path: transition CONNECTING -> CONNECTED as soon as link is up.
  if (_wifiState == WIFI_CONNECTING && WiFi.status() == WL_CONNECTED)
  {
    Trace::logf(TraceLevel::DEBUG,
                "WiFi connected after connection attempt %u",
                _reconnectAttempt);
    _wifiState = WIFI_CONNECTED;
    _reconnectAttempt = 0;
  }
  else if (_wifiState == WIFI_DISCONNECTED &&
           millis() >= _nextReconnectAttemptTime)
  {
    // Reconnect attempt is due according to scheduled backoff window.
    manageConnection();
  }

  // Return connection status: true if connected, false otherwise
  return _wifiState == WIFI_CONNECTED;
}

void WifiManager::manageConnection()
{
  // Retry budget is bounded to avoid infinite tight reconnect loops.
  if (_reconnectAttempt < WIFI_MAX_RECONNECT_ATTEMPTS)
  {
    Trace::log(TraceLevel::INFO, "Attempting to reconnect to WiFi...");
    WiFi.disconnect();
    WiFi.begin(_ssid.c_str(), _password.c_str());
    _wifiConnectStartTime = millis();
    _reconnectAttempt++;

    // Random jitter prevents synchronized reconnect storms across many devices.
    const uint32_t jitter = esp_random() % (WIFI_RECONNECT_JITTER_MS + 1);
    // Exponential backoff keeps network pressure low during outages.
    const uint32_t reconnectDelayMs = ReconnectPolicy::computeDelayMs(
        _reconnectAttempt, WIFI_RECONNECT_BASE_DELAY_MS,
        WIFI_RECONNECT_MAX_DELAY_MS, jitter);
    _nextReconnectAttemptTime = _wifiConnectStartTime + reconnectDelayMs;

    Trace::logf(TraceLevel::DEBUG, "Next reconnect in ms: %lu",
                static_cast<unsigned long>(reconnectDelayMs));
  }
  else
  {
    Trace::log(TraceLevel::ERROR,
               "Max reconnection attempts reached");
#if WIFI_RESTART_ON_RECONNECT_FAILURE
    // Optional self-healing mode: hard reset after exhausting retries.
    // Useful for unattended deployments that prefer automatic reboot cycles.
    Trace::log(TraceLevel::WARNING, "Restarting due to reconnect policy");
    delay(1000);
    ESP.restart();
#else
    // Default diagnostics-first mode:
    // Stay alive for serial/remote diagnostics and retry again later.
    Trace::log(TraceLevel::WARNING,
               "Keeping device alive for diagnostics (no forced restart)");
    _nextReconnectAttemptTime = millis() + WIFI_RECONNECT_MAX_DELAY_MS;
    _reconnectAttempt = 0;
#endif
  }
}
