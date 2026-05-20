#include "application.h"
#include "trace.h"

// External secrets – located outside this project in ../_secrets/
// Include path is set via build_flags in platformio.ini: -I ../_secrets
#include "WifiSecret.h"
#include "OtaSecret.h"

Application::Application(WifiManager &wifiManager, OtaManager &otaManager,
                         SystemConfig &systemConfig)
    : _wifiManager(wifiManager), _otaManager(otaManager),
      _systemConfig(systemConfig), _lastStatusPrint(0), _startupWaitStart(0),
      _startupState(StartupState::WAITING_FOR_WIFI)
{
}

void Application::setup()
{
  Trace::log(TraceLevel::INFO, "Application setup started");

  // We intentionally initialize WiFi early in setup().
  // Rationale:
  // - Network services (OTA, telemetry, remote diagnostics) depend on WiFi.
  // - The WifiManager keeps reconnect logic internally and continues in loop().
  // Credentials come from external secret headers to keep sensitive values
  // outside the repository.
  _wifiManager.setup(WIFI_SSID, WIFI_PWD, DEVICE_NAME);
  
  // Non-blocking startup marker:
  // Instead of waiting here (blocking setup for seconds), we record the start
  // timestamp and finalize network-dependent startup steps from loop().
  // This keeps boot responsive and avoids long initialization stalls.
  _startupWaitStart = millis();
  _startupState = StartupState::WAITING_FOR_WIFI;
  Trace::log(TraceLevel::INFO,
             "Startup is non-blocking, waiting for WiFi in main loop");

  Trace::log(TraceLevel::INFO, "Application setup complete");
  _lastStatusPrint = millis();
}

void Application::loop()
{
  unsigned long currentTime = millis();

  // First, progress startup state machine (WiFi wait -> OTA init -> running).
  // Keeping this at the top ensures startup completion is checked every cycle.
  handleStartup();

  // Service WiFi state machine and reconnect behavior.
  _wifiManager.loop();

  // OTA handler must run regularly so the device can receive update packets.
  // If this is starved, OTA can timeout/fail.
  _otaManager.loop();

  // Check for connection events
  if (_wifiManager.consumeConnectedEvent())
  {
    Trace::log(TraceLevel::INFO, "WiFi connected event");
  }

  if (_wifiManager.consumeDisconnectedEvent())
  {
    Trace::log(TraceLevel::WARNING, "WiFi disconnected event");
  }

  // Print status periodically
  if (currentTime - _lastStatusPrint >= STATUS_PRINT_INTERVAL_MS)
  {
    _lastStatusPrint = currentTime;
    if (_wifiManager.isConnected())
    {
      if (_otaManager.isEnabled())
      {
        Trace::log(TraceLevel::INFO, "WiFi connected, OTA enabled");
      }
      else
      {
        Trace::log(TraceLevel::INFO, "WiFi connected, OTA disabled");
      }
    }
    else
    {
      Trace::log(TraceLevel::INFO, "WiFi disconnected");
    }
  }
}

void Application::handleStartup()
{
  // Once startup is complete, this function becomes a fast no-op.
  if (_startupState != StartupState::WAITING_FOR_WIFI)
  {
    return;
  }

  // As soon as WiFi is available, initialize OTA exactly once.
  // Doing this lazily in loop() avoids blocking setup() on network timing.
  if (_wifiManager.isConnected())
  {
    Trace::log(TraceLevel::INFO,
               "Startup: WiFi available, initializing OTA");
    _otaManager.setup(DEVICE_NAME, OTA_PASSWORD);
    _startupState = StartupState::RUNNING;
    return;
  }

  // Guard against waiting forever for initial WiFi:
  // The application continues even without immediate connectivity.
  // WifiManager will still keep reconnecting in the background.
  if (millis() - _startupWaitStart > WIFI_INITIAL_CONNECT_TIMEOUT_MS)
  {
    Trace::log(TraceLevel::WARNING,
               "Initial WiFi connection timeout; continuing non-blocking");
    _startupState = StartupState::RUNNING;
  }
}
