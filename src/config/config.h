#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// Minimal configuration for WiFi + OTA only

// Serial configuration
#define SERIAL_BAUD_RATE 115200
constexpr uint32_t SERIAL_STARTUP_DELAY_MS = 500;

// System timings (in milliseconds)
#define WATCHDOG_TIMEOUT 30000
#define LONG_INTERVAL 60000    // 1 minute
#define MIDDLE_INTERVAL 10000  // 10 seconds  
#define SHORT_INTERVAL 1000    // 1 second

// Trace level for logging
#define TRACE_LEVEL TraceLevel::INFO

// OTA configuration
#define ENABLE_OTA true
constexpr uint16_t OTA_PORT = 3232;
// Security default: fail closed if no OTA password is configured
#define OTA_ALLOW_INSECURE_NO_PASSWORD false

// Device identity
#define DEVICE_NAME "esp32-template"

// WiFi configuration
constexpr int WIFI_CONNECTION_TIMEOUT = 10000; // milliseconds
constexpr uint32_t WIFI_INITIAL_CONNECT_TIMEOUT_MS = 15000;
constexpr uint8_t WIFI_MAX_RECONNECT_ATTEMPTS = 10;
constexpr uint32_t WIFI_RECONNECT_BASE_DELAY_MS = 1000;
constexpr uint32_t WIFI_RECONNECT_MAX_DELAY_MS = 30000;
constexpr uint32_t WIFI_RECONNECT_JITTER_MS = 500;
// Keep device alive for diagnostics instead of forced reboot
#define WIFI_RESTART_ON_RECONNECT_FAILURE false

// Application loop behavior
constexpr uint32_t STATUS_PRINT_INTERVAL_MS = 30000;

#endif // CONFIG_H
