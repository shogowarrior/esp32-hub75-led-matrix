#ifndef ANIMATED_GIF_PANEL_H
#define ANIMATED_GIF_PANEL_H

/**
 * @file AnimatedGIFPanel.h
 * @brief Combined GIF rendering and management for ESP32 HUB75 LED Matrix
 *
 * Handles GIF rendering on LED matrix, manages GIF categories and playback state,
 * and copies GIF files from SD to local storage.
 */

#include <Arduino.h>
#include <string>
#include <vector>

#include <AnimatedGIF.h>
#include <ArduinoJson.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#include "FSUtils.h"
#include "DisplayService.h"


/**
 * @struct GifCategory
 * @brief Represents a GIF category containing multiple GIF files
 */
struct GifCategory {
     String name;                    //< Category name
     std::vector<String> files;      //< GIF files in category
     size_t currentIndex = 0;        //< Current playback index

     GifCategory() = default;
     GifCategory(const String &n) : name(n) {}
 };

/**
 * @class AnimatedGIFPanel
 * @brief GIF rendering panel combined with category and playback management
 *
 * Provides:
 * - GIF rendering on LED matrix
 * - GIF file callbacks for AnimatedGIF library
 * - SD card scanning for categories
 * - Category and file playback management
 * - Copying files from SD to LittleFS
 */
class AnimatedGIFPanel {
public:
    // =============================================================================
    // Singleton Management
    // =============================================================================
    static AnimatedGIFPanel& getInstance();

    // =============================================================================
    // Initialization and Control
    // =============================================================================
    bool initialize();
    void stop();

    // =============================================================================
    // Playback and File Management
    // =============================================================================
    bool ShowGIF(const String &path);

    // =============================================================================
    // Category Management
    // =============================================================================
    bool setCategory(const String &categoryName);
    String getCurrentCategory() const;
    std::vector<String> getCategoryList() const;
    String getCategoryInfo(const String &categoryName) const;
    size_t getCategoryCount() const { return categories.size(); }

    // =============================================================================
    // File Management
    // =============================================================================
    String getCurrentGif() const { return currentGifFile; }

    /**
     * @brief Check if a filename has a GIF extension (.gif or .GIF)
     * @param filename Filename to check
     * @return true if file is a GIF
     */
    static bool isGifFile(const String& filename);

    // =============================================================================
    // Uploaded GIF Management
    // =============================================================================
    bool saveUploadedGif(const String &categoryName, const String &filename, const uint8_t *data, size_t size);
    bool createCategoryIfNotExists(const String &categoryName);
    bool deleteUploadedGif(const String &categoryName, const String &filename);
    bool refreshCategoryFiles(const String &categoryName);

    // =============================================================================
    // GIF Processing and Resizing
    // =============================================================================
    bool processAndSaveGif(const String &categoryName, const String &filename, const uint8_t *data, size_t size);
    bool resizeGif(const uint8_t *inputData, size_t inputSize, uint8_t *outputData, size_t &outputSize, int targetWidth, int targetHeight);
    bool validateGif(const uint8_t *data, size_t size, int &width, int &height);

    // =============================================================================
    // Playback Control
    // =============================================================================
    bool isCategoryPlayback() const;
    void setCategoryPlayback(bool playback);
    void playCategory();
    void playCurrentGif();
    void playbackTask();

    // =============================================================================
    // Navigation
    // =============================================================================
    String getNextGif();
    String getPreviousGif();

    // =============================================================================
    // Status and Information
    // =============================================================================
    String getStatusJson() const;

    // =============================================================================
    // Power Management
    // =============================================================================
    bool isPowerOn() const;
    void setPowerState(bool state);

    // =============================================================================
    // State Management
    // =============================================================================
    bool loadStateFromFile();
    void updateState();

    // =============================================================================
    // GIF Callbacks (static because AnimatedGIF expects static functions)
    // =============================================================================
    static void GIFDraw(GIFDRAW *pDraw);
    static void GIFCloseFile(void *pHandle);
    static int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen);
    static int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition);
    static void *GIFOpenFile(const char *szFilename, int32_t *pSize);

private:
    // =============================================================================
    // Private Members
    // =============================================================================
    // Private constructor to prevent direct instantiation
    AnimatedGIFPanel();

    MatrixPanel_I2S_DMA *display; //< Pointer to LED matrix display object

    // Static instance
    static AnimatedGIFPanel instance;

    // GIF handling
    AnimatedGIF gif; //< AnimatedGIF instance for GIF handling

    // Category management
    std::vector<GifCategory> categories; //< List of GIF categories
    size_t currentCategoryIndex = 0;     //< Currently selected category index


    // Playback state
    String currentGifFile;                //< Name of the current GIF file (for tracking)
    bool categoryPlayback = false;         //< Is category playback active?
    bool powerOn = true;                   //< Power state of the display

    // File handling
    File currentFile; //< Current file handle for GIF operations
    fs::FS &fs = FSUtils::getFS(FSType::SD); //< Filesystem reference for SD card

    // =============================================================================
    // Private Methods
    // =============================================================================
    bool scanCategories();
};

#endif // ANIMATED_GIF_PANEL_H
