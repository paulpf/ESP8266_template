#ifndef SYSTEMCONFIG_H
#define SYSTEMCONFIG_H

#include "config.h"

struct SystemConfig
{
  // Watchdog timeout to reset system on lockup
  unsigned long watchdogTimeoutMs = WATCHDOG_TIMEOUT;
  
  // Main loop interval for long-running tasks (e.g., periodic telemetry)
  unsigned long longIntervalMs = LONG_INTERVAL;
  
  // Main loop interval for medium-frequency tasks
  unsigned long middleIntervalMs = MIDDLE_INTERVAL;
  
  // Main loop interval for high-frequency tasks
  unsigned long shortIntervalMs = SHORT_INTERVAL;
};

#endif // SYSTEMCONFIG_H
