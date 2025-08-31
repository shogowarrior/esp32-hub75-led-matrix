/*
 * Service.cpp - Main service initialization and management for ESP32 LED Matrix
 *
 * This file contains the core service management functionality including:
 * - FreeRTOS task management for OTA updates and display
 * - Service initialization and startup sequence
 * - Error handling and system monitoring
 */

// ============================================================================
// System Includes
// ============================================================================
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// External Libraries
#include <ArduinoOTA.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <WiFi.h>

// ============================================================================
// Local Includes
// ============================================================================
#include "AnimatedGIFPanel.h"
#include "constants.h"
#include "FSUtils.h"
#include "Service.h"
#include "web/WebService.h"
#include "ConfigManager.h"
#include "DisplayService.h"
#include "Logger.h"
#include "Network.h"

// ============================================================================
// Global Variables and Task Handles
// ============================================================================

/**
 * @brief FreeRTOS task handles for background services
 *
 * These handles allow monitoring and control of the background tasks
 * that run the OTA update service and LED matrix display updates.
 */
TaskHandle_t arduinoOTATaskHandle = nullptr;
TaskHandle_t displayTaskHandle = nullptr;

// Service class constructor and destructor
Service::Service() : webServer(nullptr) {
    // Initialize member variables
}

Service::~Service() {
    // Clean up FreeRTOS tasks
    if (arduinoOTATaskHandle != nullptr) {
        vTaskDelete(arduinoOTATaskHandle);
        arduinoOTATaskHandle = nullptr;
        LOG_INFO("OTA task cleaned up");
    }

    if (displayTaskHandle != nullptr) {
        vTaskDelete(displayTaskHandle);
        displayTaskHandle = nullptr;
        LOG_INFO("Display task cleaned up");
    }

    // Clean up web server if it was allocated
    if (webServer != nullptr) {
        delete webServer;
        webServer = nullptr;
        LOG_INFO("Web server cleaned up");
    }

    // Stop the global web server
    ::stopWebServer();

    LOG_INFO("Service destructor completed - all resources cleaned up");
}

// ============================================================================
// FreeRTOS Background Task Functions
// ============================================================================

// OTA Task Function
void Service::arduinoOTATask(void* parameter) {
  for (;;) {
    ArduinoOTA.handle();
    // Run every 5 seconds to check for OTA updates
    vTaskDelay(OTA_CHECK_INTERVAL_MS / portTICK_PERIOD_MS);
  }
}

/**
 * @brief LED matrix display background task
 *
 * This FreeRTOS task runs continuously to update the LED matrix display
 * with GIF animations. It reads state from state.json and uses those values
 * to control playback. It runs on CPU core 1 with higher priority for
 * smooth animation performance.
 *
 * @param parameter Unused task parameter
 */
void Service::displayTask(void* pvParameters) {
  Service* self = static_cast<Service*>(pvParameters);
  AnimatedGIFPanel& gifPanel = AnimatedGIFPanel::getInstance();

  if (!&gifPanel) {
    LOG_ERROR("GifDisplayTask: Invalid parameters");
    vTaskDelete(nullptr);
    return;
  }

  // Load state from file using the AnimatedGIFPanel method
  gifPanel.loadStateFromFile();

  while (true) {
    gifPanel.playbackTask();
    // The playback is now handled internally by AnimatedGIFPanel
    vTaskDelay(DISPLAY_UPDATE_INTERVAL_MS / portTICK_PERIOD_MS);
  }
}

/**
 * @brief Report task creation status
 *
 * Logs the result of FreeRTOS task creation attempts with detailed
 * error information for debugging and system monitoring.
 *
 * @param taskResult The result code from xTaskCreatePinnedToCore
 */
void Service::taskStatus(BaseType_t taskResult) {
  if (taskResult != pdPASS) {
    LOG_ERROR("Failed to create LED Matrix background task (Error: %d)", taskResult);
  }
}

/**
 * @brief Create a FreeRTOS task with error handling
 *
 * Helper function to create FreeRTOS tasks with standardized error handling
 * and logging for consistent task creation across the application.
 *
 * @param taskFunction The task function to execute
 * @param taskName Descriptive name for the task
 * @param stackSize Stack size in bytes
 * @param taskParameter Parameter to pass to the task
 * @param priority Task priority (0-24, higher = more priority)
 * @param taskHandle Pointer to store the task handle
 * @param coreId CPU core to pin the task to (0 or 1)
 * @return true if task created successfully, false otherwise
 */
bool Service::createBackgroundTask(TaskFunction_t taskFunction, const char* taskName,
                                 uint32_t stackSize, void* taskParameter, UBaseType_t priority,
                                 TaskHandle_t* taskHandle, BaseType_t coreId) {
  BaseType_t result = xTaskCreatePinnedToCore(
      taskFunction, taskName, stackSize, taskParameter, priority, taskHandle, coreId);

  if (result != pdPASS) {
    LOG_CRITICAL("%s background taskcreation failed!", taskName);
    return false;
  }

  LOG_INFO("✓ %s task running on core %d", taskName, coreId);
  return true;
}

/**
 * @brief Setup FreeRTOS background tasks
 *
 * Creates and starts the background tasks for OTA handling and LED matrix
 * display updates. Tasks are pinned to specific CPU cores for optimal
 * performance distribution.
 */
bool Service::setupBackgroundTasks() {
  LOG_INFO("Initializing background tasks...");

  // Create OTA task on core 0 (lower priority)
  if (!createBackgroundTask(arduinoOTATask, "OTA_Task", OTA_TASK_STACK_SIZE, NULL, OTA_TASK_PRIORITY,
                           &arduinoOTATaskHandle, 0)) {
    return false;
  }

  // Create LED matrix task on core 1 (higher priority)
  if (!createBackgroundTask(displayTask, "DISPLAY_Task", DISPLAY_TASK_STACK_SIZE, NULL, DISPLAY_TASK_PRIORITY,
                           &displayTaskHandle, 1)) {
    return false;
  }
  return true;
}

// Initialize OTA service
bool Service::initializeOTA() {
  ArduinoOTA.onStart([]() {
   LOG_INFO("OTA: Update started");
 });
 ArduinoOTA.onEnd([]() {
   LOG_INFO("OTA: Update completed successfully");
 });
 ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
   LOG_INFO("OTA: Progress: %u%%", (progress / (total / 100)));
 });
 ArduinoOTA.onError([](ota_error_t error) {
   const char* errorMsg = "";
   if (error == OTA_AUTH_ERROR)
     errorMsg = "Auth Failed";
   else if (error == OTA_BEGIN_ERROR)
     errorMsg = "Begin Failed";
   else if (error == OTA_CONNECT_ERROR)
     errorMsg = "Connect Failed";
   else if (error == OTA_RECEIVE_ERROR)
     errorMsg = "Receive Failed";
   else if (error == OTA_END_ERROR)
     errorMsg = "End Failed";
   else
     errorMsg = "Unknown Error";
   LOG_ERROR("OTA ERROR[%u]: %s", error, errorMsg);
 });

  ArduinoOTA.begin();
  return true;
}

// Web server service
bool Service::startWebServer() {
  // Call the global startWebServer function from WebService
  return ::startWebServer();
}

// ============================================================================
// Main Service Startup Function
// ============================================================================

bool Service::initialize() {

  LOG_MESSAGE("SERVICE INITIALIZATION", "Starting all services...");

  // Step 1: Initialize LittleFS first (to access config files)
  if (!FSUtils::begin(FSType::LITTLEFS)) {
    LOG_CRITICAL("Failed to initialize LittleFS!");
    return false;
  }
  LOG_INFO("✓ LittleFS file system initialized successfully");

  // Step 2: Load configuration using ConfigManager
  ConfigManager& config = ConfigManager::getInstance();
  if (!config.loadConfiguration()) {
    LOG_CRITICAL("Failed to load configuration!");
    return false;
  }
  LOG_INFO("✓ Configuration loaded successfully");

  // Step 3: Initialize SD card with pins from loaded configuration
  if (!FSUtils::begin(FSType::SD)) {
    LOG_CRITICAL("Failed to initialize SD card!");
    return false;
  }
  LOG_INFO("✓ SD card file system initialized successfully");

  // Step 4: Initialize LED Matrix hardware
  DisplayService& displayService = DisplayService::getInstance();
  if (!displayService.initialize()) {
    LOG_CRITICAL("Failed to initialize LED Matrix!");
    return false;
  }
  LOG_INFO("✓ LED Matrix hardware initialized successfully");

  // Step 5: Initialize Animated GIF Panel
  AnimatedGIFPanel& gifPanel = AnimatedGIFPanel::getInstance();
  if (!gifPanel.initialize()) {
    LOG_CRITICAL("Failed to initialize GIF Panel!");
    return false;
  }
  LOG_INFO("✓ Animated GIF Panel initialized successfully");

  // Step 6: Start WiFi connectivity using the Network class
  Network& network = Network::getInstance();
  if (!network.initialize()) {
    LOG_CRITICAL("Failed to initialize network service!");
    return false;
  }
  LOG_INFO("✓ WiFi connectivity initialized successfully");

  // Step 7: Start OTA service
  if (!initializeOTA()) {
    LOG_CRITICAL("Failed to initialize OTA service!");
    return false;
  }
  LOG_INFO("✓ OTA service initialized successfully");

  // Step 8: Start Web server
  if (!this->startWebServer()) {
    LOG_CRITICAL("Failed to start Web server!");
    return false;
  }
  LOG_INFO("✓ Web server started successfully");

  // Step 9: Setup background tasks
  if (!setupBackgroundTasks()) {
    return false;
  }
  LOG_INFO("✓ Background tasks initialized successfully");

  // Startup complete
  LOG_MESSAGE("SYSTEM READY", "All services initialized and running successfully!");
  // LOG_INFO("Web interface available at: http://%s", network.getLocalIP().toString().c_str());
  // LOG_INFO("OTA updates available at: %s.local", network.getHostname());

  return true;
}
