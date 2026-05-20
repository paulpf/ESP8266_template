#ifndef GLOBAL_DEFINES_H
#define GLOBAL_DEFINES_H

#include <Arduino.h>
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

#include <functional>
#include <map>
#include <algorithm>
#include <cmath>
#include <array>

#include "trace.h"
#include "config.h"

#endif // GLOBAL_DEFINES_H
