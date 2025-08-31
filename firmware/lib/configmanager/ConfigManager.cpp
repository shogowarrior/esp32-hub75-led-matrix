#include "ConfigManager.h"
#include "constants.h"
#include "FSUtils.h"
#include "Logger.h"

// Static instance
ConfigManager ConfigManager::instance;

// Constructor
ConfigManager::ConfigManager() = default;

// Get singleton instance
ConfigManager& ConfigManager::getInstance() {
    return instance;
}

// Load configuration from config.json
bool ConfigManager::loadConfiguration() {
    LOG_INFO("Loading configuration from config.json...");

    FS fs = FSUtils::getFS(FSType::LITTLEFS);
    File configFile = fs.open(CONFIG_FILE, "r");
    if (!configFile) {
        LOG_ERROR("Could not open %s file", CONFIG_FILE);
        return false;
    }

    DeserializationError error = deserializeJson(config, configFile);

    if (error) {
        LOG_ERROR("Failed to parse %s - %s", CONFIG_FILE, error.c_str());
        return false;
    }

    

    // Optional: validate essential fields dynamically
    if (!config[NETWORK][WIFI_SSID].is<const char*>()) {
        LOG_ERROR("Network SSID not configured");
        return false;
    }

    if (!config[NETWORK][WIFI_PASSWORD].is<const char*>()) {
        LOG_ERROR("Network password not configured");
        return false;
    }
    return true;
}

// Get direct access to the configuration document
JsonDocument& ConfigManager::getConfig() {
    return config;
}

// Get pins configuration for a specific component
JsonObject ConfigManager::getPins(const char* component) {
    return getInstance().config[PINS][component].as<JsonObject>();
}

// Get a specific pin value for a component
int8_t ConfigManager::getPin(const char* component, const char* pin) {
    JsonObject pins = getPins(component);
    return pins[pin].as<int8_t>();
}

