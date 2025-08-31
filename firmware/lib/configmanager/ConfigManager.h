#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

/**
 * @file ConfigManager.h
 * @brief Configuration management for ESP32 LED Matrix Controller
 *
 * This class provides centralized configuration loading and access to system settings
 * stored in JSON format on the LittleFS filesystem.
 */

#include <Arduino.h>
#include <ArduinoJson.h>

#pragma once

class ConfigManager {
public:
    /**
     * @brief Get the singleton instance of ConfigManager
     * @return Reference to the ConfigManager instance
     */
    static ConfigManager& getInstance();

    /**
     * @brief Load configuration from JSON file
     * @return true if configuration loaded successfully, false otherwise
     */
    bool loadConfiguration();

    /**
     * @brief Get direct access to the configuration document
     * @return Reference to the JsonDocument containing configuration
     */
    JsonDocument& getConfig();

    /**
     * @brief Get pins configuration for a specific component
     * @param component Name of the component (e.g., "display", "sd")
     * @return JsonObject containing pin configuration for the component
     */
    static JsonObject getPins(const char* component);

    /**
     * @brief Get a specific pin value for a component
     * @param component Name of the component (e.g., "display", "sd")
     * @param pin Name of the specific pin (e.g., "r1", "g1", "b1")
     * @return Pin value as integer
     */
    static int8_t getPin(const char* component, const char* pin);

private:
    /**
     * @brief Private constructor to prevent direct instantiation
     */
    ConfigManager();

    // Prevent copying
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    static ConfigManager instance;  //< Singleton instance
    JsonDocument config;            //< Configuration data container
};

#endif // CONFIG_MANAGER_H
