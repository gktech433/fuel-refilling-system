#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_camera.h"
#include "base64.h"  // Install Arduino Base64 library if needed
#include <esp_log.h>
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOS

// Replace with your WiFi credentials
const char* ssid = "***";
const char* password = "***";


// HTTPS Server details

// const char* server = "";
// const int httpsPort = 443;  // HTTPS default port
// const char* endpoint = "/";  // API endpoint

const char* serverUrl = "http://1";

// Camera configuration (for ESP32-CAM)
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

void setup() {
  Serial.begin(115200);
  esp_log_level_set("*", ESP_LOG_VERBOSE);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  delay(1000);
  WiFi.disconnect();
  delay(400);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi1");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi1");

  // Initialize camera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_VGA;  // Reduce from UXGA to VGA (640x480)
    config.jpeg_quality = 20;           // Increase compression for smaller size
    config.fb_count = 2;                // Keep double buffer for smoother performance
  } else {
    config.frame_size = FRAMESIZE_QVGA;  // Reduce to QVGA (320x240) for low-memory devices
    config.jpeg_quality = 25;            // More compression for smaller size
    config.fb_count = 1;                 // Single buffer for stability
  }

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed");
    return;
  }

  Serial.println("Camera Ready");
}

void loop() {
  Serial.println("Capturing photo...");

  // Capture a photo
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    delay(5000);
    return;
  }

  Serial.println("Encoding image to Base64...");
  // String base64Image = base64::encode(fb->buf, fb->len);


  // Send photo to server
  if (sendPhoto(fb)) {
    Serial.println("Photo uploaded successfully");
  } else {
    Serial.println("Photo upload failed");
  }

  // Release camera frame buffer
  esp_camera_fb_return(fb);

  delay(30000);  // Wait 10 seconds before next capture
}

bool sendPhoto(camera_fb_t* fb) {
  Serial.println("Free Heap Before: " + String(ESP.getFreeHeap()));
  Serial.println("Connecting to server1...");
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    Serial.println("Free Heap 1111: " + String(ESP.getFreeHeap()));

    Serial.println("Connecting to server2...");
    http.setTimeout(10000);
    http.begin(serverUrl);  // Connect to the server
    Serial.println("Free Heap 2222: " + String(ESP.getFreeHeap()));

    // http.addHeader("Content-Type", "application/json");
    http.addHeader("Content-Type", "application/octet-stream");

    // Create JSON payload
    // String jsonPayload = "{\"image\":\"test\"}";
    Serial.println("Free Heap 3333: " + String(ESP.getFreeHeap()));

    // int httpResponseCode = http.POST(jsonPayload);  // Send POST request
    int httpResponseCode = http.POST(fb->buf, fb->len);
    Serial.println("Free Heap 4444: " + String(ESP.getFreeHeap()));
    if (httpResponseCode > 0) {
      Serial.println("Free Heap 5555: " + String(ESP.getFreeHeap()));
      Serial.println("POST request successful");
      String response = http.getString();
      Serial.println("Response: " + response);
    } else {
      Serial.println("Free Heap 6666: " + String(ESP.getFreeHeap()));
      Serial.println("POST request failed, Error Code: " + String(httpResponseCode));
    }
    Serial.println("Free Heap After: " + String(ESP.getFreeHeap()));
    http.end();  // Close connection
  } else {
    Serial.println("Free Heap 7777: " + String(ESP.getFreeHeap()));
    Serial.println("WiFi not connected2");
    Serial.print("Connecting to WiFi2");
    WiFi.disconnect();
    delay(400);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi3");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.println("Free Heap 8888: " + String(ESP.getFreeHeap()));
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nConnected to WiFi2");
  }
}
