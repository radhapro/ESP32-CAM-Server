#include "esp_camera.h"
#include "img_converters.h"
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <time.h>
#include "mbedtls/base64.h"

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

WebServer server(80);

void syncTime() {
  configTime(19800, 0, "pool.ntp.org");
  Serial.print("[TIME] Syncing");
  struct tm t;
  int retry = 0;
  while (!getLocalTime(&t) && retry < 20) {
    delay(500);
    Serial.print(".");
    retry++;
  }
  Serial.println(" Done!");
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

void handleGetImage() {
  camera_fb_t* warmup = esp_camera_fb_get();
  if (warmup) esp_camera_fb_return(warmup);

  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) { server.send(500, "text/plain", "Capture fail!"); return; }

  uint8_t* jpg_buf = nullptr;
  size_t   jpg_len = 0;
  bool ok = frame2jpg(fb, 80, &jpg_buf, &jpg_len);
  esp_camera_fb_return(fb);

  if (!ok || !jpg_buf) { server.send(500, "text/plain", "JPEG fail!"); return; }

  WiFiClient client = server.client();
  client.print(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: image/jpeg\r\n"
    "Content-Length: " + String(jpg_len) + "\r\n"
    "Connection: close\r\n\r\n"
  );
  client.write(jpg_buf, jpg_len);
  free(jpg_buf);
  Serial.printf("[OK] JPEG %u bytes\n", jpg_len);
}

void handleGetImageJson() {
  camera_fb_t* warmup = esp_camera_fb_get();
  if (warmup) esp_camera_fb_return(warmup);

  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    server.send(500, "application/json",
      "{\"status\":\"error\",\"message\":\"Capture fail\"}");
    return;
  }

  uint8_t* jpg_buf = nullptr;
  size_t   jpg_len = 0;
  bool ok = frame2jpg(fb, 80, &jpg_buf, &jpg_len);
  esp_camera_fb_return(fb);

  if (!ok || !jpg_buf) {
    server.send(500, "application/json",
      "{\"status\":\"error\",\"message\":\"JPEG convert fail\"}");
    return;
  }

  size_t b64_len = 0;
  mbedtls_base64_encode(nullptr, 0, &b64_len, jpg_buf, jpg_len);
  uint8_t* b64_buf = (uint8_t*)malloc(b64_len + 1);

  if (!b64_buf) {
    free(jpg_buf);
    server.send(500, "application/json",
      "{\"status\":\"error\",\"message\":\"Memory fail\"}");
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
  Serial.println("[OK] JSON image sent");
}

void handleRoot() {
  server.send(200, "text/html",
    "<!DOCTYPE html><html><head>"
    "<title>ESP32-CAM</title>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<style>"
    "body{background:#111;color:#0f0;font-family:monospace;"
    "display:flex;flex-direction:column;align-items:center;"
    "padding:20px;margin:0}"
    "h2{color:#0f0;margin-bottom:4px}"
    "img{max-width:90vw;max-height:50vh;border:2px solid #0f0;"
    "border-radius:6px;margin:12px auto;display:block}"
    ".btn{display:inline-block;margin:6px;padding:11px 26px;"
    "border:2px solid #0f0;border-radius:6px;color:#0f0;"
    "background:#000;font-family:monospace;font-size:0.95em;"
    "cursor:pointer;text-decoration:none;transition:all 0.2s}"
    ".btn:hover{background:#0f0;color:#000}"
    ".btn.blue{border-color:#0ff;color:#0ff}"
    ".btn.blue:hover{background:#0ff;color:#000}"
    ".btn.yellow{border-color:#ff0;color:#ff0}"
    ".btn.yellow:hover{background:#ff0;color:#000}"
    "#status{color:#aaa;font-size:0.82em;margin:4px 0}"
    "#json-box{margin-top:16px;border:1px solid #0ff;border-radius:6px;"
    "padding:14px;width:90vw;max-width:500px;display:none;"
    "word-break:break-all;font-size:0.75em;color:#0ff;text-align:left}"
    "#converted-img{display:none}"
    "</style></head><body>"
    "<h2>ESP32-CAM</h2>"
    "<div>"
    "<button class='btn' onclick='capture()'>Capture</button>"
    "<button class='btn blue' onclick='showJson()'>Show JSON</button>"
    "<button class='btn yellow' onclick='convertToImage()'>Convert to Image</button>"
    "</div>"
    "<div id='status'>Ready</div>"
    "<div id='json-box'></div>"
    "<img id='converted-img' alt='converted'/>"
    "<script>"
    "var lastB64='';"

    "function capture(){"
    "  document.getElementById('status').innerText='Capturing...';"
    "  document.getElementById('json-box').style.display='none';"
    "  document.getElementById('converted-img').style.display='none';"
    "  fetch('/get_image_json/')"
    "  .then(r=>r.json())"
    "  .then(d=>{"
    "    lastB64=d.data.image_base64;"
    "    document.getElementById('status').innerText='Captured! '+d.data.captured_at;"
    "  })"
    "  .catch(e=>{"
    "    document.getElementById('status').innerText='Capture fail!';"
    "  });"
    "}"

    "function showJson(){"
    "  if(!lastB64){"
    "    document.getElementById('status').innerText='Pehle Capture karo!';"
    "    return;"
    "  }"
    "  var box=document.getElementById('json-box');"
    "  box.style.display='block';"
    "  box.innerText='{\"status\":\"success\",\"data\":{\"image_base64\":\"'+lastB64+'\"}';"
    "  document.getElementById('status').innerText='JSON ready!';"
    "}"

    "function convertToImage(){"
    "  if(!lastB64){"
    "    document.getElementById('status').innerText='Pehle Capture karo!';"
    "    return;"
    "  }"
    "  var img=document.getElementById('converted-img');"
    "  img.src='data:image/jpeg;base64,'+lastB64;"
    "  img.style.display='block';"
    "  document.getElementById('status').innerText='Image ready!';"
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

  WiFiManager wm;
  bool connected = wm.autoConnect("ESP32-CAM-Setup");
  if (!connected) {
    Serial.println("[WiFi] Failed! Restarting...");
    delay(3000);
    ESP.restart();
  }

  Serial.print("[WiFi] Connected! IP: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

  syncTime();

  server.on("/",                HTTP_GET, handleRoot);
  server.on("/get_image/",      HTTP_GET, handleGetImage);
  server.on("/get_image_json/", HTTP_GET, handleGetImageJson);
  server.onNotFound([]() { server.send(404, "application/json",
    "{\"status\":\"error\",\"message\":\"Not found\"}"); });
  server.begin();
  Serial.println("[HTTP] Server ready!");
}

void loop() {
  server.handleClient();
}