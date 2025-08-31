#include "AnimatedGIFPanel.h"
#include "constants.h"
#include "FSUtils.h"
#include "WebService.h"
#include "Logger.h"

AsyncWebServer server(DEFAULT_WEB_PORT);

fs::FS *imageFS;  // For GIF storage (SD card)
fs::FS *uiFS;     // For web UI files (LittleFS)

bool startWebServer() {
    // Initialize filesystems
    imageFS = &FSUtils::getFS(FSType::SD);
    uiFS = &FSUtils::getFS(FSType::LITTLEFS);

    if (!imageFS || !uiFS) {
        LOG_ERROR("Failed to initialize filesystems - SD card and/or LittleFS not accessible");
        LOG_ERROR("Web server cannot start without proper filesystem initialization");
        return false;
    }

    // Setup web server components
    bool setupSuccess = true;
    try {
        setupStaticFiles();
        setupSpaRouting();
        setupApiEndpoints();
    } catch (const std::exception& e) {
        LOG_ERROR("Exception during setup: %s", e.what());
        setupSuccess = false;
    } catch (...) {
        LOG_ERROR("Unknown exception during web server setup");
        setupSuccess = false;
    }

    if (!setupSuccess) {
        LOG_ERROR("Web server setup failed - cannot start server");
        return false;
    }

    // Start the server
    server.begin();
    LOG_INFO("Web server started successfully on port %d", WEB_SERVER_PORT);
    return true;
}

void stopWebServer() {
    server.end();
}

void setupStaticFiles() {
    server.serveStatic("/", *uiFS, "/")
        .setDefaultFile(DEFAULT_FILE)
        .setCacheControl("max-age=3600");
}

bool isApiRoute(const String& url) {
    return url.startsWith("/api");
}

void handleSpaFallback(AsyncWebServerRequest *request) {
    if (!uiFS->exists(DEFAULT_FILE_PATH)) {
        request->send(500, "text/plain", "Internal Server Error");
        return;
    }
    request->send(*uiFS, DEFAULT_FILE_PATH, "text/html");
}

void setupSpaRouting() {
    server.onNotFound([](AsyncWebServerRequest *request) {
        if (isApiRoute(request->url())) {
            request->send(404);
        } else {
            handleSpaFallback(request);
        }
    });
}

void onGifUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    static File uploadFile;
    static String uploadPath;
    static String destination;
    static String category;

    if (index == 0) {
        // First chunk - get parameters and create temp file
        destination = request->arg("destination");
        category = request->arg("category");

        // Create temp file path
        uploadPath = "/temp/" + filename;

        // Create temp directory if it doesn't exist
        if (!imageFS->exists("/temp")) {
            imageFS->mkdir("/temp");
        }

        // Open file for writing (use imageFS for temp file)
        uploadFile = imageFS->open(uploadPath, "w");
        if (!uploadFile) {
            request->send(500, "application/json", "{\"success\":false,\"message\":\"Failed to create upload file\"}");
            return;
        }
    }

    if (uploadFile && len > 0) {
        // Write data to file
        uploadFile.write(data, len);
    }

    if (final) {
        if (uploadFile) {
            uploadFile.close();

            // Process the uploaded file
            bool success = processUploadedGif(uploadPath, destination, category);

            if (success) {
                // Clean up temp file
                imageFS->remove(uploadPath);
                request->send(200, "application/json", "{\"success\":true,\"message\":\"GIF uploaded and processed successfully\"}");
            } else {
                // Clean up temp file
                imageFS->remove(uploadPath);
                request->send(500, "application/json", "{\"success\":false,\"message\":\"Failed to process GIF\"}");
            }
        } else {
            request->send(500, "application/json", "{\"success\":false,\"message\":\"Upload failed\"}");
        }
    }
}

String getFilenameFromPath(const String& path) {
    int lastSlash = path.lastIndexOf('/');
    if (lastSlash == -1) {
        return path;
    }
    return path.substring(lastSlash + 1);
}

/**
 * Helper method to get the AnimatedGIFPanel instance and handle error response
 * @param request The web request to send error response to (nullptr if not needed)
 * @return Pointer to the AnimatedGIFPanel instance or nullptr if not initialized
 */
AnimatedGIFPanel* getGifPanelWithError(AsyncWebServerRequest *request) {
    AnimatedGIFPanel& gifPanel = AnimatedGIFPanel::getInstance();
    if (!&gifPanel && request) {
        request->send(500, "application/json", "{\"error\":\"GIF panel not initialized\"}");
    }
    return &gifPanel;
}

bool processUploadedGif(const String& tempPath, const String& destination, const String& category) {
    // Open the uploaded file
    File gifFile = imageFS->open(tempPath, "r");
    if (!gifFile) {
        LOG_ERROR("Failed to open uploaded GIF file");
        return false;
    }

    // Check file size (max 2MB)
    const size_t MAX_FILE_SIZE = 2 * 1024 * 1024; // 2MB
    if (gifFile.size() > MAX_FILE_SIZE) {
        LOG_ERROR("File size exceeds 2MB limit");
        gifFile.close();
        return false;
    }

    // Read the file data into memory
    size_t fileSize = gifFile.size();
    uint8_t* fileData = new uint8_t[fileSize];
    if (!fileData) {
        LOG_ERROR("Failed to allocate memory for GIF data");
        gifFile.close();
        return false;
    }

    gifFile.read(fileData, fileSize);
    gifFile.close();

    // Get the AnimatedGIFPanel instance
    AnimatedGIFPanel& gifPanel = AnimatedGIFPanel::getInstance();
    if (!&gifPanel) {
        LOG_ERROR("GIF panel not initialized");
        delete[] fileData;
        return false;
    }

    // Process and save the GIF using the new methods
    // Extract filename from path
    int lastSlash = tempPath.lastIndexOf('/');
    String filename = lastSlash == -1 ? tempPath : tempPath.substring(lastSlash + 1);

    if (destination == "current") {
        // Save as current GIF
        bool success = gifPanel.processAndSaveGif("current", filename, fileData, fileSize);
        delete[] fileData;
        return success;
    } else if (destination == "category") {
        // Save to specific category
        bool success = gifPanel.processAndSaveGif(category, filename, fileData, fileSize);
        delete[] fileData;
        return success;
    } else {
        LOG_ERROR("Invalid destination specified");
        delete[] fileData;
        return false;
    }
}

void setupApiEndpoints() {

    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        JsonDocument doc;
        doc["status"] = "ok";
        doc["network"] = Network::getInstance().isConnected();
        doc["ip"] = Network::getInstance().getLocalIP().toString();

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    server.on("/api/brightness", HTTP_GET, [](AsyncWebServerRequest *request) {
        JsonDocument doc;
        doc["brightness"] = DisplayService::getInstance().getBrightness();
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // Power state endpoint
    server.on("/api/power", HTTP_GET, [](AsyncWebServerRequest *request) {
        // Use AnimatedGIFPanel for power state
        AnimatedGIFPanel* gifPanel = getGifPanelWithError(request);
        if (!gifPanel) {
            return;
        }

        JsonDocument doc;
        doc["power"] = DisplayService::getInstance().isPowerOn();
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    server.on("/api/power", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (!request->hasParam("power", true)) {
            request->send(400, "application/json", "{\"error\":\"Missing power parameter\"}");
            return;
        }

        bool powerState = request->getParam("power", true)->value() == "true";

        // Use AnimatedGIFPanel for power state
        AnimatedGIFPanel* gifPanel = getGifPanelWithError(request);
        if (!gifPanel) {
            return;
        }

        // Set the power state
        gifPanel->setPowerState(powerState);

        JsonDocument doc;
        doc["success"] = true;
        doc["power"] = powerState;
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // Category endpoints
    server.on("/api/categories", HTTP_GET, [](AsyncWebServerRequest *request) {
        JsonDocument doc;
        JsonArray categories = doc["categories"].to<JsonArray>();

        // Get all available categories
        AnimatedGIFPanel* gifPanel = getGifPanelWithError(request);
        if (!gifPanel) {
            return;
        }

        std::vector<String> categoryList = gifPanel->getCategoryList();

        for (const String& category : categoryList) {
            JsonObject categoryObj = categories.add<JsonObject>();
            categoryObj["name"] = category;
        }

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // GIF Upload endpoint
    server.on("/api/upload", HTTP_POST,
        [](AsyncWebServerRequest *request) {
            if (!request->hasParam("destination", true)) {
                request->send(400, "application/json", "{\"success\":false,\"message\":\"Missing destination parameter\"}");
                return;
            }

            // Handle file upload
            request->send(200);
        },
        onGifUpload
    );

    // Legacy endpoints
    server.on("/setCategory", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!request->hasParam("category")) {
            request->send(400);
            return;
        }

        // Redirect to API endpoint
        String category = request->getParam("category")->value();
        AsyncWebServerResponse *response = request->beginResponse(301, "text/plain", "Moved to API endpoint");
        response->addHeader("Location", "/api/category/set?category=" + category);
        request->send(response);
    });

    server.on("/setBrightness", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!request->hasParam("brightness")) {
            request->send(400);
            return;
        }

        int brightness = request->getParam("brightness")->value().toInt();
        if (brightness >= 0 && brightness <= 255) {
            DisplayService::getInstance().setBrightness(brightness);
        }
        request->send(200, "text/plain", "Brightness set");
    });
}

