#ifndef DISPLAY_SERVICE_H
#define DISPLAY_SERVICE_H

/**
 * @file DisplayService.h
 * @brief LED matrix display service for ESP32 HUB75 LED Matrix
 *
 * Manages the LED matrix display hardware, including initialization,
 * brightness control, power management, and test patterns.
 */

#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

class DisplayService {
public:
    // =============================================================================
    // Singleton Management
    // =============================================================================

    /**
     * @brief Get the singleton instance of DisplayService
     *
     * @return DisplayService& Reference to the singleton instance
     */
    static DisplayService& getInstance();

    // =============================================================================
    // Initialization and Control
    // =============================================================================
    bool initialize();
    void runTestPattern();

    // =============================================================================
    // Brightness Management
    // =============================================================================
    uint8_t getBrightness();
    void setBrightness(uint8_t brightness);

    // =============================================================================
    // Power Management
    // =============================================================================
    bool isPowerOn() const;
    void setPowerState(bool state);

    // =============================================================================
    // Display Access
    // =============================================================================
    MatrixPanel_I2S_DMA *getDisplay() { return display; };

private:
    // =============================================================================
    // Private Members
    // =============================================================================

    // Private constructor to prevent direct instantiation
    DisplayService();
    ~DisplayService();

    // Static instance
    static DisplayService instance;

    MatrixPanel_I2S_DMA *display;  //< Pointer to LED matrix display object
    uint8_t currentBrightness;     //< Current display brightness level
};

#endif // DISPLAY_SERVICE_H