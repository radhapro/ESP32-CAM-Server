 # ESP32-CAM Server

## Kya Karta Hai Ye Code?
ESP32-CAM se photo capture karta hai aur 3 kaam karta hai:
- **JPEG image** direct browser mein dikhata hai
- **JSON format** mein Base64 image data deta hai
- **Web UI** se capture, JSON dekho, aur image convert karo

## Features
- WiFiManager - pehli baar hotspot se WiFi connect karo
- NTP Time Sync - IST time stamp har image pe
- 3 Buttons - Capture / Show JSON / Convert to Image
- Dark theme web interface

## Hardware
- ESP32-CAM (AI Thinker)

## API Endpoints
| URL | Kaam |
|-----|------|
| `/` | Web UI |
| `/get_image/` | Direct JPEG |
| `/get_image_json/` | Base64 JSON |

## Setup
1. Code upload karo ESP32-CAM pe
2. `ESP32-CAM-Setup` WiFi se connect karo
3. Apna WiFi password daalo
4. Serial monitor mein IP dekho
5. Browser mein `http://[IP]/` kholo

## Libraries Needed
- ESP32 Camera
- WiFiManager
- WebServer
- mbedtls/base64