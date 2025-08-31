#ifndef CONSTANTS_H
#define CONSTANTS_H

/**
 * @file constants.h
 * @brief Global constants and configuration for ESP32 LED Matrix Controller
 *
 * This file defines all system-wide constants including file paths,
 * hardware configuration, and default values used throughout the application.
 */

// =============================================================================
// File System Configuration
// =============================================================================

/** @brief Base path for GIF storage on LittleFS */
#define GIFS_BASE_PATH "/gifs"

/** @brief Path for default GIF file */
#define GIF_DEFAULT_PATH (GIFS_BASE_PATH "/current.gif")

/** @brief Path for configuration JSON file */
#define CONFIG_FILE "/config.json"

// =============================================================================
// Hardware Configuration
// =============================================================================

/** @brief Default LED matrix brightness level (0-255) */
#define DEFAULT_BRIGHTNESS 10

/** @brief Serial communication baud rate */
#define SERIAL_BAUD_RATE 115200

/** @brief Maximum time to play a single GIF in milliseconds */
#define MAX_GIF_PLAY_TIME 8000

/** @brief LED matrix panel width & height in pixels */
#define PANEL_WIDTH 64
#define PANEL_HEIGHT 64

/** @brief Number of LED matrix panels */
#define PANELS_NUMBER 1

// =============================================================================
// Network Configuration
// =============================================================================

/** @brief Default web server port */
#define DEFAULT_WEB_PORT 80

/** @brief Default web server file */
#define DEFAULT_FILE "index.html"

/** @brief Default file path */
#define DEFAULT_FILE_PATH "/" DEFAULT_FILE

/** @brief Maximum number of WiFi connection attempts */
#define MAX_WIFI_CONNECTION_ATTEMPTS 20

// =============================================================================
// FreeRTOS Task Configuration
// =============================================================================

/** @brief OTA task stack size in bytes */
#define OTA_TASK_STACK_SIZE 4096

/** @brief Display task stack size in bytes */
#define DISPLAY_TASK_STACK_SIZE 8192

/** @brief OTA task priority (0-24, higher = more priority) */
#define OTA_TASK_PRIORITY 2

/** @brief Display task priority (0-24, higher = more priority) */
#define DISPLAY_TASK_PRIORITY 1

// =============================================================================
// Timing Constants
// =============================================================================

/** @brief OTA update check interval in milliseconds */
#define OTA_CHECK_INTERVAL_MS 5000

/** @brief Display task update interval in milliseconds */
#define DISPLAY_UPDATE_INTERVAL_MS 1000

/** @brief Test pattern delay between colors in milliseconds */
#define TEST_PATTERN_DELAY_MS 1000

/** @brief Directory listing delay in milliseconds */
#define DIRECTORY_LISTING_DELAY_MS 1000

/** @brief HTTP cache control max age in seconds (1 hour) */
#define HTTP_CACHE_MAX_AGE_SECONDS 3600

// =============================================================================
// Config Key Constants
// =============================================================================

/** @brief Main config sections */
#define STATE "state"
#define SYSTEM "system"
#define NETWORK "network"


/** @brief Pins keys */
#define PINS_DISPLAY "display"
#define PINS_SD "sd"
#define PINS "pins"

/** @brief State keys */
#define BRIGHTNESS "brightness"
#define IS_POWER_ON "isPowerOn"
#define LAST_SELECTED_CATEGORY "lastSelectedCategory"
#define CATEGORY_PLAYBACK "categoryPlayback"

/** @brief System keys */
#define DEBUG_MODE "debugMode"
#define LOG_LEVEL "logLevel"
#define WEB_SERVER_PORT "webServerPort"
#define OTA_ENABLED "otaEnabled"

/** @brief Network keys */
#define WIFI_SSID "ssid"
#define WIFI_PASSWORD "password"
#define HOSTNAME "hostname"
#define STATIC_IP "staticIp"
#define LOCAL_IP "localIp"
#define GATEWAY "gateway"
#define SUBNET "subnet"
#define PRIMARY_DNS "primaryDns"
#define SECONDARY_DNS "secondaryDns"

/** @brief Display pins */
#define PIN_R1 "R1"
#define PIN_G1 "G1"
#define PIN_B1 "B1"
#define PIN_R2 "R2"
#define PIN_G2 "G2"
#define PIN_B2 "B2"
#define PIN_A "A"
#define PIN_B "B"
#define PIN_C "C"
#define PIN_D "D"
#define PIN_E "E"
#define PIN_CLK "CLK"
#define PIN_LAT "LAT"
#define PIN_OE "OE"

/** @brief SD pins */
#define PIN_CS "CS"
#define PIN_MOSI "MOSI"
#define PIN_MISO "MISO"
#define PIN_SCK "SCK"

#endif // CONSTANTS_H
