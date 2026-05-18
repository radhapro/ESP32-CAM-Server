#include "esp_camera.h"
#include "img_converters.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

const char* WIFI_SSID     = "Robozz Lab";
const char* WIFI_PASSWORD = "Robotics@cloud";

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
  cfg.frame_size   = FRAMESIZE_QVGA;
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
  Serial.printf("[OK] %u bytes\n", jpg_len);
}

void handleGetDetails() {
  sensor_t* s = esp_camera_sensor_get();
  if (!s) {
    server.send(500, "text/plain", "Sensor read fail!");
    return;
  }

  String json = "{";
  json += "\"resolution\":\"320x240\",";
  json += "\"brightness\":" + String(s->status.brightness) + ",";
  json += "\"contrast\":"   + String(s->status.contrast)   + ",";
  json += "\"saturation\":" + String(s->status.saturation) + ",";
  json += "\"sharpness\":"  + String(s->status.sharpness)  + ",";
  json += "\"whitebal\":"   + String(s->status.awb)        + ",";
  json += "\"exposure\":"   + String(s->status.aec)        + ",";
  json += "\"gain\":"       + String(s->status.agc)        + ",";
  json += "\"quality\":80";
  json += "}";

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
  Serial.println("[OK] Details bheje");
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
    "border-radius:6px;margin:12px auto;display:block;transition:opacity 0.3s}"
    ".btn{display:inline-block;margin:6px;padding:11px 26px;"
    "border:2px solid #0f0;border-radius:6px;color:#0f0;"
    "background:#000;font-family:monospace;font-size:0.95em;"
    "cursor:pointer;text-decoration:none;transition:all 0.2s}"
    ".btn:hover{background:#0f0;color:#000}"
    ".btn.blue{border-color:#0ff;color:#0ff}"
    ".btn.blue:hover{background:#0ff;color:#000}"
    ".btn.red{border-color:#f55;color:#f55}"
    ".btn.red:hover{background:#f55;color:#000}"
    "#status{color:#aaa;font-size:0.82em;margin:4px 0}"
    "#dl{display:none}"
    "#details-box{margin-top:16px;border:1px solid #0f0;border-radius:6px;"
    "padding:14px;width:90vw;max-width:400px;display:none}"
    "#details-box h3{margin:0 0 10px;color:#0ff;font-size:1em}"
    ".row{display:flex;justify-content:space-between;padding:4px 0;"
    "border-bottom:1px solid #222;font-size:0.88em}"
    ".row:last-child{border-bottom:none}"
    ".label{color:#aaa}"
    ".value{color:#0f0;font-weight:bold}"
    "</style></head><body>"
    "<h2>ESP32-CAM</h2>"
    "<div>"
    "<button class='btn' onclick='capture()'>Capture</button>"
    "<button class='btn blue' onclick='getDetails()'>Image Details</button>"
    "<button class='btn red' onclick='window.history.back()'>Back</button>"
    "</div>"
    "<div id='status'>Ready</div>"
    "<img id='photo' src='/get_image/' alt='photo'>"
    "<a id='dl' class='btn' download='photo.jpg'>Download</a>"

    "<div id='details-box'>"
    "<h3>Image Details</h3>"
    "<div class='row'><span class='label'>Resolution</span><span class='value' id='d-res'>-</span></div>"
    "<div class='row'><span class='label'>Brightness</span><span class='value' id='d-br'>-</span></div>"
    "<div class='row'><span class='label'>Contrast</span><span class='value' id='d-con'>-</span></div>"
    "<div class='row'><span class='label'>Saturation</span><span class='value' id='d-sat'>-</span></div>"
    "<div class='row'><span class='label'>Sharpness</span><span class='value' id='d-sharp'>-</span></div>"
    "<div class='row'><span class='label'>White Balance</span><span class='value' id='d-wb'>-</span></div>"
    "<div class='row'><span class='label'>Auto Exposure</span><span class='value' id='d-exp'>-</span></div>"
    "<div class='row'><span class='label'>Auto Gain</span><span class='value' id='d-gain'>-</span></div>"
    "<div class='row'><span class='label'>JPEG Quality</span><span class='value' id='d-q'>-</span></div>"
    "</div>"

    "<script>"
    "function capture(){"
    "  var s=document.getElementById('status');"
    "  var img=document.getElementById('photo');"
    "  var dl=document.getElementById('dl');"
    "  s.innerText='Capturing...';"
    "  img.style.opacity='0.3';"
    "  dl.style.display='none';"
    "  var url='/get_image/?t='+Date.now();"
    "  img.onload=function(){"
    "    img.style.opacity='1';"
    "    s.innerText='Done! '+new Date().toLocaleTimeString();"
    "    dl.href=url;dl.style.display='inline-block';"
    "  };"
    "  img.src=url;"
    "}"
    "function getDetails(){"
    "  document.getElementById('status').innerText='Fetching details...';"
    "  fetch('/get_details/')"
    "  .then(r=>r.json())"
    "  .then(d=>{"
    "    document.getElementById('d-res').innerText=d.resolution;"
    "    document.getElementById('d-br').innerText=d.brightness;"
    "    document.getElementById('d-con').innerText=d.contrast;"
    "    document.getElementById('d-sat').innerText=d.saturation;"
    "    document.getElementById('d-sharp').innerText=d.sharpness;"
    "    document.getElementById('d-wb').innerText=d.whitebal?'Auto':'Manual';"
    "    document.getElementById('d-exp').innerText=d.exposure?'Auto':'Manual';"
    "    document.getElementById('d-gain').innerText=d.gain?'Auto':'Manual';"
    "    document.getElementById('d-q').innerText=d.quality;"
    "    document.getElementById('details-box').style.display='block';"
    "    document.getElementById('status').innerText='Details loaded!';"
    "  })"
    "  .catch(e=>{"
    "    document.getElementById('status').innerText='Details fetch fail!';"
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
    Serial.println("Camera fail! Restart...");
    delay(3000);
    ESP.restart();
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("WiFi connecting");
  int t = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
    if (++t > 40) { ESP.restart(); }
  }
  Serial.print("\nIP: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/get_image/");

  server.on("/",            HTTP_GET, handleRoot);
  server.on("/get_image/",  HTTP_GET, handleGetImage);
  server.on("/get_details/",HTTP_GET, handleGetDetails);
  server.onNotFound([]() { server.send(404, "text/plain", "Try /get_image/"); });
  server.begin();
  Serial.println("Server ready!");
}

void loop() {
  server.handleClient();
}