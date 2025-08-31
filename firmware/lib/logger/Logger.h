#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

/**
 * @class Logger
 * @brief Centralized logging utility for the ESP32 LED Matrix Controller
 *
 * This class provides comprehensive logging functionality with different log levels,
 * formatted output, and consistent message formatting throughout the application.
 */
class Logger {
public:
    // Log levels
    enum LogLevel {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        CRITICAL
    };

    /**
     * @brief Set the current log level
     * @param level Minimum log level to display
     */
    static void setLogLevel(LogLevel level);

    /**
     * @brief Get the current log level
     * @return Current log level
     */
    static LogLevel getLogLevel();

    // Single unified logging method per level - handles printf-style formatting
    static void debug(const char* format, ...);
    static void info(const char* format, ...);
    static void warning(const char* format, ...);
    static void error(const char* format, ...);
    static void critical(const char* format, ...);

    // Formatted logging methods
    static void printBorderedMessage(const String& title, const String& message);

    // Utility methods
    static void printBorder();
    static void println(const String& message = "");
    static void print(const String& message);

private:
    // Private constructor to prevent instantiation
    Logger() = delete;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // Static member variables
    static LogLevel currentLogLevel;
    static bool initialized;

    // Helper methods
    static void log(LogLevel level, const String& prefix, const String& message);
    static String getTimestamp();
    static String formatMessage(const char* format, va_list args);
};

// Convenience macros for easier usage
// These work with both String and printf-style arguments
#define LOG_DEBUG(...) Logger::debug(__VA_ARGS__)
#define LOG_INFO(...) Logger::info(__VA_ARGS__)
#define LOG_WARNING(...) Logger::warning(__VA_ARGS__)
#define LOG_ERROR(...) Logger::error(__VA_ARGS__)
#define LOG_CRITICAL(...) Logger::critical(__VA_ARGS__)
#define LOG_MESSAGE(title, message) Logger::printBorderedMessage(title, message)
#define LOG_FLUSH() Serial.flush()

#endif // LOGGER_H