#ifndef WEBSERVICE_H
#define WEBSERVICE_H

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include "Network.h"
#include "constants.h"

#pragma once


// Function declarations
bool startWebServer();
void stopWebServer();
void setupStaticFiles();
void setupSpaRouting();
void setupApiEndpoints();
void setupLegacyEndpoints();
void onUpload(AsyncWebServerRequest *request, String filename, size_t index,
              uint8_t *data, size_t len, bool final);
void getImage(AsyncWebServerRequest *request);

String getFilenameFromPath(const String& path);
AnimatedGIFPanel* getGifPanelWithError(AsyncWebServerRequest *request);
bool downloadAndProcessGif(const String &url, const String &destination,
                           const String &category);
bool processUploadedGif(const String &tempPath, const String &destination,
                        const String &category);
#endif  // WEBSERVICE_H