# final.ino - ESP32-CAM Web Server

## Kya Karta Hai?
ESP32-CAM se photo capture karke web browser mein dikhata hai.

## Features
- WiFiManager - pehli baar hotspot se WiFi setup
- NTP Time Sync - IST timestamp har image pe
- 3 Buttons - Capture / Show JSON / Convert to Image
- Dark theme web UI

## Hardware
- ESP32-CAM (AI Thinker)

## API Endpoints
| URL | Kaam |
|-----|------|
| `/` | Web UI |
| `/get_image/` | Direct JPEG |
| `/get_image_json/` | Base64 JSON |

## Libraries
- ESP32 Camera, WiFiManager, WebServer, mbedtls/base64