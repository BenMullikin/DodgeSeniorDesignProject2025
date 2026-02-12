#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <ArduinoJson.h>

// ESPNOW peer MAC addresses (your C3 sender)
uint8_t senderMac[] = {0x84, 0xFC, 0xE6, 0x00, 0xFD, 0x74};

unsigned long lastReceiveTime = 0;
bool connectionActive = false;

// ESPNOW receive callback
void onESPNOWReceive(const uint8_t *mac, const uint8_t *data, int len) {
  // Parse JSON data
  String jsonString = String((char*)data).substring(0, len);
  
  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, jsonString);
  
  if (error) {
    Serial.print("JSON parsing failed: ");
    Serial.println(error.c_str());
    return;
  }
  
  // Check if it's keyboard data
  const char* type = doc["type"];
  if (strcmp(type, "keyboard_input") == 0) {
    const char* keyboardData = doc["data"];
    int dataLength = doc["length"];
    unsigned long timestamp = doc["timestamp"];
    
    
    // Update display
    Serial.println();
    Serial.println("=== KEYBOARD INPUT RECEIVED ===");
    Serial.print("From: ");
    for (int i = 0; i < 6; i++) {
      Serial.print(mac[i], HEX);
      if (i < 5) Serial.print(":");
    }
    Serial.println();
    Serial.print("Data: \"");
    Serial.print(keyboardData);
    Serial.println("\"");
    Serial.print("Length: ");
    Serial.println(dataLength);

    
    lastReceiveTime = millis();
    
    if (!connectionActive) {
      connectionActive = true;
      Serial.println();
      Serial.println("KEYBOARD CONNECTION ACTIVE - Ready for input!");
      Serial.println();
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println();
  Serial.println("ESP32-S3 ESPNOW Keyboard Receiver");
  Serial.println("==================================");
  
  // Set device as WiFi Station
  WiFi.mode(WIFI_STA);
  
  // Print MAC address for sender configuration
  Serial.print("S3 MAC Address: ");
  Serial.println(WiFi.macAddress());
  
  // Initialize ESPNOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESPNOW");
    return;
  }
  
  // Register receive callback
  esp_now_register_recv_cb(onESPNOWReceive);
  
  // Add peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, senderMac, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
  } else {
    Serial.println("ESPNOW Receiver Ready - Waiting for keyboard input...");
  }
  
  Serial.println("==================================");
  Serial.println("Waiting for keyboard connection...");
  Serial.println();
}

void loop() {
  // Check for connection timeout
  if (connectionActive && millis() - lastReceiveTime > 5000) {
    Serial.println();
    Serial.println("No keyboard input received for 5 seconds");
    Serial.println("   Connection may be inactive");
    connectionActive = false;
  }
  
  
  delay(1000);
}