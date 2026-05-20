// Implementation example for Trace.cpp
#include "trace.h"
#include <stdarg.h>
#include <stdio.h>

void Trace::log(TraceLevel level, String message)
{
  // Only log if the message level is >= the configured trace level
  if (!shouldLog(level))
  {
    return;
  }

  String prefix;
  switch (level)
  {
  case TraceLevel::TRACE:
    prefix = "[TRACE] ";
    break;
  case TraceLevel::INFO:
    prefix = "[INFO] ";
    break;
  case TraceLevel::WARNING:
    prefix = "[WARNING] ";
    break;
  case TraceLevel::DEBUG:
    prefix = "[DEBUG] ";
    break;
  case TraceLevel::ERROR:
    prefix = "[ERROR] ";
    break;
  default:
    prefix = "[LOG] ";
    break;
  }

  Serial.println(prefix + message);
}

void Trace::logf(TraceLevel level, const char *format, ...)
{
  // Formatted logging variant:
  // avoids repeated dynamic String concatenation in hot paths.
  if (!shouldLog(level))
  {
    return;
  }

  const char *prefix = "[LOG] ";
  switch (level)
  {
  case TraceLevel::TRACE:
    prefix = "[TRACE] ";
    break;
  case TraceLevel::INFO:
    prefix = "[INFO] ";
    break;
  case TraceLevel::WARNING:
    prefix = "[WARNING] ";
    break;
  case TraceLevel::DEBUG:
    prefix = "[DEBUG] ";
    break;
  case TraceLevel::ERROR:
    prefix = "[ERROR] ";
    break;
  default:
    break;
  }

  // Fixed-size stack buffer keeps memory behavior predictable on MCU.
  // Messages longer than buffer are truncated by vsnprintf.
  char messageBuffer[160];
  va_list args;
  va_start(args, format);
  vsnprintf(messageBuffer, sizeof(messageBuffer), format, args);
  va_end(args);

  Serial.print(prefix);
  Serial.println(messageBuffer);
}

// Plotting function for debugging purposes
// This function will print the state of each zone to the Serial Monitor
// It is only enabled if ENABLE_ZONE_PLOTTING is defined
// Visualizing with the Serial Plotter in Arduino IDE or PlatformIO
void Trace::plotBoolState(String stateName, bool stateValue,
                          int valueToPlotForTrue)
{
#ifdef ENABLE_ZONE_PLOTTING
  Serial.print(">BoolState_" + stateName + ":");
  Serial.print(stateValue ? String(valueToPlotForTrue) : "0");
  Serial.println(" | np");
#endif
}

// Method to plot loop time for debugging purposes
void Trace::plotLoopTime(String loopName, int loopNameindex,
                         unsigned long loopTime)
{
#ifdef ENABLE_LOOP_TIME_PLOTTING
  Serial.print(">LoopTime_" + loopName + "_" + String(loopNameindex));
  Serial.print(":");
  Serial.print(loopTime);
  Serial.println(" | np");
#endif
}
