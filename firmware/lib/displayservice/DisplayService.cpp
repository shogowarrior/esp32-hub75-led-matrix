#include "DisplayService.h"
#include "constants.h"
#include "Logger.h"
#include "ConfigManager.h"
#include <ArduinoJson.h>

// Initialize the static instance
DisplayService DisplayService::instance;


// Get the singleton instance
DisplayService& DisplayService::getInstance() {
    return instance;
}

DisplayService::DisplayService() : display(nullptr), currentBrightness(0) {
   LOG_DEBUG("DisplayService: Instance created");
}

DisplayService::~DisplayService() {
   if (display != nullptr) {
     delete display;
     display = nullptr;
   }
   LOG_DEBUG("DisplayService: Cleanup completed");
}

bool DisplayService::initialize() {
   LOG_INFO("DisplayService: Initializing...");

   // Initialize hardware with dynamic pin configuration
   JsonObject pins = ConfigManager::getPins(PINS_DISPLAY);
    if (pins.isNull()) {
        LOG_ERROR("SD pin configuration not found");
        return false;
    }
   HUB75_I2S_CFG::i2s_pins _pins = {
       pins[PIN_R1].as<int8_t>(),
       pins[PIN_G1].as<int8_t>(),
       pins[PIN_B1].as<int8_t>(),
       pins[PIN_R2].as<int8_t>(),
       pins[PIN_G2].as<int8_t>(),
       pins[PIN_B2].as<int8_t>(),
       pins[PIN_A].as<int8_t>(),
       pins[PIN_B].as<int8_t>(),
       pins[PIN_C].as<int8_t>(),
       pins[PIN_D].as<int8_t>(),
       pins[PIN_E].as<int8_t>(),
       pins[PIN_LAT].as<int8_t>(),
       pins[PIN_OE].as<int8_t>(),
       pins[PIN_CLK].as<int8_t>()
   };

   HUB75_I2S_CFG mxconfig(PANEL_HEIGHT, PANEL_WIDTH, PANELS_NUMBER, _pins);

   display = new MatrixPanel_I2S_DMA(mxconfig);
   if (not display->begin()) {
     LOG_ERROR("I2S memory allocation failed");
     return false;
   }
   display->setBrightness(DEFAULT_BRIGHTNESS); // Set initial brightness
   return true;
}

uint8_t DisplayService::getBrightness() {
    if (!display) {
        LOG_ERROR("Display not initialized");
        return 0;
    }
    return currentBrightness;
}

void DisplayService::setBrightness(uint8_t brightness) {
    if (!display) {
        LOG_ERROR("Display not initialized");
        return;
    }

    // Validate brightness range
    if (brightness > 255) {
        brightness = 255;
    }

    display->setBrightness(brightness);
    currentBrightness = brightness;
    LOG_INFO("Brightness set to %d", brightness);
}

// ============================================================================
// Display Effects
// ============================================================================

void DisplayService::runTestPattern() {
   LOG_INFO("Running test pattern");

   if (display) {
     // Run the test pattern
     display->fillScreenRGB888(127, 0, 0);
     delay(TEST_PATTERN_DELAY_MS);
     display->fillScreenRGB888(0, 127, 0);
     delay(TEST_PATTERN_DELAY_MS);
     display->fillScreenRGB888(0, 0, 127);
     delay(TEST_PATTERN_DELAY_MS);
     display->fillScreenRGB888(127, 127, 127);
     delay(TEST_PATTERN_DELAY_MS);
     display->fillScreenRGB888(0, 0, 0);
     LOG_INFO("Test pattern completed");
   }
}

// ============================================================================
// Power Management
// ============================================================================

/**
 * @brief Check if the display is powered on
 * @return true if display is on, false otherwise
 */
bool DisplayService::isPowerOn() const
{
    // This is a simple implementation - in a real application, you might need
    // to track the power state or check the hardware
    return true;
}

/**
 * @brief Set the power state of the display
 * @param state true to turn on, false to turn off
 */
void DisplayService::setPowerState(bool state)
{
    // In a real implementation, this would control the display power
    // For now, we'll just log the action
    LOG_INFO("Power state set to %s", state ? "ON" : "OFF");
}
