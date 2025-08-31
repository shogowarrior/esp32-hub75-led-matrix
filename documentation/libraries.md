# Libraries

## Table of Contents

- [Libraries](#libraries)
  - [Table of Contents](#table-of-contents)
  - [FSUtils](#fsutils)
  - [Logger](#logger)
  - [Network](#network)
  - [ConfigManager](#configmanager)
  - [DisplayService](#displayservice)
  - [AnimatedGIFs](#animatedgifs)
  - [Plasma](#plasma)

## FSUtils

**Location:** [fsutils](../firmware/lib/fsutils)

Unified filesystem utility for ESP32 with support for both LittleFS and SD card storage. Provides file operations, directory management, and filesystem information with robust error handling.

## Logger

**Location:** [logger](../firmware/lib/logger)

Centralized logging utility with multiple log levels (DEBUG, INFO, WARNING, ERROR, CRITICAL) and formatted output. Includes convenience macros for consistent message formatting.

For detailed logging configuration and usage, see [`logger.md`](logger.md).

## Network

**Location:** [network](../firmware/lib/network)

Network management library handling WiFi connectivity, IP configuration, and network services. Includes connection monitoring, static/DHCP configuration, and OTA update integration.

## ConfigManager

**Location:** [configmanager](../firmware/lib/configmanager)

Configuration management library for loading, saving, and validating JSON-based system configuration. Provides runtime access to configuration values with validation and default handling.

## DisplayService

**Location:** [displayservice](../firmware/lib/displayservice)

Display service for HUB75 LED matrix hardware initialization and management. Handles display configuration, brightness, refresh rates, and basic matrix operations.

## AnimatedGIFs

**Location:** [animatedgifs](../firmware/lib/animatedgifs)

GIF playback functionality for LED matrix with category management and file handling capabilities.

## Plasma

**Location:** [plasma](../firmware/lib/plasma)

Plasma visual effects generator with multiple color palettes for LED matrix displays
