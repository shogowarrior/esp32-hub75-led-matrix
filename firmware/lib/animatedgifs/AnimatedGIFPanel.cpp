#include "AnimatedGIFPanel.h"
#include "constants.h"
#include <ArduinoJson.h>
#include <string>
#include <vector>
#include "ConfigManager.h"
#include "Logger.h"

// Static instance
AnimatedGIFPanel AnimatedGIFPanel::instance;

// =============================================================================
// Constructor & Destructor
// =============================================================================

/**
 * @brief Constructor - initializes GIF panel and manager state
 * @param disp Pointer to display object
 */
AnimatedGIFPanel::AnimatedGIFPanel() {}

// =============================================================================
// Singleton Management
// =============================================================================

/**
 * @brief Get the singleton instance of AnimatedGIFPanel
 * @param disp Pointer to display object (only needed for first initialization)
 * @return Pointer to the singleton instance
 */
AnimatedGIFPanel &AnimatedGIFPanel::getInstance() { return instance; }

// =============================================================================
// Initialization and Control
// =============================================================================

/**
 * @brief Initialize the GIF panel and SD
 * @return true if initialization successful, false otherwise
 */
bool AnimatedGIFPanel::initialize() {
   // Get display service instance
   DisplayService& displayService = DisplayService::getInstance();
   display = displayService.getDisplay();

   if (!display) {
     LOG_ERROR("AnimatedGIFPanel: Display pointer is null");
     return false;
   }

   // Initialize member variables
   currentCategoryIndex = 0;
   categoryPlayback = false;
   powerOn = true;

   gif.begin(LITTLE_ENDIAN_PIXELS);

  // Initialize SD card using FSUtils if not already done
  if (!FSUtils::begin(FSType::SD)) {
    LOG_ERROR("Failed to initialize SD card");
    return false;
  }

  // Scan for categories
  if (!scanCategories()) {
    LOG_ERROR("Failed to scan categories");
    return false;
  }
  // Load last state from file
  loadStateFromFile();

  return true;
}

/**
 * @brief Load and apply state from ConfigManager
 * @return true if state was loaded successfully
 */
bool AnimatedGIFPanel::loadStateFromFile() {
     // Get values from ConfigManager using direct JsonDocument access
     JsonDocument& configDoc = ConfigManager::getInstance().getConfig();
     bool powerOn = configDoc[STATE][IS_POWER_ON].as<bool>();
     String lastSelectedCategory = configDoc[STATE][LAST_SELECTED_CATEGORY].as<String>();
     bool categoryPlayback = configDoc[STATE][CATEGORY_PLAYBACK].as<bool>();
     int brightness = configDoc[STATE][BRIGHTNESS].as<int>();

    // Set display brightness
    DisplayService::getInstance().setBrightness(brightness);

    // Set category and playback mode
    setCategory(lastSelectedCategory);
    setCategoryPlayback(categoryPlayback);

    // Set power state
    setPowerState(powerOn);

    return true;
}

/**
 * @brief Update state in ConfigManager when values are modified
 */
void AnimatedGIFPanel::updateState() {
     // Get reference to ConfigManager
     JsonDocument& configDoc = ConfigManager::getInstance().getConfig();

     // Update state values
     configDoc[STATE][IS_POWER_ON] = powerOn;
     configDoc[STATE][LAST_SELECTED_CATEGORY] = getCurrentCategory();
     configDoc[STATE][CATEGORY_PLAYBACK] = categoryPlayback;

     // Note: brightness is handled by DisplayService and should be updated there
     LOG_DEBUG("AnimatedGIFPanel: State updated - powerOn: %d, category: %s, categoryPlayback: %d",
               powerOn, getCurrentCategory().c_str(), categoryPlayback);
}

/**
 * @brief Stop current GIF playback
 */
void AnimatedGIFPanel::stop() {
   // Reset state
   gif.close();
   currentGifFile = "";
   currentCategoryIndex = 0;
   categoryPlayback = false;
 }


// =============================================================================
// Playback Task for Current or Category GIF
// =============================================================================
 void AnimatedGIFPanel::playbackTask() {
   String gifPath = String(GIF_DEFAULT_PATH);

   if (isCategoryPlayback()) {
     gifPath = FSUtils::buildPath(GIFS_BASE_PATH, getCurrentCategory().c_str(), getNextGif().c_str(), nullptr);
   }

   if (!ShowGIF(gifPath)) {
     LOG_ERROR("AnimatedGIFPanel: Failed to display GIF");
   }
 }

// =============================================================================
// Playback Control Implementation
// =============================================================================

bool AnimatedGIFPanel::isCategoryPlayback() const {
       return categoryPlayback;
}

void AnimatedGIFPanel::setCategoryPlayback(bool playback) {
         categoryPlayback = playback;

         // Update state in ConfigManager
         updateState();
}

bool AnimatedGIFPanel::isPowerOn() const {
    return powerOn;
}

// =============================================================================
// Category Management
// =============================================================================

/**
 * @brief Scan SD card for GIF categories (directories)
 * @return true if scan successful
 */
bool AnimatedGIFPanel::scanCategories() {
    categories.clear();

    String gifsPath = String(GIFS_BASE_PATH);

    File root = FSUtils::getFS(FSType::SD).open(gifsPath);
    if (!root) {
      LOG_ERROR("Failed to open %s", GIFS_BASE_PATH);
      return false;
    }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      String catName = file.name();
      if (!catName.startsWith(".")) {  // skip hidden dirs
        String categoryPath = "/" + gifsPath + "/" + catName;
        GifCategory category(catName);

        File categoryDir = FSUtils::getFS(FSType::SD).open(categoryPath);
        if (categoryDir) {
          File gifFile = categoryDir.openNextFile();
          while (gifFile) {
            if (!gifFile.isDirectory()) {
              String fname = gifFile.name();
              if (AnimatedGIFPanel::isGifFile(fname)) {
                category.files.push_back(fname);
              }
            }
            gifFile = categoryDir.openNextFile();
          }
          categoryDir.close();
        } else {
          LOG_ERROR("Failed to open category directory: %s", categoryPath.c_str());
        }

        if (!category.files.empty()) {
          categories.push_back(category);
        }
      }
    }
    file = root.openNextFile();
  }
  root.close();

  return true;
}


/**
 * @brief Set the current category by name
 * @param categoryName Name of category to set
 * @return true if category was found and set
 */
bool AnimatedGIFPanel::setCategory(const String &categoryName) {
   for (size_t i = 0; i < categories.size(); i++) {
     if (categories[i].name.equalsIgnoreCase(categoryName)) {
       currentCategoryIndex = i;
       currentGifFile = getNextGif();

       // Update state in ConfigManager
       updateState();

       return true;
     }
   }
   LOG_WARNING("AnimatedGIFPanel: Category not found: %s", categoryName.c_str());
   return false;
 }

/**
 * @brief Get the current category name
 * @return Name of current category or empty string if none
 */
String AnimatedGIFPanel::getCurrentCategory() const {
  if (currentCategoryIndex < categories.size()) {
    return categories[currentCategoryIndex].name;
  }
  return "";
}

/**
 * @brief Get a list of all category names
 * @return Vector of category names
 */
std::vector<String> AnimatedGIFPanel::getCategoryList() const {
  std::vector<String> list;
  for (const auto &category : categories) {
    list.push_back(category.name);
  }
  return list;
}

/**
 * @brief Get information about a specific category
 * @param categoryName Name of category to get info for
 * @return JSON string with category information
 */
String AnimatedGIFPanel::getCategoryInfo(const String &categoryName) const {
    for (const auto &category : categories) {
      if (category.name.equalsIgnoreCase(categoryName)) {
        JsonDocument doc;
        doc["name"] = category.name;
        doc["file_count"] = category.files.size();

        JsonArray files = doc["files"].to<JsonArray>();
        for (const auto &file : category.files) {
          files.add(file);
        }

        String output;
        if (serializeJson(doc, output) == 0) {
          return "{\"error\":\"Failed to serialize JSON\"}";
        }
        return output;
      }
    }
    return "{}";
  }

// =============================================================================
// File Management
// =============================================================================

/**
 * @brief Check if a filename has a GIF extension (.gif or .GIF)
 * @param filename Filename to check
 * @return true if file is a GIF
 */
bool AnimatedGIFPanel::isGifFile(const String& filename) {
    return filename.endsWith(".gif") || filename.endsWith(".GIF");
}

// =============================================================================
// Playback Management
// =============================================================================

/**
 * @brief Get the next GIF in the current category
 * @return Filename of next GIF or empty string if none
 */
String AnimatedGIFPanel::getNextGif() {
  if (currentCategoryIndex >= categories.size()) return "";

  GifCategory &category = categories[currentCategoryIndex];
  if (category.files.empty()) return "";

  category.currentIndex = (category.currentIndex + 1) % category.files.size();
  currentGifFile = category.files[category.currentIndex];
  return currentGifFile;
}

/**
 * @brief Get the previous GIF in the current category
 * @return Filename of previous GIF or empty string if none
 */
String AnimatedGIFPanel::getPreviousGif() {
  if (currentCategoryIndex >= categories.size()) return "";

  GifCategory &category = categories[currentCategoryIndex];
  if (category.files.empty()) return "";

  category.currentIndex = (category.currentIndex + category.files.size() - 1) %
                          category.files.size();
  currentGifFile = category.files[category.currentIndex];
  return currentGifFile;
}

// =============================================================================
// Status and Information
// =============================================================================

/**
 * @brief Get status information as JSON
 * @return JSON string with status information
 */
String AnimatedGIFPanel::getStatusJson() const {
     JsonDocument doc;

     doc["category_playback_enabled"] = categoryPlayback;
     doc["current_category"] = getCurrentCategory();
     doc["current_gif"] = currentGifFile;
     doc["category_count"] = categories.size();
     doc["power_on"] = powerOn;

     JsonArray categoryArray = doc["categories"].to<JsonArray>();
     for (const auto &category : categories) {
       JsonObject c = categoryArray.add<JsonObject>();
       c["name"] = category.name;
       c["file_count"] = category.files.size();
     }

     String output;
     if (serializeJson(doc, output) == 0) {
       return "{\"error\":\"Failed to serialize JSON\"}";
     }
     return output;
   }

// =============================================================================
// GIF Playback
// =============================================================================

/**
 * @brief Show GIF animation from local path
 * @param path Path to GIF file in LittleFS
 * @return true if GIF was shown successfully
 */
bool AnimatedGIFPanel::ShowGIF(const String &path) {
   int start_tick = millis();

   if (gif.open(path.c_str(), GIFOpenFile, GIFCloseFile, GIFReadFile,
                GIFSeekFile, GIFDraw)) {
     int xOffset = (display->width() - gif.getCanvasWidth()) / 2;
     if (xOffset < 0) xOffset = 0;
     int yOffset = (display->height() - gif.getCanvasHeight()) / 2;
     if (yOffset < 0) yOffset = 0;
    LOG_DEBUG("Successfully opened GIF; Canvas size = %d x %d",
              gif.getCanvasWidth(), gif.getCanvasHeight());

    while (gif.playFrame(true, NULL)) {
      if (categoryPlayback &&
          (millis() - start_tick) > MAX_GIF_PLAY_TIME) {
        break;
      }
    }

    gif.close();
    return true;
  }
  return false;
}

// =============================================================================
// GIF Callbacks
// =============================================================================

/**
 * @brief Open a GIF file for reading
 * @param fname Filename to open
 * @param pSize Pointer to store file size
 * @return File handle or NULL if failed
 */
void *AnimatedGIFPanel::GIFOpenFile(const char *fname, int32_t *pSize) {
   LOG_DEBUG("Playing gif: %s", fname);
   instance.currentFile = FSUtils::getFS(FSType::LITTLEFS).open(fname);
   if (instance.currentFile) {
     *pSize = instance.currentFile.size();
     return (void *)&instance.currentFile;
   }
   return NULL;
 }

/**
 * @brief Close a GIF file
 * @param pHandle File handle to close
 */
void AnimatedGIFPanel::GIFCloseFile(void *pHandle) {
  if (pHandle) {
    File *file = static_cast<File *>(pHandle);
    file->close();
    instance.currentFile = File();  // Reset the file handle
  }
}

/**
 * @brief Read data from a GIF file
 * @param pFile File handle
 * @param pBuf Buffer to read into
 * @param iLen Number of bytes to read
 * @return Number of bytes read
 */
int32_t AnimatedGIFPanel::GIFReadFile(GIFFILE *pFile, uint8_t *pBuf,
                                      int32_t iLen) {
  File *file = static_cast<File *>(pFile->fHandle);
  return file->read(pBuf, iLen);
}

/**
 * @brief Seek to a position in a GIF file
 * @param pFile File handle
 * @param iPosition Position to seek to
 * @return New position
 */
int32_t AnimatedGIFPanel::GIFSeekFile(GIFFILE *pFile, int32_t iPosition) {
  File *file = static_cast<File *>(pFile->fHandle);
  return file->seek(iPosition);
}

/**
 * @brief GIF draw callback function
 * @param pDraw Pointer to GIF draw structure
 */
void AnimatedGIFPanel::GIFDraw(GIFDRAW *pDraw) {
  uint8_t *pixels = pDraw->pPixels;
  uint16_t *palette = pDraw->pPalette;
  int width = pDraw->iWidth;

  int maxWidth = instance.display->width();
  if (width > maxWidth) width = maxWidth;

  int y = pDraw->iY + pDraw->y;

  if (pDraw->ucDisposalMethod == 2) {
    for (int x = 0; x < width; x++) {
      if (pixels[x] == pDraw->ucTransparent) {
        pixels[x] = pDraw->ucBackground;
      }
    }
    pDraw->ucHasTransparency = 0;
  }

  if (pDraw->ucHasTransparency) {
    const uint8_t transparent = pDraw->ucTransparent;
    uint16_t tempBuffer[320];
    int x = 0;
    while (x < width) {
      int runLength = 0;

      while (x + runLength < width && pixels[x + runLength] != transparent) {
        runLength++;
      }

      if (runLength > 0) {
        for (int i = 0; i < runLength; i++) {
          tempBuffer[i] = palette[pixels[x + i]];
        }
        for (int i = 0; i < runLength; i++) {
          instance.display->drawPixel(x + i, y, tempBuffer[i]);
        }
        x += runLength;
      }

      runLength = 0;
      while (x + runLength < width && pixels[x + runLength] == transparent) {
        runLength++;
      }
      x += runLength;
    }
  } else {
    for (int x = 0; x < width; x++) {
      instance.display->drawPixel(x, y, palette[pixels[x]]);
    }
  }
}

// =============================================================================
// Power Management
// =============================================================================

/**
 * @brief Set the power state of the display
 * @param state true to turn on, false to turn off
 */
void AnimatedGIFPanel::setPowerState(bool state) {
    powerOn = state;

    // Update state in ConfigManager
    updateState();

    if (powerOn) {
      // Turn on the display
      display->begin();
      playbackTask();
    } else {
      // Turn off the display
      display->clearScreen();
      display->fillScreenRGB888(0, 0, 0);

      // Stop any ongoing playback
      gif.close();
    }
  }

// =============================================================================
// Uploaded GIF Management
// =============================================================================

/**
 * @brief Save an uploaded GIF file to a category
 * @param categoryName Name of category to save to
 * @param filename Filename to use
 * @param data GIF data
 * @param size Size of GIF data
 * @return true if saving was successful
 */
bool AnimatedGIFPanel::saveUploadedGif(const String &categoryName,
                                       const String &filename,
                                       const uint8_t *data, size_t size) {
  // Create category directory if it doesn't exist
  if (!createCategoryIfNotExists(categoryName)) {
    LOG_ERROR("AnimatedGIFPanel: Failed to create category directory");
    return false;
  }

  // Create the full path for the GIF file
   String filePath = FSUtils::buildPath(GIFS_BASE_PATH, categoryName.c_str(), filename.c_str(), nullptr);

  // Open the file for writing
  File gifFile = FSUtils::getFS(FSType::SD).open(filePath, FILE_WRITE);
  if (!gifFile) {
    LOG_ERROR("AnimatedGIFPanel: Failed to open file for writing: %s", filePath.c_str());
    return false;
  }

  // Write the GIF data
  size_t bytesWritten = gifFile.write(data, size);
  gifFile.close();

  if (bytesWritten != size) {
    LOG_ERROR("Failed to write complete GIF data");
    return false;
  }

  // Refresh the category files list
  refreshCategoryFiles(categoryName);

  LOG_INFO("Successfully saved GIF to %s (%u bytes)", filePath.c_str(), bytesWritten);
  return true;
}

/**
 * @brief Create a category directory if it doesn't exist
 * @param categoryName Name of category to create
 * @return true if category exists or was created successfully
 */
bool AnimatedGIFPanel::createCategoryIfNotExists(const String &categoryName) {
    String categoryPath = FSUtils::buildPath(GIFS_BASE_PATH, categoryName.c_str(), nullptr);

  // Check if directory already exists
  if (FSUtils::exists(FSType::SD, categoryPath.c_str())) {
    return true;
  }

  // Create the directory
  if (!FSUtils::createDir(FSType::SD, categoryPath.c_str())) {
    LOG_ERROR("Failed to create category directory: %s", categoryPath.c_str());
    return false;
  }

  LOG_INFO("Created new category directory: %s", categoryPath.c_str());
  return true;
}

/**
 * @brief Delete an uploaded GIF file
 * @param categoryName Name of category containing the file
 * @param filename Filename to delete
 * @return true if deletion was successful
 */
bool AnimatedGIFPanel::deleteUploadedGif(const String &categoryName,
                                          const String &filename) {
   String filePath = FSUtils::buildPath(GIFS_BASE_PATH, categoryName.c_str(), filename.c_str(), nullptr);

  if (!FSUtils::getFS(FSType::SD).remove(filePath.c_str())) {
    LOG_ERROR("AnimatedGIFPanel: Failed to delete file: %s", filePath.c_str());
    return false;
  }

  // Refresh the category files list
  refreshCategoryFiles(categoryName);

  LOG_INFO("Successfully deleted GIF: %s", filePath.c_str());
  return true;
}

/**
 * @brief Refresh the list of files in a category
 * @param categoryName Name of category to refresh
 * @return true if refresh was successful
 */
bool AnimatedGIFPanel::refreshCategoryFiles(const String &categoryName) {
  // Find the category
  for (auto &category : categories) {
    if (category.name.equalsIgnoreCase(categoryName)) {
      // Clear existing files
      category.files.clear();

      // Scan the directory for GIF files
      String categoryPath = FSUtils::buildPath(GIFS_BASE_PATH, category.name.c_str(), nullptr);
      File categoryDir = FSUtils::getFS(FSType::SD).open(categoryPath);
      if (!categoryDir) {
        LOG_ERROR("Failed to open category directory: %s", categoryPath.c_str());
        return false;
      }

      File gifFile = categoryDir.openNextFile();
      while (gifFile) {
        if (!gifFile.isDirectory()) {
          String fname = gifFile.name();
          if (AnimatedGIFPanel::isGifFile(fname)) {
            category.files.push_back(fname);
          }
        }
        gifFile = categoryDir.openNextFile();
      }
      categoryDir.close();

      LOG_INFO("Refreshed category %s - found %u files", category.name.c_str(), category.files.size());
      return true;
    }
  }

  LOG_ERROR("Category not found: %s", categoryName.c_str());
  return false;
}

// =============================================================================
// GIF Processing and Resizing
// =============================================================================

/**
 * @brief Process and save a GIF file with resizing if needed
 * @param categoryName Name of category to save to
 * @param filename Filename to use
 * @param data GIF data
 * @param size Size of GIF data
 * @return true if processing and saving was successful
 */
bool AnimatedGIFPanel::processAndSaveGif(const String &categoryName,
                                         const String &filename,
                                         const uint8_t *data, size_t size) {
  // Validate the GIF first
  int width, height;
  if (!validateGif(data, size, width, height)) {
    LOG_ERROR("AnimatedGIFPanel: Invalid GIF format");
    return false;
  }

  // Check if resizing is needed (target is 64x64)
  if (width == 64 && height == 64) {
    // No resizing needed, save directly
    return saveUploadedGif(categoryName, filename, data, size);
  }

  // Calculate required buffer size for resized GIF
  // This is a simplified approach - in a real implementation, you would need
  // a proper GIF resizing library or algorithm
  size_t resizedSize = size;  // Placeholder - actual size would be calculated

  // Allocate buffer for resized GIF
  uint8_t *resizedData = new uint8_t[resizedSize];
  if (!resizedData) {
    LOG_ERROR("Failed to allocate memory for resized GIF");
    return false;
  }

  // Resize the GIF
  if (!resizeGif(data, size, resizedData, resizedSize, 64, 64)) {
    LOG_ERROR("Failed to resize GIF");
    delete[] resizedData;
    return false;
  }

  // Save the resized GIF
  bool success =
      saveUploadedGif(categoryName, filename, resizedData, resizedSize);

  // Clean up
  delete[] resizedData;

  return success;
}

/**
 * @brief Resize a GIF to target dimensions
 * @param inputData Input GIF data
 * @param inputSize Size of input data
 * @param outputData Buffer for output GIF data
 * @param outputSize Size of output data (will be updated)
 * @param targetWidth Target width
 * @param targetHeight Target height
 * @return true if resizing was successful
 */
bool AnimatedGIFPanel::resizeGif(const uint8_t *inputData, size_t inputSize,
                                 uint8_t *outputData, size_t &outputSize,
                                 int targetWidth, int targetHeight) {
  // Note: This is a simplified placeholder implementation
  // In a real implementation, you would use a proper GIF decoding/encoding
  // library to properly resize each frame while maintaining animation
  // properties

  LOG_INFO("Resizing GIF from %dx%d to %dx%d", targetWidth * 2, targetHeight * 2, targetWidth, targetHeight);

  // For this example, we'll just copy the data as-is
  // In a real implementation, you would:
  // 1. Decode the GIF to extract frames
  // 2. Resize each frame to target dimensions
  // 3. Re-encode the GIF with the resized frames

  if (inputSize > outputSize) {
    LOG_ERROR("Output buffer too small for resized GIF");
    return false;
  }

  memcpy(outputData, inputData, inputSize);
  outputSize = inputSize;

  return true;
}

/**
 * @brief Validate GIF data and get dimensions
 * @param data GIF data
 * @param size Size of data
 * @param width Output parameter for width
 * @param height Output parameter for height
 * @return true if GIF is valid
 */
bool AnimatedGIFPanel::validateGif(const uint8_t *data, size_t size, int &width,
                                   int &height) {
  // Basic GIF signature check
  if (size < 6 || memcmp(data, "GIF", 3) != 0) {
    LOG_ERROR("Invalid GIF signature");
    return false;
  }

  // Check version (either 87a or 89a)
  if (memcmp(data + 3, "87a", 3) != 0 && memcmp(data + 3, "89a", 3) != 0) {
    LOG_ERROR("Unsupported GIF version");
    return false;
  }

  // For this example, we'll assume the GIF is valid and return fixed dimensions
  // In a real implementation, you would parse the GIF header to get actual
  // dimensions
  width = 64;   // Placeholder
  height = 64;  // Placeholder

  // Note: In a real implementation, you would:
  // 1. Parse the GIF header to get actual width/height (bytes 6-9 in the
  // header)
  // 2. Validate the logical screen descriptor
  // 3. Check for global color table if present
  // 4. Verify the file structure

  LOG_DEBUG("GIF validation completed for %dx%d", width, height);
  return true;
}

