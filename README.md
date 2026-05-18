 # ESP32-CAM Image Server

## What This Project Does
This project connects the ESP32-CAM to WiFi and runs an HTTP server.
Open the IP address in any browser and get a live camera image instantly.

## Hardware Used
- ESP32-CAM module (RHYX M21-45 sensor)
- ESP32-CAM-MB shield (for USB flashing)

## What We Did
- Connected ESP32-CAM to WiFi
- Created an HTTP server on port 80
- Built a `/get_image/` endpoint that returns a fresh JPEG photo
- Fixed RHYX M21-45 sensor issue (used RGB565 format then converted to JPEG)
- Added Capture, Back and Download buttons in the browser UI

## Problems We Faced and How We Fixed Them
| Problem | Fix |
|---|---|
| JPEG format not supported on sensor | Used RGB565 mode and converted using `frame2jpg()` |
| Wrong COM port error (COM5) | Selected correct port COM4 in Arduino IDE |
| Camera init failed | Changed pixel format to RGB565 |

## API Endpoints
| Endpoint | Description |
|---|---|
| `GET /` | Main page with Capture, Back, Download buttons |
| `GET /get_image/` | Returns a fresh JPEG image |

## How to Flash
1. Open Arduino IDE
2. Select Board → `AI Thinker ESP32-CAM`
3. Select Port → `COM4`
4. Enter your WiFi name and password in the code
5. Hold IO0 button → Press RST → Click Upload
6. After upload press RST once → Check Serial Monitor for IP

## How to Use
1. Open Serial Monitor at 115200 baud
2. Note the IP address shown (example: `192.168.29.243`)
3. Open browser and go to `http://192.168.29.243/`
4. Click **Capture** to take a photo
5. Click **Download** to save the photo

## WiFi Setup
```cpp
const char* WIFI_SSID     = "Your_WiFi_Name";
const char* WIFI_PASSWORD = "Your_Password";
```

## Libraries Used
- `esp_camera.h` — Camera control
- `img_converters.h` — RGB565 to JPEG conversion
- `WiFi.h` — WiFi connection
- `WebServer.h` — HTTP server

## Result Images
![photo](images/photo.jpg)
![photo1](images/photo1.jpg)
![photo2](images/photo2.jpg)
![photo3](images/photo3.jpg)
![photo4](images/photo4.jpg)
![photo5](images/photo5.jpg)