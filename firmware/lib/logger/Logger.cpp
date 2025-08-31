#include "Logger.h"
#include <cstdarg>

// Static member initialization
Logger::LogLevel Logger::currentLogLevel = Logger::INFO;
bool Logger::initialized = false;

// Set the current log level
void Logger::setLogLevel(LogLevel level) {
    currentLogLevel = level;
    initialized = true;
}

// Get the current log level
Logger::LogLevel Logger::getLogLevel() {
    return currentLogLevel;
}


// Print a bordered message with custom title
void Logger::printBorderedMessage(const String& title, const String& message) {
    printBorder();
    Serial.println(title);
    if (message.length() > 0) {
        Serial.println(message);
    }
    printBorder();
}


// Utility method to print border
void Logger::printBorder() {
    Serial.println("==============================================");
}

// Utility method to print line with newline
void Logger::println(const String& message) {
    Serial.println(message);
}

// Utility method to print without newline
void Logger::print(const String& message) {
    Serial.print(message);
}

// Core logging method
void Logger::log(LogLevel level, const String& prefix, const String& message) {
    Serial.print("[");
    Serial.print(getTimestamp());
    Serial.print("] ");
    Serial.print(prefix);
    Serial.print(": ");
    Serial.println(message);
}

// Get formatted timestamp (simplified for ESP32)
String Logger::getTimestamp() {
    // For ESP32, we'll use millis() for timestamp
    // In a real implementation, you might want to use NTP or RTC
    unsigned long ms = millis();
    unsigned long seconds = ms / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;

    char buffer[20];
    sprintf(buffer, "%02lu:%02lu:%02lu.%03lu",
            hours % 24, minutes % 60, seconds % 60, ms % 1000);
    return String(buffer);
}

// Helper method to format messages with variadic arguments
String Logger::formatMessage(const char* format, va_list args) {
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    return String(buffer);
}

// Single method per level - handles printf-style formatting
void Logger::debug(const char* format, ...) {
    if (currentLogLevel <= DEBUG) {
        va_list args;
        va_start(args, format);
        String message = formatMessage(format, args);
        va_end(args);
        log(DEBUG, "DEBUG", message);
    }
}

void Logger::info(const char* format, ...) {
    if (currentLogLevel <= INFO) {
        va_list args;
        va_start(args, format);
        String message = formatMessage(format, args);
        va_end(args);
        log(INFO, "INFO", message);
    }
}

void Logger::warning(const char* format, ...) {
    if (currentLogLevel <= WARNING) {
        va_list args;
        va_start(args, format);
        String message = formatMessage(format, args);
        va_end(args);
        log(WARNING, "WARNING", message);
    }
}

void Logger::error(const char* format, ...) {
    if (currentLogLevel <= ERROR) {
        va_list args;
        va_start(args, format);
        String message = formatMessage(format, args);
        va_end(args);
        log(ERROR, "ERROR", message);
    }
}

void Logger::critical(const char* format, ...) {
    if (currentLogLevel <= CRITICAL) {
        va_list args;
        va_start(args, format);
        String message = formatMessage(format, args);
        va_end(args);
        log(CRITICAL, "CRITICAL", message);
    }
}