# Setup Instructions

This guide provides comprehensive instructions for setting up and running the ESP32 LED Matrix Controller system.

## Table of Contents

- [Setup Instructions](#setup-instructions)
  - [Table of Contents](#table-of-contents)
  - [Hardware Requirements](#hardware-requirements)
  - [Software Prerequisites](#software-prerequisites)
  - [Quick Start](#quick-start)
  - [Usage](#usage)
  - [Quick Start Guide](#quick-start-guide)
    - [1. Clone the Repository](#1-clone-the-repository)
    - [2. PlatformIO Setup (Recommended)](#2-platformio-setup-recommended)
      - [Install PlatformIO](#install-platformio)
      - [Configure WiFi Settings](#configure-wifi-settings)
      - [Build and Upload](#build-and-upload)
  - [Initial Configuration](#initial-configuration)
    - [Configuration File Setup](#configuration-file-setup)
      - [File Locations](#file-locations)
      - [Security Notes](#security-notes)
    - [File System Setup](#file-system-setup)
      - [SD Card Setup (for GIF storage)](#sd-card-setup-for-gif-storage)
      - [ESP32 LittleFS Setup (for configuration)](#esp32-littlefs-setup-for-configuration)
  - [Accessing the System](#accessing-the-system)
    - [Web Interface](#web-interface)
    - [REST API](#rest-api)
  - [Troubleshooting](#troubleshooting)
    - [Common Issues](#common-issues)
    - [Getting Help](#getting-help)

## Hardware Requirements

Before starting, ensure you have the following hardware components:

- **ESP32 development board** (ESP32-WROOM-32 recommended)
- **HUB75 LED matrix panel** (64×64 pixels recommended)
- **MicroSD card** (FAT32 formatted, 1GB minimum)
- **SD card module** for ESP32
- **5V power supply** (2A minimum recommended)
- **HUB75 cable** or jumper wires for custom wiring
- **USB cable** for programming the ESP32

## Software Prerequisites

- **PlatformIO** (Recommended) or **Arduino IDE**
- **Git** for cloning the repository
- **Web browser** for accessing the control interface

## Quick Start

1. **Clone the repository** and navigate to the project directory
2. **Choose your development environment**:
   - **PlatformIO** (Recommended): Run `pio run -t upload`
   - **Arduino IDE**: Follow the setup instructions in the documentation
3. **Configure WiFi** in `firmware/data/config.json`
4. **Access web interface** using the ESP32's IP address

## Usage

1. **Connect** ESP32 to power and wait for WiFi connection
2. **Open web interface** using the ESP32's IP address
3. **Upload GIFs** and organize them into categories
4. **Start playback** by selecting a category
5. **Control brightness** and power settings via web UI

## Quick Start Guide

### 1. Clone the Repository

```bash
git clone [repository-url]
cd esp32-hub75-led-matrix
```

### 2. PlatformIO Setup (Recommended)

#### Install PlatformIO

1. Install PlatformIO extension in VS Code
2. Open the project folder in VS Code

#### Configure WiFi Settings

1. Copy the example configuration:

   ```bash
   cp firmware/data/config.example.json firmware/data/config.json
   ```

2. Edit `firmware/data/config.json` with your settings:
   - Set WiFi SSID and password
   - Configure hostname (optional)
   - Set static IP configuration (optional)
   - Adjust brightness and other display settings

#### Build and Upload

```bash
# Build the project
pio run

# Upload to ESP32
pio run -t upload
```

## Initial Configuration

### Configuration File Setup

1. Copy `config.example.json` to `config.json`
2. Edit `config.json` with your settings:
   - Set WiFi SSID and password
   - Configure hostname if needed
   - Set staticIP to true if using static IP
   - Adjust brightness and other state settings
  
For advanced configuration options, see the [Configuration Guide](configuration.md).

#### File Locations

- Main config: `firmware/data/config.json`
- Example: `firmware/data/config.example.json`

#### Security Notes

- Keep config.json in .gitignore
- Never commit sensitive credentials
- Use config.example.json for sharing template

### File System Setup

#### SD Card Setup (for GIF storage)

1. Format your microSD card as FAT32
2. Create the following directory structure on the SD card:

   ```bash
   /SD_CARD/
   ├── gifs/
   │   ├── category1/
   │   ├── category2/
   │   └── ...
   └── current.gif
   ```

   The SD card is used exclusively for storing GIF files and categories.

#### ESP32 LittleFS Setup (for configuration)

1. Edit `firmware/data/config.json` with your WiFi and system settings
2. Upload the configuration to ESP32's LittleFS:

   ```bash
   # Upload filesystem image to ESP32
   pio run -t uploadfs
   ```

   The config.json file is stored in the ESP32's LittleFS filesystem and contains:
   - WiFi network settings (SSID, password)
   - System configuration (hostname, brightness, etc.)
   - Display settings and preferences

## Accessing the System

### Web Interface

1. Connect your computer/device to the same network as the ESP32
2. Open a web browser
3. Navigate to the ESP32's IP address (displayed in serial output or web interface)
4. The web interface provides:
   - GIF upload and management
   - Category selection and playback control
   - Brightness adjustment
   - Power On/Off control
   - OTA update functionality

### REST API

The system provides a comprehensive REST API for programmatic control. See [API Reference](api-reference.md) for detailed endpoint documentation.

## Troubleshooting

### Common Issues

**ESP32 not connecting to WiFi:**

- Verify SSID and password in `config.json`
- Check WiFi signal strength
- Ensure ESP32 is within range of the access point

**Display not working:**

- Verify HUB75 pin connections (see [Pin Mapping](pin-mapping.md))
- Check power supply voltage and current capacity
- Ensure LED matrix panel is compatible

**SD card not detected:**

- Verify SD card is properly formatted (FAT32)
- Check SD card module wiring
- Ensure SD card is not write-protected

**Web interface not loading:**

- Confirm ESP32 is connected to WiFi
- Check firewall settings
- Try accessing via different browser

### Getting Help

- Check serial output for error messages
- Review the logging system documentation
- Consult the [Implementation Details](implementation-details.md) for system architecture
