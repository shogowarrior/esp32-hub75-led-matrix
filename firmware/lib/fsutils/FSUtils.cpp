#include "FSUtils.h"
#include "Logger.h"
#include <cstdarg>
#include <ArduinoJson.h>
#include "ConfigManager.h"

// Initialize static members
bool FSUtils::_littlefsInitialized = false;
bool FSUtils::_sdInitialized = false;

// ============================================================================
// Filesystem Management
// ============================================================================

/**
 * @brief Initialize filesystem
 * @param fsType Type of filesystem to initialize
 * @return true if initialization successful, false otherwise
 */
bool FSUtils::begin(FSType fsType) {
  switch (fsType) {
    case FSType::LITTLEFS:
      if (_littlefsInitialized) {
        LOG_INFO("LittleFS already initialized");
        return true;
      }
      if (!LittleFS.begin(true)) {
        LOG_ERROR("Failed to initialize LittleFS");
        return false;
      }
      _littlefsInitialized = true;
      return true;

    case FSType::SD: {
      if (_sdInitialized) {
        LOG_INFO("SD card already initialized");
        return true;
      }

      // Get SD pin configuration from ConfigManager
      JsonObject pins = ConfigManager::getPins(PINS_SD);
      if (pins.isNull()) {
        LOG_ERROR("SD pin configuration not found");
        return false;
      }

      int8_t csPin = pins[PIN_CS].as<int8_t>();
      int8_t cmosiPin = pins[PIN_MOSI].as<int8_t>();
      int8_t misoPin = pins[PIN_MISO].as<int8_t>();
      int8_t sckPin = pins[PIN_SCK].as<int8_t>();

      SPI.begin(sckPin, misoPin, cmosiPin, csPin);
      if (!SD.begin(csPin, SPI)) {
        LOG_ERROR("Failed to initialize SD card");
        return false;
      }
      _sdInitialized = true;
      LOG_INFO("SD card initialized successfully with pins SCK:%d, MOSI:%d, MISO:%d, CS:%d",
               sckPin, cmosiPin, misoPin, csPin);
      return true;
    }

    default:
      LOG_ERROR("Unknown filesystem type");
      return false;
  }
}

/**
 * @brief Get filesystem instance based on type
 * @param fsType Type of filesystem
 * @return Reference to the filesystem instance
 */
fs::FS& FSUtils::getFS(FSType fsType) {
  switch (fsType) {
    case FSType::SD:
      return SD;
    case FSType::LITTLEFS:
    default:
      return LittleFS;
    }
  }
  
  /**
   * @brief Check if filesystem is initialized
   * @param fsType Type of filesystem
   * @return true if initialized, false otherwise
   */
  bool FSUtils::isInitialized(FSType fsType) {
    switch (fsType) {
      case FSType::LITTLEFS:
        return _littlefsInitialized;
      case FSType::SD:
        return _sdInitialized;
      default:
        return false;
    }
  }
  
  const char* FSUtils::getFSTypeName(FSType fsType) {
  switch (fsType) {
    case FSType::SD:
      return "SD";
    case FSType::LITTLEFS:
    default:
      return "LittleFS";
  }
}

// ============================================================================
// File Operations
// ============================================================================

/**
 * @brief Write binary data to file
 * @param fs Filesystem type or reference
 * @param path File path
 * @param data Data to write
 * @param len Length of data
 * @param mode File write mode (default: FILE_WRITE)
 * @return true if write successful, false otherwise
 */
bool FSUtils::writeFile(FSType fsType, const char* path, const uint8_t* data,
                      size_t len, const char* mode) {

  fs::FS& fs = getFS(fsType);
  if (!data || !len || !path || !mode) {
    LOG_ERROR("Invalid parameters for writeFile");
    return false;
  }

  File file = fs.open(path, mode);
  if (!file) {
    LOG_ERROR("Failed to open file for writing: %s", path);
    return false;
  }

  size_t written = file.write(data, len);
  file.close();

  if (written != len) {
    LOG_ERROR("Write failed - wrote %d/%d bytes to %s", written, len, path);
    return false;
  }

  LOG_INFO("Successfully wrote %d bytes to %s", written, path);
  return true;
}

/**
 * @brief Write string data to file using binary write wrapper
 * @param fs Filesystem type or reference
 * @param path File path
 * @param message Null-terminated string
 * @param mode File write mode (default: FILE_WRITE)
 * @return true if write successful, false otherwise
 */
bool FSUtils::writeFile(FSType fsType, const char* path, const char* message,
                       const char* mode) {
  if (!message) {
    LOG_ERROR("Null string in writeFile");
    return false;
  }
  return FSUtils::writeFile(fsType, path, (const uint8_t*)message, strlen(message), mode);
}

/**
 * @brief Read file content as String
 * @param fs Filesystem type or reference
 * @param path File path
 * @return File content as String, empty String if failed
 */
String FSUtils::readFile(FSType fsType, const char* path) {
  fs::FS& fs = FSUtils::getFS(fsType);
  if (!path) {
    LOG_ERROR("Invalid path parameter");
    return String();
  }

  File file = fs.open(path);
  if (!file || file.isDirectory()) {
    LOG_ERROR("Failed to open file for reading: %s", path);
    return String();
  }

  String content;
  while (file.available()) {
    content += (char)file.read();
  }
  file.close();
  return content;
}

/**
 * @brief Read file content into buffer
 * @param fsType Type of filesystem
 * @param path File path
 * @param buffer Buffer to store data
 * @param maxLen Maximum length to read
 * @return Number of bytes read
 */
size_t FSUtils::readFile(FSType fsType, const char* path, uint8_t* buffer, size_t maxLen) {
  fs::FS& fs = FSUtils::getFS(fsType);
  if (!path || !buffer || !maxLen) {
    LOG_ERROR("Invalid parameters for readFile");
    return 0;
  }

  File file = fs.open(path);
  if (!file || file.isDirectory()) {
    LOG_ERROR("Failed to open file for reading: %s", path);
    return 0;
  }

  size_t bytesRead = file.read(buffer, maxLen);
  file.close();
  return bytesRead;
}

/**
 * @brief Check if file exists
 * @param fs Filesystem type or reference
 * @param path File path
 * @return true if file exists, false otherwise
 */
bool FSUtils::exists(FSType fsType, const char* path) {
  fs::FS& fs = FSUtils::getFS(fsType);
  if (!path) {
    return false;
  }
  return fs.exists(path);
}

/**
 * @brief Get file size
 * @param fs Filesystem type or reference
 * @param path File path
 * @return File size in bytes, 0 if file doesn't exist or is directory
 */
size_t FSUtils::fileSize(FSType fsType, const char* path) {
  fs::FS& fs = FSUtils::getFS(fsType);
  if (!path) {
    return 0;
  }

  File file = fs.open(path);
  if (!file || file.isDirectory()) {
    return 0;
  }

  size_t size = file.size();
  file.close();
  return size;
}

/**
 * @brief Delete file
 * @param fs Filesystem type or reference
 * @param path File path
 * @return true if deletion successful, false otherwise
 */
bool FSUtils::deleteFile(FSType fsType, const char* path) {
  fs::FS& fs = FSUtils::getFS(fsType);
  if (!path) {
    LOG_ERROR("Invalid path parameter");
    return false;
  }

  if (!fs.remove(path)) {
    LOG_ERROR("Failed to delete file: %s", path);
    return false;
  }

  LOG_INFO("Successfully deleted file: %s", path);
  return true;
}

// ============================================================================
// Directory Operations
// ============================================================================

// Public wrapper, calls internal with printDir = true
void FSUtils::listDir(FSType fsType, const char* dirname) {
  FSUtils::_listDir(fsType, dirname, false, false);
}

/**
 * @brief List directory contents by FS reference
 * @param fsType Type of filesystem
 * @param dirname Directory path
 * @param recursive Whether to recursively list subdirectories
 * @param printDir Whether to print directory information
 * @return Vector of directory entries
 */
std::vector<String> FSUtils::_listDir(FSType fsType, const char* dirname, bool recursive, bool printDir) {
     if (!dirname) {
         LOG_ERROR("Invalid directory path");
         return {};
     }

     fs::FS& fs = FSUtils::getFS(fsType);
     File root = fs.open(dirname);
     if (!root || !root.isDirectory()) {
         LOG_ERROR("Failed to open directory or not a directory");
         return {};
     }

    if (printDir) {
        LOG_INFO("DIR: %s", dirname);
    }

    std::vector<String> entries;

    File file = root.openNextFile();
    while (file) {
        String fullPath = String(dirname) + "/" + file.name();

        if (file.isDirectory()) {
            entries.push_back(fullPath);

            if (recursive) {
                std::vector<String> subEntries = FSUtils::_listDir(fsType, fullPath.c_str(), recursive, printDir);
                entries.insert(entries.end(), subEntries.begin(), subEntries.end());
            }
        }
        else {
            entries.push_back(fullPath);
        }

        file = root.openNextFile();
    }
    root.close();

    if (printDir) {
        FSUtils::printDirEntries(entries);
    }

    return entries;
}

void FSUtils::printDirEntries(const std::vector<String> &entries) {
     for (const auto &entry : entries) {
         LOG_INFO("Directory entry: %s", entry.c_str());
     }
 }

// ============================================================================
// Directory Management
// ============================================================================

/**
 * @brief Create directory
 * @param fs Filesystem type or reference
 * @param path Directory path
 * @return true if creation successful, false otherwise
 */
bool FSUtils::createDir(FSType fsType, const char* path) {
   fs::FS& fs = FSUtils::getFS(fsType);
   if (!path) {
     LOG_ERROR("Invalid path parameter");
     return false;
   }

   if (!fs.mkdir(path)) {
     LOG_ERROR("Failed to create directory: %s", path);
     return false;
   }

   LOG_INFO("Successfully created directory: %s", path);
   return true;
 }

/**
 * @brief Remove directory
 * @param fs Filesystem type or reference
 * @param path Directory path
 * @return true if removal successful, false otherwise
 */
bool FSUtils::removeDir(FSType fsType, const char* path) {
   fs::FS& fs = FSUtils::getFS(fsType);
   if (!path) {
     LOG_ERROR("Invalid path parameter");
     return false;
   }

   if (!fs.rmdir(path)) {
     LOG_ERROR("Failed to remove directory: %s", path);
     return false;
   }

   LOG_INFO("Successfully removed directory: %s", path);
   return true;
 }

/**
 * @brief Copy file from one filesystem to another
 * @param srcFS Source filesystem
 * @param srcPath Source file path
 * @param dstFS Destination filesystem
 * @param dstPath Destination file path
 * @return true if copy successful, false otherwise
 */
bool FSUtils::copyFile(FSType srcFS, const char* srcPath, FSType dstFS,
                      const char* dstPath) {
   fs::FS& source = FSUtils::getFS(srcFS);
   fs::FS& destination = FSUtils::getFS(dstFS);

   if (!srcPath || !dstPath) {
     LOG_ERROR("Invalid path parameters");
     return false;
   }

   File srcFile = source.open(srcPath);
   if (!srcFile || srcFile.isDirectory()) {
     LOG_ERROR("Failed to open source file: %s", srcPath);
     return false;
   }

   File dstFile = destination.open(dstPath, FILE_WRITE);
   if (!dstFile) {
     LOG_ERROR("Failed to create destination file: %s", dstPath);
     srcFile.close();
     return false;
   }

   size_t bytesCopied = 0;
   uint8_t buffer[1024];
   while (srcFile.available()) {
     size_t bytesRead = srcFile.read(buffer, sizeof(buffer));
     size_t bytesWritten = dstFile.write(buffer, bytesRead);
     if (bytesWritten != bytesRead) {
       LOG_ERROR("Write failed during copy");
       srcFile.close();
       dstFile.close();
       return false;
     }
     bytesCopied += bytesWritten;
   }

   srcFile.close();
   dstFile.close();

   LOG_INFO("Successfully copied %d bytes from %s to %s", bytesCopied, srcPath, dstPath);
   return true;
 }
  // ============================================================================
  // Path Utilities
  // ============================================================================

  /**
   * @brief Build a path from multiple components using "/" delimiter
   * @param first First path component
   * @param args Additional path components (variadic)
   * @return Built path as String
   */
  String FSUtils::buildPath(const char* first, ...) {
    if (!first) {
      return String();
    }

    String path = String(first);

    va_list args;
    va_start(args, first);

    const char* component;
    while ((component = va_arg(args, const char*)) != nullptr) {
      if (component && strlen(component) > 0) {
        // Remove trailing slash from path if present
        if (path.endsWith("/")) {
          path = path.substring(0, path.length() - 1);
        }

        // Remove leading slash from component if present
        String compStr = String(component);
        if (compStr.startsWith("/")) {
          compStr = compStr.substring(1);
        }

        // Only add separator if both parts are non-empty
        if (!compStr.isEmpty()) {
          if (!path.isEmpty()) {
            path += "/";
          }
          path += compStr;
        }
      }
    }

    va_end(args);
    return path;
  }

  /**
   * @brief Build a path from multiple String components using "/" delimiter
   * @param components Vector of path components
   * @return Built path as String
   */
  String FSUtils::buildPath(const std::vector<String>& components) {
    if (components.empty()) {
      return String();
    }

    String path;

    for (size_t i = 0; i < components.size(); ++i) {
      String component = components[i];

      if (component.isEmpty()) {
        continue;
      }

      // Remove trailing slash from path if present
      if (path.endsWith("/")) {
        path = path.substring(0, path.length() - 1);
      }

      // Remove leading slash from component if present
      if (component.startsWith("/")) {
        component = component.substring(1);
      }

      // Only add separator if both parts are non-empty
      if (!component.isEmpty()) {
        if (!path.isEmpty()) {
          path += "/";
        }
        path += component;
      }
    }

    return path;
  }