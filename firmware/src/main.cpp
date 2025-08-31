#include "DisplayService.h"
#include "FSUtils.h"
#include "Logger.h"
#include "Service.h"
#include "ConfigManager.h"
#include "constants.h"

// Flag to enable/disable test mode - determined by configuration
bool TEST = true;

void setup() {
   // Initialize serial communication for debugging
   Serial.begin(SERIAL_BAUD_RATE);
   LOG_MESSAGE("SYSTEM STARTUP",
               "ESP32 HUB75 LED Matrix - Initializing all services...");

  //  // Determine test mode from configuration
  //  ConfigManager& config = ConfigManager::getInstance();
  //  if (config.loadConfiguration()) {
  //     JsonDocument& cfg = config.getConfig();
  //     TEST = cfg["system"]["debugMode"].as<bool>();
  //     LOG_INFO("Test mode: %s", TEST ? "ENABLED" : "DISABLED");
  //  } else {
  //     // If config loading fails, default to test mode for debugging
  //     TEST = true;
  //     LOG_WARNING("Configuration loading failed, defaulting to test mode");
  //  }

  if (TEST) {
    // Only initialize DisplayService for testing
    LOG_INFO("Running in test mode - DisplayService only");
    DisplayService::getInstance().initialize();
  } else {
    // Start all services (WiFi, OTA, Web Server, LED Matrix, etc.)
    LOG_INFO("Starting services...");
    Service service;
    if (!service.initialize()) {
      LOG_CRITICAL("Service initialization failed!");
      while (true) {  // Halt execution
        delay(DIRECTORY_LISTING_DELAY_MS);
      }
    }
  }


}

void loop() {
  if (TEST) {
    // Run test pattern continuously in loop
    // DisplayService::getInstance().runTestPattern();

    // List SD card contents as part of test mode
    FSUtils::begin(FSType::SD);
    // FSUtils::_listDir(FSType::SD, "/gifs/", true, true);

    File root = SD.open("/gifs");
    if (root) {
      while (true) {
        File entry = root.openNextFile();
        if (!entry) break;
        Serial.println(entry.name());  // list files/folders at root
        entry.close();  // Close each entry file handle
      }
      root.close();  // Close the root directory handle
    } else {
      Serial.println("Failed to open /gifs directory");
    }
    delay(DIRECTORY_LISTING_DELAY_MS);  // Delay between directory listings
  }
}