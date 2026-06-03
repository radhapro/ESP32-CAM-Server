# ESP32-CAM Viewer — HTML File

This is a browser-based viewer for the ESP32-CAM.  
Open this HTML file in any browser to capture and view live images from your ESP32-CAM over WiFi.

---

## What This File Does

- Connects to the ESP32-CAM web server using its IP address
- Fetches captured image data as JSON from the device
- Displays the raw JSON response
- Converts Base64 image data into a visible photograph
- Shows all three direct URLs as clickable links

---

## How to Use

### Step 1 — Find Your ESP32-CAM IP Address
Open Serial Monitor in Arduino IDE at 115200 baud and press Reset on the board.  
You will see:
```
[WiFi] Connected! IP: http://192.168.1.47/
```
Note down this IP address.

### Step 2 — Open the HTML File
Open `t.html` in Notepad and find this line:
```html
value="http://esp32cam.local"
```
Change it to your device IP:
```html
value="http://192.168.1.47"
```
Save the file — press **Ctrl + S**

### Step 3 — Open in Browser
Double-click the `t.html` file — it opens in your browser.

---

## Buttons

| Button | What It Does |
|---|---|
| **Fetch JSON** | Sends request to ESP32-CAM and downloads image data |
| **Show JSON** | Displays the raw JSON response from the device |
| **Convert to Image** | Decodes Base64 data and shows the photograph on screen |
| **Clear** | Clears the image and JSON from the screen |

---

## Direct URLs

Once the IP is set, the viewer also shows clickable direct links:

| Link | Opens |
|---|---|
| `http://192.168.1.47/` | ESP32-CAM built-in homepage |
| `http://192.168.1.47/get_image_json/` | Raw JSON with image data |
| `http://192.168.1.47/image` | Live photo directly in browser |

---

## How to Capture a Photo — Step by Step

1. Open `t.html` in browser
2. Click **Fetch JSON** — status shows `Done! [timestamp]`
3. Click **Convert to Image** — photo appears on screen
4. Click **Fetch JSON** again for a new photo
5. Click **Convert to Image** again to update

---

## Requirements

- ESP32-CAM must be powered on and connected to WiFi
- Your computer must be on the **same WiFi network** as the ESP32-CAM
- Any modern browser works — Chrome, Firefox, Edge

---

## Troubleshooting

| Problem | Solution |
|---|---|
| Fetch JSON fails | Check IP address is correct in the file |
| Image not showing | Always click Fetch JSON before Convert to Image |
| Status shows error | Make sure PC and ESP32-CAM are on same WiFi |
| Image is dark | Click Fetch JSON 2-3 times for camera warmup |

---

## License
MIT — free to use and modify.
