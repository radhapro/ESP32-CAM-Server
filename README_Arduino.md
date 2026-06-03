# ESP32-CAM ImageServer — Arduino Sketch

This is the firmware for the AI Thinker ESP32-CAM board.  
It starts a web server over WiFi and serves camera images through three direct URLs.

---

## What This Code Does

- Connects ESP32-CAM to your WiFi network
- Starts an HTTP web server on port 80
- Captures JPEG images from the camera on request
- Serves images as raw JPEG or Base64 encoded JSON
- Syncs time from the internet (IST timezone)
- Falls back to WiFiManager portal if WiFi credentials are wrong

---

## Available URLs After Upload

Replace `192.168.1.47` with your device IP shown in Serial Monitor.

| URL | Returns |
|---|---|
| `http://192.168.1.47/` | Built-in homepage with capture button |
| `http://192.168.1.47/get_image_json/` | JSON with Base64 image + timestamp + IP |
| `http://192.168.1.47/image` | Raw JPEG — opens directly in browser |

---

## Hardware Required

| Item | Details |
|---|---|
| ESP32-CAM | AI Thinker model |
| FTDI Programmer | USB to TTL, 3.3V |
| USB Cable | For FTDI to PC |
| 5x Jumper Wires | Female to Female |

---

## Wiring

| ESP32-CAM Pin | FTDI Pin | Notes |
|---|---|---|
| GND | GND | |
| 5V | VCC / 5V | |
| U0R (RXD) | TX | Cross-connect |
| U0T (TXD) | RX | Cross-connect |
| IO0 | GND | **Upload mode only — remove after upload** |

---

## Setup Steps

### 1. Install Arduino IDE
Download from: https://arduino.cc/en/software

### 2. Add ESP32 Board Support
Go to **File → Preferences → Additional Boards Manager URLs** and paste:
```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```
Then go to **Tools → Boards Manager**, search `esp32`, install **esp32 by Espressif Systems**

### 3. Install Library
Go to **Sketch → Include Library → Manage Libraries**  
Search: `WiFiManager` → Install **WiFiManager by tzapu**

### 4. Set Your WiFi Credentials
Open the `.ino` file and update these two lines at the top:
```cpp
#define ROBOZZ_SSID "YourWiFiName"
#define ROBOZZ_PASS "YourWiFiPassword"
```

### 5. Select Board and Port
- Board: **Tools → Board → esp32 → AI Thinker ESP32-CAM**
- Port: **Tools → Port → (your FTDI port)**

### 6. Upload
1. Connect IO0 pin to GND (upload mode)
2. Click **Upload** in Arduino IDE
3. Wait for: `Hard resetting via RTS pin...`
4. Remove IO0 wire
5. Press **Reset** button on board

---

## Serial Monitor Output (Normal Boot)

Open Serial Monitor at **115200 baud** and press Reset:

```
Booting...
[CAM] OK!
[WiFi] Connecting...
.......
[WiFi] Connected! IP: http://192.168.1.47/
[mDNS] Ready: http://esp32cam.local
[HTTP] Server ready!

Available URLs:
  http://192.168.1.47/
  http://192.168.1.47/get_image_json/
  http://192.168.1.47/image
```

---

## Troubleshooting

| Problem | Solution |
|---|---|
| Upload fails: timed out | Connect IO0 to GND before uploading |
| Device keeps restarting | Remove IO0-GND wire and press Reset |
| `[CAM] FAIL` in Serial Monitor | Re-seat the camera ribbon cable |
| WiFi not connecting | Check SSID and password in the code |
| `/image` returns 404 | Make sure you uploaded the latest version |
| Image is very dark | Refresh `/image` URL 2-3 times for warmup |

---

## License
MIT — free to use and modify.
