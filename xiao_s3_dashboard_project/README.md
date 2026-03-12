# XIAO ESP32-S3 Dashboard Project

This Arduino project is set up for the Seeed XIAO ESP32-S3 and includes:

- LittleFS-hosted web UI
- WebSocket live status updates
- REST APIs for status, stats, config, and sensors
- OTA firmware upload page
- Rotating log file stored in LittleFS
- Counters stored in NVS/Preferences instead of the filesystem

## Folder layout

- `xiao_s3_dashboard_project.ino` - Arduino entry point
- `src/` - support classes
- `data/` - files uploaded to LittleFS

## Libraries to install

Install these in Arduino IDE Library Manager:

- ArduinoJson
- WebSockets by Markus Sattler

The ESP32 core already provides:

- WiFi
- WebServer
- LittleFS
- ESPmDNS
- Preferences
- Update

## Arduino IDE settings

- Board: **XIAO_ESP32S3**
- Flash partition: choose a scheme with LittleFS space
- Serial monitor: **115200**

## First-time use

1. Edit `data/config.json` and put in your Wi-Fi credentials.
2. Optionally change `deviceName`, `adminPassword`, `sampleIntervalMs`, and `analogPin`.
3. Upload the LittleFS image from the `data/` folder.
4. Upload the sketch.
5. Open Serial Monitor and browse to the shown IP address.

## API endpoints

- `GET /api/status`
- `GET /api/config`
- `POST /api/config`
- `GET /api/stats`
- `GET /api/sensors`
- `POST /api/restart`
- `GET /logs.txt`
- `GET /ota`
- `POST /api/ota`

## Notes

- The analog sensor example reads one analog pin and publishes it both in JSON and over WebSocket.
- WebSocket server listens on port 81.
- Logs are rotated automatically when they exceed about 24 KB.
- Boot count, config save count, and OTA count are stored in Preferences (NVS).

## Security note

This example uses a simple password gate for config save, restart, and OTA update. It is fine for a trusted LAN test setup, but it is not hardened for internet exposure.
