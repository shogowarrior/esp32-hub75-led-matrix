#include "Network.h"

#include <Arduino.h>
#include <LittleFS.h>

#include "ConfigManager.h"
#include "Logger.h"
#include "constants.h"

// Static instance
Network Network::instance;

// Constructor
Network::Network() {}

// Get the singleton instance
Network& Network::getInstance() { return instance; }

// Set Hostname
void Network::setHostname(JsonDocument config) {
  // Set device hostname
  LOG_INFO("Setting device hostname...");
  const char* hostname = config[NETWORK][HOSTNAME].as<const char*>();
  LOG_INFO("Setting hostname to: %s", hostname);
  if (!WiFi.setHostname(hostname)) {
    LOG_WARNING("Failed to set hostname");
  } else {
    LOG_INFO("✓ Hostname set successfully");
  }
}

// Set IP address
void Network::setIP(JsonDocument config) {
  // Set static IP configuration if enabled
  if (config[NETWORK][STATIC_IP].as<bool>()) {
    LOG_INFO("Configuring static IP address...");

    IPAddress localIP, gateway, subnet, primaryDNS, secondaryDNS;
    localIP.fromString(config[NETWORK][LOCAL_IP].as<String>().c_str());
    gateway.fromString(config[NETWORK][GATEWAY].as<String>().c_str());
    subnet.fromString(config[NETWORK][SUBNET].as<String>().c_str());
    primaryDNS.fromString(config[NETWORK][PRIMARY_DNS].as<String>().c_str());
    secondaryDNS.fromString(config[NETWORK][SECONDARY_DNS].as<String>().c_str());

    if (!WiFi.config(localIP, gateway, subnet, primaryDNS, secondaryDNS)) {
      LOG_ERROR("Failed to configure static IP - falling back to DHCP");
    } else {
      LOG_INFO("✓ Static IP configuration applied successfully");
    }
  } else {
    LOG_INFO("Using DHCP for IP configuration");
  }
}

// Print detailed network connection information
void Network::printNetworkDetails() {
  LOG_MESSAGE("NETWORK CONNECTION DETAILS",
              "Current network status and configuration");
  LOG_INFO("SSID: %s", WiFi.SSID().c_str());
  LOG_INFO("BSSID: %s", WiFi.BSSIDstr().c_str());
  LOG_INFO("IP Address: %s", WiFi.localIP().toString().c_str());
  LOG_INFO("Hostname: %s", WiFi.getHostname());
  LOG_INFO("MAC Address: %s", WiFi.macAddress().c_str());
  LOG_INFO("Subnet Mask: %s", WiFi.subnetMask().toString().c_str());
  LOG_INFO("Gateway IP: %s", WiFi.gatewayIP().toString().c_str());
  LOG_INFO("DNS Server: %s", WiFi.dnsIP().toString().c_str());
  LOG_INFO("Signal Strength: %d dBm", WiFi.RSSI());
  LOG_INFO("Channel: %d", WiFi.channel());
  LOG_INFO("Network ID: %ld", WiFi.networkID());
}

// Main network service method
bool Network::initialize() {
  LOG_INFO("Initializing Network Service...");
  LOG_MESSAGE("NETWORK INITIALIZATION",
              "Starting network service configuration");

  // Load configuration from config.json
  if (!ConfigManager::getInstance().loadConfiguration()) {
    LOG_CRITICAL("Failed to load configuration!");
    return false;
  }

  JsonDocument config = ConfigManager::getInstance().getConfig();
  // Configure hostname and IP settings
  setHostname(config);
  setIP(config);

  // Connect to network
  const char* ssid = config[NETWORK][WIFI_SSID].as<const char*>();
  const char* password = config[NETWORK][WIFI_PASSWORD].as<const char*>();
  LOG_INFO("Connecting to network: %s", ssid);
  WiFi.begin(ssid, password);

  // Wait for connection with timeout
  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED &&
         attempts < MAX_WIFI_CONNECTION_ATTEMPTS) {
    delay(500);
    LOG_DEBUG("Connection attempt %d/%d", attempts + 1,
              MAX_WIFI_CONNECTION_ATTEMPTS);
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    printNetworkDetails();
    return true;
  } else {
    LOG_CRITICAL("Network initialization failed!");
    LOG_ERROR("Failed to connect to network after %d attempts",
              MAX_WIFI_CONNECTION_ATTEMPTS);
    LOG_ERROR(
        "Please check your network credentials and ensure the network is "
        "available");
    return false;
  }
}