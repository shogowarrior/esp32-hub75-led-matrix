# API Reference

## Table of Contents

- [API Reference](#api-reference)
  - [Table of Contents](#table-of-contents)
  - [System Status](#system-status)
  - [Category Management](#category-management)
  - [Display Control](#display-control)
  - [File Management](#file-management)
  - [System Control](#system-control)

## System Status

- `GET /api/status` - Get system status

## Category Management

- `GET /api/categories` - List available categories
- `POST /api/category/set` - Set current category
- `POST /api/category/start` - Start category playback
- `POST /api/category/stop` - Stop category playback

## Display Control

- `POST /api/brightness` - Set display brightness
- `GET /api/brightness` - Get current brightness

## File Management

- `GET /api/files` - List files in current category
- `POST /api/upload` - Upload new GIF file

## System Control

- `POST /api/restart` - Restart device
- `POST /api/ota/check` - Check for OTA updates