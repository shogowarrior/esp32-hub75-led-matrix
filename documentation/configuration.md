# Configuration Guide

This guide covers advanced configuration options and technical details for the ESP32 LED Matrix Controller system.

## Table of Contents

- [Configuration Guide](#configuration-guide)
  - [Table of Contents](#table-of-contents)
  - [Configuration Files](#configuration-files)
    - [Main Configuration (config.json)](#main-configuration-configjson)
    - [File Locations](#file-locations)
  - [Configuration Parameters](#configuration-parameters)
    - [State Settings](#state-settings)
    - [System Settings](#system-settings)
      - [Debug Mode Behavior](#debug-mode-behavior)
    - [Network Settings](#network-settings)
  - [Security Considerations](#security-considerations)
  - [Advanced Configuration](#advanced-configuration)
    - [Display Pins Settings](#display-pins-settings)
    - [SD Card Pins Settings](#sd-card-pins-settings)
  - [Troubleshooting Configuration Issues](#troubleshooting-configuration-issues)
    - [Configuration Not Loading](#configuration-not-loading)
    - [Network Configuration Problems](#network-configuration-problems)

## Configuration Files

### Main Configuration (config.json)

The primary configuration file contains all system settings:

```json
{
  "state": {
    "brightness": 128,
    "isPowerOn": true,
    "lastSelectedCategory": "characters",
    "categoryPlayback": true
  },
  "system": {
    "debugMode": false,
    "logLevel": "INFO",
    "webServerPort": 80,
    "otaEnabled": true
  },
  "network": {
    "ssid": "YOUR_WIFI_SSID",
    "password": "YOUR_WIFI_PASSWORD",
    "hostname": "led-matrix",
    "staticIp": true,
    "localIp": "192.168.0.130",
    "gateway": "192.168.0.1",
    "subnet": "255.255.255.0",
    "primaryDns": "8.8.8.8",
    "secondaryDns": "8.8.4.4"
  },
  "pins": {
    "display": {
      "R1": 32,
      "G1": 23,
      "B1": 33,
      "R2": 25,
      "G2": 22,
      "B2": 26,
      "A": 27,
      "B": 16,
      "C": 14,
      "D": 4,
      "E": 21,
      "CLK": 12,
      "LAT": 15,
      "OE": 13
    },
    "sd": {
      "CS": 17,
      "MOSI": 5,
      "MISO": 19,
      "SCK": 18
    }
  }
}
```

### File Locations

- Main config: `firmware/data/config.json`
- Example: `firmware/data/config.example.json`

## Configuration Parameters

### State Settings

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `brightness` | integer | 128 | Display brightness level (0-255) |
| `isPowerOn` | boolean | true | Power state of the display |
| `lastSelectedCategory` | string | "characters" | Last selected content category |
| `categoryPlayback` | boolean | true | Enable automatic category playback |

### System Settings

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `debugMode` | boolean | false | Enable debug mode for detailed logging and test mode |
| `logLevel` | string | "INFO" | Logging level (ERROR, WARN, INFO, DEBUG) |
| `webServerPort` | integer | 80 | HTTP web server port |
| `otaEnabled` | boolean | true | Enable Over-The-Air updates |

#### Debug Mode Behavior
When `debugMode` is set to `true`:
- Enables detailed logging output
- Forces the system into **test mode** on startup, which:
  - Only initializes the DisplayService for testing
  - Runs continuous display test patterns
  - Lists SD card contents periodically
  - Bypasses full service initialization (WiFi, OTA, Web Server, etc.)

When `debugMode` is set to `false`:
- Uses normal logging levels
- Runs full service initialization including all network and display services

### Network Settings

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `ssid` | string | "" | Network name |
| `password` | string | "" | Network password |
| `hostname` | string | "led-matrix" | Device hostname |
| `staticIp` | boolean | false | Enable static IP configuration |
| `localIp` | string | "192.168.0.130" | Static IP address |
| `gateway` | string | "192.168.0.1" | Gateway IP address |
| `subnet` | string | "255.255.255.0" | Subnet mask |
| `primaryDns` | string | "8.8.8.8" | Primary DNS server |
| `secondaryDns` | string | "8.8.4.4" | Secondary DNS server |

## Security Considerations

- Keep config.json in .gitignore
- Never commit sensitive credentials
- Use config.example.json for sharing template
- Consider using environment variables for sensitive data in production

## Advanced Configuration

### Display Pins Settings

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `R1` | integer | 32 | Red data pin for row 1 |
| `G1` | integer | 23 | Green data pin for row 1 |
| `B1` | integer | 33 | Blue data pin for row 1 |
| `R2` | integer | 25 | Red data pin for row 2 |
| `G2` | integer | 22 | Green data pin for row 2 |
| `B2` | integer | 26 | Blue data pin for row 2 |
| `A` | integer | 27 | Address line A |
| `B` | integer | 16 | Address line B |
| `C` | integer | 14 | Address line C |
| `D` | integer | 4 | Address line D |
| `E` | integer | 21 | Address line E |
| `CLK` | integer | 12 | Clock pin |
| `LAT` | integer | 15 | Latch pin |
| `OE` | integer | 13 | Output enable pin |

### SD Card Pins Settings

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `CS` | integer | 17 | Chip select pin |
| `MOSI` | integer | 5 | Master out, slave in pin |
| `MISO` | integer | 19 | Master in, slave out pin |
| `SCK` | integer | 18 | Serial clock pin |

## Troubleshooting Configuration Issues

### Configuration Not Loading

- Verify JSON syntax is valid
- Check file permissions on SD card
- Ensure config.json is in the correct location
- Review serial logs for parsing errors

### Network Configuration Problems

- Verify Network credentials
- Check IP address conflicts
- Ensure router allows device connections
- Test with a simple network setup first

For basic setup instructions, see the [Setup Instructions](setup.md).
