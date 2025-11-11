#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <esp_now.h>
#include <LittleFS.h>
#include "WiFiManager/WiFiManager.h"
#include "ESPNOWManager/ESPNOWManager.h"
#include "WebServerManager/WebServerManager.h"
#include "WebSocketManager/WebSocketManager.h"
#include "SensorData/SensorData.h"

// Configuration
const char* WIFI_SSID = "SensorHub-Center";
const char* WIFI_PASSWORD = "sensor123";

// ESPNOW peer MAC addresses (update with real addresses)
uint8_t SENSOR_PEERS[][6] = {
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  // Sensor 0
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  // Sensor 1
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  // Sensor 2
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}   // Sensor 3
};

// Global Managers
WiFiManager wifi_manager(WIFI_SSID, WIFI_PASSWORD);
ESPNOWManager espnow_manager;
WebServerManager web_server_manager;
WebSocketManager websocket_manager;
SensorDataManager sensor_data_manager;

// Forward Declarations
void setupManagers();
void onESPNOWData(const uint8_t* mac, const uint8_t* data, int len);
void onWebSocketConnect(uint8_t client_id);
void onWebSocketDisconnect(uint8_t client_id);
int findSensorIdByMac(const uint8_t* mac);
void simulateSensorDataForTesting();

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println();
  Serial.println("ESP32-S3 Central Receiver Starting...");
  Serial.println("======================================");
  
  setupManagers();
  
  Serial.println("All systems initialized!");
  Serial.println("Connect to WiFi: " + String(WIFI_SSID));
  Serial.println("======================================");
}

void loop() {
  websocket_manager.loop();
  
  // Simulate data for testing
  static unsigned long last_simulation = 0;
  if (millis() - last_simulation > 3000) {
    simulateSensorDataForTesting();
    last_simulation = millis();
  }
  
  // Check for stale sensor data
  static unsigned long last_stale_check = 0;
  if (millis() - last_stale_check > 5000) {
    sensor_data_manager.markAllStale();
    websocket_manager.broadcast(sensor_data_manager.toJSON());
    last_stale_check = millis();
  }
  
  delay(10);
}

void setupManagers() {
  // 1. WiFi
  if (!wifi_manager.begin()) {
    Serial.println("WiFi setup failed!");
    return;
  }
  
  // 2. ESPNOW
  if (!espnow_manager.begin()) {
    Serial.println("ESPNOW setup failed!");
    return;
  }
  
  // Add ESPNOW peers
  for (int i = 0; i < 4; i++) {
    espnow_manager.addPeer(SENSOR_PEERS[i]);
  }
  
  // Set ESPNOW callback
  espnow_manager.setCallback(onESPNOWData);
  
  // 3. Web Server
  if (!web_server_manager.begin()) {
    Serial.println("Web server setup failed!");
    return;
  }
  
  // 4. WebSocket
  websocket_manager.setConnectCallback(onWebSocketConnect);
  websocket_manager.setDisconnectCallback(onWebSocketDisconnect);
  websocket_manager.begin();
}

void onESPNOWData(const uint8_t* mac, const uint8_t* data, int len) {
  Serial.print("ESPNOW Data from: ");
  for (int i = 0; i < 6; i++) {
    Serial.print(mac[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
  
  // Parse JSON data
  String jsonString = String((char*)data).substring(0, len);
  Serial.println("Raw JSON: " + jsonString);
  
  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, jsonString);
  
  if (error) {
    Serial.println("JSON parsing failed!");
    return;
  }
  
  // Find which sensor sent the data
  int sensor_id = findSensorIdByMac(mac);
  if (sensor_id == -1) {
    Serial.println("Unknown sensor MAC");
    return;
  }
  
  // Update sensor data
  sensor_data_manager.updateSensor(
    sensor_id,
    doc["distance_mm"] | 0.0,
    doc["stable"] | false,
    doc["ch1_active"] | false,
    doc["ch2_active"] | false
  );
  
  Serial.println("Updated Sensor " + String(sensor_id));
  
  // Broadcast to WebSocket clients
  websocket_manager.broadcast(sensor_data_manager.toJSON());
}

int findSensorIdByMac(const uint8_t* mac) {
  for (int i = 0; i < 4; i++) {
    if (memcmp(mac, SENSOR_PEERS[i], 6) == 0) {
      return i;
    }
  }
  return -1;
}

void onWebSocketConnect(uint8_t client_id) {
  Serial.println("WebSocket Client " + String(client_id) + " connected");
  websocket_manager.sendToClient(client_id, sensor_data_manager.toJSON());
}

void onWebSocketDisconnect(uint8_t client_id) {
  Serial.println("WebSocket Client " + String(client_id) + " disconnected");
}

void simulateSensorDataForTesting() {
  for (int i = 0; i < SensorDataManager::NUM_SENSORS; i++) {
    sensor_data_manager.updateSensor(
      i,
      1000 + (i * 500) + random(-100, 100),
      random(0, 10) > 2,
      random(0, 2),
      random(0, 2)
    );
  }
  
  Serial.println("Simulated sensor data updated");
  websocket_manager.broadcast(sensor_data_manager.toJSON());
}