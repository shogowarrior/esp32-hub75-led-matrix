#ifndef NETWORK_H
#define NETWORK_H

/**
 * @file Network.h
 * @brief Network connectivity and configuration management for ESP32 LED Matrix
 *
 * This class provides comprehensive network management including WiFi connectivity,
 * static/DHCP IP configuration, hostname setting, and network information reporting.
 */

#include <ArduinoJson.h>
#include <IPAddress.h>
#include <WiFi.h>

// =============================================================================
// Network Management Class
// =============================================================================

class Network {
public:
    // =============================================================================
    // Singleton Management
    // =============================================================================
    /**
     * @brief Get the singleton instance of Network
     * @return Network& Reference to the singleton instance
     */
    static Network& getInstance();

    // =============================================================================
    // Network Configuration
    // =============================================================================
    /**
     * @brief Set the device hostname from configuration
     * @param config JsonDocument containing network configuration
     */
    void setHostname(JsonDocument config);

    /**
     * @brief Configure IP settings (static or DHCP) from configuration
     * @param config JsonDocument containing network configuration
     */
    void setIP(JsonDocument config);

    /**
     * @brief Print current network connection details to serial
     */
    void printNetworkDetails();

    // =============================================================================
    // Network Connection Management
    // =============================================================================
    /**
     * @brief Initialize and establish WiFi connection
     * @return true if WiFi connection was established successfully, false otherwise
     */
    bool initialize();

    // =============================================================================
    // Network Status Getters
    // =============================================================================
    /**
     * @brief Check if WiFi is connected
     * @return true if connected, false otherwise
     */
    bool isConnected() const { return WiFi.status() == WL_CONNECTED; }

    /**
     * @brief Get the local IP address
     * @return IPAddress object containing the local IP
     */
    IPAddress getLocalIP() const { return WiFi.localIP(); }

    /**
     * @brief Get the current hostname
     * @return String containing the hostname
     */
    String getHostname() const { return WiFi.getHostname(); }

private:
    // =============================================================================
    // Private Constructor and Destructor
    // =============================================================================
    /**
     * @brief Private constructor to prevent direct instantiation
     */
    Network();

    // Prevent copying
    Network(const Network&) = delete;
    Network& operator=(const Network&) = delete;

    // =============================================================================
    // Private Members
    // =============================================================================
    static Network instance;  //< Singleton instance
};

#endif  // NETWORK_H