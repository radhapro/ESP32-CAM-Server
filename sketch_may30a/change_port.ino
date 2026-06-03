 #include "esp_camera.h"
#include "img_converters.h"
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <time.h>
#include "mbedtls/base64.h"

// Port 9950 set kiya gaya hai
WebServer server(9950); 

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define ROBOZZ_SSID "Robozz Lab"
#define ROBOZZ_PASS "Robotics@cloud"   

void syncTime() {
  configTime(19800, 0, "pool.ntp.org");
  struct tm t;
  int retry = 0;
  while (!getLocalTime(&t) && retry < 20) {
    delay(500);
    retry++;
  }
}

String getTimestamp() {
  struct tm t;
  if (!getLocalTime(&t)) return "1970-01-01T00:00:00Z";
  char buf[30];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &t);
  return String(buf);
}

bool initCamera() {
  camera_config_t cfg;
  cfg.ledc_channel = LEDC_CHANNEL_0;
  cfg.ledc_timer   = LEDC_TIMER_0;
  cfg.pin_d0       = Y2_GPIO_NUM;
  cfg.pin_d1       = Y3_GPIO_NUM;
  cfg.pin_d2       = Y4_GPIO_NUM;
  cfg.pin_d3       = Y5_GPIO_NUM;
  cfg.pin_d4       = Y6_GPIO_NUM;
  cfg.pin_d5       = Y7_GPIO_NUM;
  cfg.pin_d6       = Y8_GPIO_NUM;
  cfg.pin_d7       = Y9_GPIO_NUM;
  cfg.pin_xclk     = XCLK_GPIO_NUM;
  cfg.pin_pclk     = PCLK_GPIO_NUM;
  cfg.pin_vsync    = VSYNC_GPIO_NUM;
  cfg.pin_href     = HREF_GPIO_NUM;
  cfg.pin_sscb_sda = SIOD_GPIO_NUM;
  cfg.pin_sscb_scl = SIOC_GPIO_NUM;
  cfg.pin_pwdn     = PWDN_GPIO_NUM;
  cfg.pin_reset    = RESET_GPIO_NUM;
  cfg.xclk_freq_hz = 20000000;
  cfg.pixel_format = PIXFORMAT_RGB565;
  cfg.frame_size   = FRAMESIZE_QQVGA;
  cfg.jpeg_quality = 12;
  cfg.fb_count     = 1;

  esp_err_t err = esp_camera_init(&cfg);
  if (err != ESP_OK) {
    Serial.printf("[CAM] FAIL: 0x%x\n", err);
    return false;
  }
  Serial.println("[CAM] OK!");
  return true;
}

void handleGetImageJson() {
  camera_fb_t* warmup = esp_camera_fb_get();
  if (warmup) esp_camera_fb_return(warmup);

  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    server.send(500, "application/json", "{\"status\":\"error\",\"message\":\"Capture fail\"}");
    return;
  }

  uint8_t* jpg_buf = nullptr;
  size_t   jpg_len = 0;
  bool ok = frame2jpg(fb, 80, &jpg_buf, &jpg_len);
  esp_camera_fb_return(fb);

  if (!ok || !jpg_buf) {
    server.send(500, "application/json", "{\"status\":\"error\",\"message\":\"JPEG convert fail\"}");
    return;
  }

  size_t b64_len = 0;
  mbedtls_base64_encode(nullptr, 0, &b64_len, jpg_buf, jpg_len);
  uint8_t* b64_buf = (uint8_t*)malloc(b64_len + 1);

  if (!b64_buf) {
    free(jpg_buf);
    server.send(500, "application/json", "{\"status\":\"error\",\"message\":\"Memory fail\"}");
    return;
  }

  mbedtls_base64_encode(b64_buf, b64_len, &b64_len, jpg_buf, jpg_len);
  b64_buf[b64_len] = '\0';
  free(jpg_buf);

  String ip = WiFi.localIP().toString();
  String ts = getTimestamp();

  String json = "{";
  json += "\"status\":\"success\",";
  json += "\"data\":{";
  json += "\"ip_address\":\"" + ip + "\",";
  json += "\"image_base64\":\"" + String((char*)b64_buf) + "\",";
  json += "\"captured_at\":\"" + ts + "\"";
  json += "}}";

  free(b64_buf);

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
  Serial.println("[OK] JSON sent via port 9950");
}

void handleImage() {
  camera_fb_t* warmup = esp_camera_fb_get();
  if (warmup) esp_camera_fb_return(warmup);

  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) { server.send(500, "text/plain", "Capture failed"); return; }

  uint8_t* jpg_buf = nullptr;
  size_t   jpg_len = 0;
  bool ok = frame2jpg(fb, 80, &jpg_buf, &jpg_len);
  esp_camera_fb_return(fb);

  if (!ok || !jpg_buf) { server.send(500, "text/plain", "JPEG failed"); return; }

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send_P(200, "image/jpeg", (const char*)jpg_buf, jpg_len);
  free(jpg_buf);
}

void handleRoot() {
  server.send(200, "text/html",
    "<!DOCTYPE html><html><head>"
    "<title>ESP32-CAM (Port 9950)</title>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<style>"
    "body{background:#111;color:#0f0;font-family:monospace;"
    "display:flex;flex-direction:column;align-items:center;"
    "padding:20px;margin:0}"
    "h2{color:#0f0;margin-bottom:16px}"
    ".btn{display:inline-block;margin:6px;padding:14px 32px;"
    "border:2px solid #0f0;border-radius:6px;color:#0f0;"
    "background:#000;font-family:monospace;font-size:1em;"
    "cursor:pointer;transition:all 0.2s}"
    ".btn:hover{background:#0f0;color:#000}"
    "#status{color:#aaa;font-size:0.85em;margin:12px 0}"
    "</style></head><body>"
    "<h2>ESP32-CAM @ Port 9950</h2>"
    "<button class='btn' onclick='capture()'>Capture</button>"
    "<div id='status'>Ready</div>"
    "<script>"
    "function capture(){"
    "  document.getElementById('status').innerText='Capturing...';"
    "  fetch('/get_image_json/')"
    "  .then(r=>r.json())"
    "  .then(d=>{"
    "    document.getElementById('status').innerText='Done! '+d.data.captured_at;"
    "  })"
    "  .catch(e=>{"
    "    document.getElementById('status').innerText='Fail!';"
    "  });"
    "}"
    "</script>"
    "</body></html>"
  );
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting...");

  if (!initCamera()) {
    Serial.println("Camera fail! Restarting...");
    delay(3000);
    ESP.restart();
  }

  Serial.println("[WiFi] Robozz Lab try kar raha hai...");
  WiFi.begin(ROBOZZ_SSID, ROBOZZ_PASS);

  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 20) {
    delay(500);
    Serial.print(".");
    tries++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n[WiFi] Robozz not found! Portal start...");
    WiFi.disconnect();
    WiFiManager wm;
    wm.setConfigPortalTimeout(180);
    bool connected = wm.startConfigPortal("ESP32-CAM-Setup");
    if (!connected) {
      Serial.println("[WiFi] Portal also fail! Restarting...");
      delay(3000);
      ESP.restart();
    }
  }

  Serial.print("\n[WiFi] Connected! IP: http://");
  Serial.print(WiFi.localIP());
  Serial.println(":9950/"); // Port update in serial

  if (MDNS.begin("esp32cam")) {
    Serial.println("[mDNS] Ready! http://esp32cam.local:9950");
  }

  syncTime();

  server.on("/",                HTTP_GET, handleRoot);
  server.on("/get_image_json/", HTTP_GET, handleGetImageJson);
  server.on("/image",           HTTP_GET, handleImage);
  server.onNotFound([]() { server.send(404, "application/json", "{\"status\":\"error\",\"message\":\"Not found\"}"); });
  
  server.begin();
  Serial.println("[HTTP] Server ready on port 9950!");
}

void loop() {
  server.handleClient();
}