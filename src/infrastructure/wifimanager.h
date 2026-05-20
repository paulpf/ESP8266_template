// wifimanager.h
#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include "global_defines.h"
#include "iwificonnectivity.h"

class WifiManager : public IWifiConnectivity
{
public:
  WifiManager(/* args */);
  ~WifiManager();
  void setup(String ssid, String password, String clientName);
  bool loop();
  void manageConnection();
  bool isConnected() const override
  {
    return _wifiState == WIFI_CONNECTED;
  }
  bool consumeConnectedEvent() override;
  bool consumeDisconnectedEvent() override;

  // Static WiFi event handler that will be used with WiFi.onEvent
  static void staticWifiEventHandler(WiFiEvent_t event);

private:
  // Static pointer to the instance (similar to MqttManager's implementation)
  static WifiManager *_instance;

  // Instance method to handle WiFi events
  void wifiEvent(WiFiEvent_t event);

  String _ssid;
  String _password;
  String _clientName;
  unsigned long _nextReconnectAttemptTime = 0;
  enum WifiState
  {
    WIFI_DISCONNECTED,
    WIFI_CONNECTING,
    WIFI_CONNECTED
  };
  WifiState _wifiState = WIFI_DISCONNECTED;
  unsigned long _wifiConnectStartTime = 0;
  uint8_t _reconnectAttempt = 0;
  bool _connectedEventPending = false;
  bool _disconnectedEventPending = false;
};

#endif // WIFIMANAGER_H
