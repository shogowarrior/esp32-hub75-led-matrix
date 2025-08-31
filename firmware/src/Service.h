#ifndef SERVICE_H
#define SERVICE_H

#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include "DisplayService.h"
#include "Network.h"

/**
 * @class Service
 * @brief Main service orchestrator for ESP32 HUB75 LED Matrix project
 *
 * This class handles all service management functions including
 * WiFi connectivity, OTA updates, web server endpoints, and background task coordination.
 */
class Service {
public:
     Service();
     ~Service();
     bool initialize();
     DisplayService& getDisplayService() {
         return DisplayService::getInstance();
     }

private:
     AsyncWebServer* webServer;

     // Web server service
     bool startWebServer();

     // OTA update service
     bool initializeOTA();

    // Background task setup
    bool setupBackgroundTasks();
    bool createBackgroundTask(TaskFunction_t taskFunction, const char* taskName,
                            uint32_t stackSize, void* taskParameter, UBaseType_t priority,
                            TaskHandle_t* taskHandle, BaseType_t coreId);
    void taskStatus(BaseType_t displayResult);
    static void arduinoOTATask(void *parameter);
    static void displayTask(void *parameter);
};

#endif // SERVICE_H