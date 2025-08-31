#ifndef FSUTILS_H
#define FSUTILS_H

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <SD.h>
#include <SPI.h>
#include <vector>

#include "constants.h"

/**
 * @file FSUtils.h
 * @brief Unified filesystem utility class for ESP32 LED Matrix Controller
 *
 * This class provides a unified interface for managing multiple filesystem types
 * including LittleFS and SD card, with operations like file I/O, directory management,
 * and filesystem initialization.
 */

// =============================================================================
// Filesystem Types
// =============================================================================

class ConfigManager;   // forward declare

/**
 * @enum FSType
 * @brief Supported filesystem types
 */
enum class FSType {
    LITTLEFS,  //< LittleFS filesystem
    SD         //< SD Card filesystem
};

/**
 * @brief Unified filesystem utility class that supports multiple filesystem types
 */
class FSUtils {
public:
    /**
     * @brief Supported filesystem types
     */

    /**
     * @brief Initialize the specified filesystem
     * @param fsType Type of filesystem to initialize
     * @return true if initialization successful, false otherwise
     */
    static bool begin(FSType fsType);

    /**
     * @brief Write binary data to a file
     * @param fsType Type of filesystem
     * @param path File path
     * @param data Data to write
     * @param len Length of data
     * @param mode File mode (default: FILE_WRITE)
     * @return true if successful, false otherwise
     */
    static bool writeFile(FSType fsType, const char* path, const uint8_t* data, size_t len, const char* mode = FILE_WRITE);

    /**
     * @brief Write text to a file
     * @param fsType Type of filesystem
     * @param path File path
     * @param message Text to write
     * @param mode File mode (default: FILE_WRITE)
     * @return true if successful, false otherwise
     */
    static bool writeFile(FSType fsType, const char* path, const char* message, const char* mode = FILE_WRITE);

    /**
     * @brief Read file content as String
     * @param fsType Type of filesystem
     * @param path File path
     * @return String containing file content
     */
    static String readFile(FSType fsType, const char* path);

    /**
     * @brief Read file content into buffer
     * @param fsType Type of filesystem
     * @param path File path
     * @param buffer Buffer to store data
     * @param maxLen Maximum length to read
     * @return Number of bytes read
     */
    static size_t readFile(FSType fsType, const char* path, uint8_t* buffer, size_t maxLen);

    /**
     * @brief List directory contents
     * @param fsType Type of filesystem
     * @param dirname Directory path
     */
    static void listDir(FSType fsType, const char* dirname);

    /**
     * @brief List directory contents with options
     * @param fsType Type of filesystem
     * @param dirname Directory path
     * @param recursive Whether to recursively list subdirectories
     * @param printDir Whether to print directory information
     * @return Vector of directory entries
     */
    static std::vector<String> _listDir(FSType fsType, const char* dirname, bool recursive = false, bool printDir = true);

    /**
     * @brief Print directory entries
     * @param entries Vector of directory entries to print
     */
    static void printDirEntries(const std::vector<String> &entries);

    /**
     * @brief Delete a file
     * @param fsType Type of filesystem
     * @param path File path
     * @return true if successful, false otherwise
     */
    static bool deleteFile(FSType fsType, const char* path);

    /**
     * @brief Create a directory
     * @param fsType Type of filesystem
     * @param path Directory path
     * @return true if successful, false otherwise
     */
    static bool createDir(FSType fsType, const char* path);

    /**
     * @brief Remove a directory
     * @param fsType Type of filesystem
     * @param path Directory path
     * @return true if successful, false otherwise
     */
    static bool removeDir(FSType fsType, const char* path);

    /**
     * @brief Check if file exists
     * @param fsType Type of filesystem
     * @param path File path
     * @return true if exists, false otherwise
     */
    static bool exists(FSType fsType, const char* path);

    /**
     * @brief Get file size
     * @param fsType Type of filesystem
     * @param path File path
     * @return File size in bytes
     */
    static size_t fileSize(FSType fsType, const char* path);

    /**
     * @brief Get filesystem type name
     * @param fsType Type of filesystem
     * @return String representation of filesystem type
     */
    static const char* getFSTypeName(FSType fsType);

    /**
     * @brief Get filesystem instance
     * @param fsType Type of filesystem
     * @return Reference to filesystem instance
     */
    static fs::FS& getFS(FSType fsType);

    /**
     * @brief Check if filesystem is initialized
     * @param fsType Type of filesystem
     * @return true if initialized, false otherwise
     */
    static bool isInitialized(FSType fsType);

    /**
     * @brief Copy file between filesystems
     * @param srcFS Source filesystem type
     * @param srcPath Source file path
     * @param dstFS Destination filesystem type
     * @param dstPath Destination file path
     * @return true if successful, false otherwise
     */
    static bool copyFile(FSType srcFS, const char* srcPath, FSType dstFS, const char* dstPath);

    /**
     * @brief Build a path from multiple components using "/" delimiter
     * @param first First path component
     * @param ... Additional path components (variadic, must end with nullptr)
     * @return Built path as String
     */
    static String buildPath(const char* first, ...);

    /**
     * @brief Build a path from multiple String components using "/" delimiter
     * @param components Vector of path components
     * @return Built path as String
     */
    static String buildPath(const std::vector<String>& components);

private:
    // Track initialization status of filesystems
    static bool _littlefsInitialized;
    static bool _sdInitialized;
};

#endif // FSUTILS_H
