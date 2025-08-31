# Logging

The system includes a comprehensive logging framework with the following LOG_ macros:

- `LOG_DEBUG(...)` - Debug level messages (disabled in production)
- `LOG_INFO(...)` - Informational messages
- `LOG_WARNING(...)` - Warning messages
- `LOG_ERROR(...)` - Error messages
- `LOG_CRITICAL(...)` - Critical error messages
- `LOG_MESSAGE(title, message)` - Bordered message with title
- `LOG_FLUSH()` - Flush serial output buffer

## Example Usage

```cpp
LOG_INFO("System starting up...");
LOG_MESSAGE("WIFI STATUS", "Connected to network successfully");
LOG_ERROR("Failed to load configuration file");
LOG_DEBUG("Current brightness: %d", brightness);
```

All logging output includes timestamps and is consistently formatted for easy debugging and monitoring
