#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_camera.h"
#include "base64.h" // Install Arduino Base64 library if needed
#include <esp_log.h>
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOS

// Replace with your WiFi credentials

const char* ssid = "******";
const char* password = "**********";

// HTTPS Server details

// const char* server = "y5f6airjixal4farjpegitu4re0mnyrd.lambda-url.us-east-2.on.aws";
// const int httpsPort = 443;  // HTTPS default port
// const char* endpoint = "/";  // API endpoint

const char* serverUrl = "http://";

// Camera configuration (for ESP32-CAM)
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

void setup() {
    Serial.begin(115200);
    esp_log_level_set("*", ESP_LOG_VERBOSE);
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");

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
    
    if(psramFound()) {
        config.frame_size = FRAMESIZE_UXGA;
        config.jpeg_quality = 10;
        config.fb_count = 2;
    } else {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
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
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        delay(5000);
        return;
    }

    Serial.println("Encoding image to Base64...");
    String base64Image = base64::encode(fb->buf, fb->len);

    // Release camera frame buffer
    esp_camera_fb_return(fb);

    // Send photo to server
    if (sendPhoto(base64Image)) {
        Serial.println("Photo uploaded successfully");
    } else {
        Serial.println("Photo upload failed");
    }

    delay(30000); // Wait 10 seconds before next capture
}

bool sendPhoto(String base64Image) {
    Serial.println("Connecting to server...");
    if (!client.connect(server, httpsPort)) {
        Serial.println("Connection to server failed");
        return false;
    }

    // Create JSON payload
    // String jsonPayload = "{\"image\":\"" + base64Image + "\"}";
    String jsonPayload = "{\"image\":\"test\"}";

    // Send HTTP POST request
    client.println("POST " + String(endpoint) + " HTTP/1.1");
    client.println("Host: " + String(server));
    client.println("User-Agent: ESP32-CAM");
    client.println("Content-Type: application/json");
    client.println("Content-Length: " + String(jsonPayload.length()));
    client.println();
    client.println(jsonPayload);

    // Wait for response
    while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
            break;
        }
    }

    // Read response
    String response = client.readString();
    Serial.println("Server response: " + response);

    return response.indexOf("success") != -1;
}
