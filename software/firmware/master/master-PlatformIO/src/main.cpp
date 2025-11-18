#include <Arduino.h>
#include "WiFiManager/WiFiManager.h"
#include "WebServerManager/WebServerManager.h"

// Configuration
const char* WIFI_SSID = "SensorHub-Center";
const char* WIFI_PASSWORD = "sensor123";

// Global Managers
WiFiManager wifi_manager(WIFI_SSID, WIFI_PASSWORD);
WebServerManager web_server_manager;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println();
  Serial.println("ESP32-S3 Central Receiver - Step 1: Captive Portal");
  Serial.println("==================================================");
  
  // Initialize WiFi
  if (!wifi_manager.begin()) {
    Serial.println("WiFi setup failed! Stopping.");
    return;
  }
  
  // Print MAC address for reference
  Serial.print("MAC Address: ");
  Serial.println(wifi_manager.getMACAddress());
  
  // Initialize Web Server
  if (!web_server_manager.begin()) {
    Serial.println("Web server setup failed! Stopping.");
    return;
  }
  
  Serial.println("System initialized successfully!");
  Serial.println("Connect to WiFi: " + String(WIFI_SSID));
  Serial.println("WebApp should open automatically in your browser");
  Serial.println("==================================================");
}

void loop() {
  web_server_manager.handleClient();
  
  // Simple heartbeat indicator
  static unsigned long last_blink = 0;
  if (millis() - last_blink > 5000) {
    Serial.println("System running...");
    last_blink = millis();
  }
  
  delay(10);
}