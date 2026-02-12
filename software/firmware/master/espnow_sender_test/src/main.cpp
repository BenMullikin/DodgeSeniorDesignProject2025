#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <ArduinoJson.h>

// ESPNOW receiver MAC address (your ESP32-S3)
uint8_t receiverMac[] = {0x34, 0x85, 0x18, 0x96, 0x20, 0x9C};

String inputString = "";         // String to hold incoming serial data
bool stringComplete = false;     // Whether the string is complete
unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 100; // Send every 100ms if there's data

void sendKeyboardData();

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Reserve 256 bytes for the inputString to avoid fragmentation
  inputString.reserve(256);
  
  Serial.println();
  Serial.println("ESP32-C3 ESPNOW Keyboard Forwarder");
  Serial.println("===================================");
  
  // Set device as WiFi Station
  WiFi.mode(WIFI_STA);
  
  // Print MAC address
  Serial.print("C3 MAC Address: ");
  Serial.println(WiFi.macAddress());
  
  // Initialize ESPNOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESPNOW");
    return;
  }
  
  // Register peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMac, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  
  Serial.println("ESPNOW Sender Ready");
  Serial.print("Target MAC: ");
  for (int i = 0; i < 6; i++) {
    Serial.print(receiverMac[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
  Serial.println("===================================");
  Serial.println("Type anything and press Enter to send via ESPNOW");
  Serial.println("Characters will be forwarded in real-time!");
  Serial.println();
}

void loop() {
  // Check for serial input
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    
    // If the incoming character is a newline, set a flag
    if (inChar == '\n') {
      stringComplete = true;
    } else {
      // Add the character to the inputString
      inputString += inChar;
    }
  }
  
  // If we have data and enough time has passed, send it
  if (inputString.length() > 0 && millis() - lastSendTime >= SEND_INTERVAL) {
    sendKeyboardData();
    lastSendTime = millis();
  }
  
  // If we have a complete line (Enter pressed), send it immediately
  if (stringComplete) {
    if (inputString.length() > 0) {
      sendKeyboardData();
    }
    
    // Clear the string and reset flag
    inputString = "";
    stringComplete = false;
  }
  
  delay(10);
}

void sendKeyboardData() {
  // Create JSON with keyboard data
  DynamicJsonDocument doc(512);
  doc["type"] = "keyboard_input";
  doc["data"] = inputString;
  doc["length"] = inputString.length();
  doc["timestamp"] = millis();
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  // Send via ESPNOW
  esp_err_t result = esp_now_send(receiverMac, (uint8_t*)jsonString.c_str(), jsonString.length());
  
  if (result == ESP_OK) {
    Serial.print("Sent: \"");
    Serial.print(inputString);
    Serial.println("\"");
  } else {
    Serial.print("Error sending: ");
    Serial.println(result);
  }
  
  // Clear the string after sending
  inputString = "";
}