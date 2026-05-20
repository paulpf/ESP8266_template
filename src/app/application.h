#ifndef APPLICATION_H
#define APPLICATION_H

#include "wifimanager.h"
#include "otamanager.h"
#include "systemconfig.h"

class Application
{
public:
  Application(WifiManager &wifiManager, OtaManager &otaManager, 
              SystemConfig &systemConfig);

  void setup();
  void loop();

private:
  enum class StartupState
  {
    WAITING_FOR_WIFI,
    RUNNING
  };

  void handleStartup();

  WifiManager &_wifiManager;
  OtaManager &_otaManager;
  SystemConfig &_systemConfig;
  unsigned long _lastStatusPrint;
  unsigned long _startupWaitStart;
  StartupState _startupState;
};

#endif // APPLICATION_H
