#include "ESPNOWManager.h"

ESPNOWCallback ESPNOWManager::user_callback_ = nullptr;

ESPNOWManager::ESPNOWManager() {
}

bool ESPNOWManager::begin() {
  Serial.println("Initializing ESPNOW...");
  
  WiFi.mode(WIFI_STA);
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESPNOW initialization failed!");
    return false;
  }
  
  esp_now_register_recv_cb(ESPNOWManager::onDataReceived);
  Serial.println("ESPNOW initialized successfully");
  return true;
}

bool ESPNOWManager::addPeer(uint8_t* mac_address) {
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, mac_address, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add ESPNOW peer");
    return false;
  }
  
  Serial.print("Added ESPNOW peer: ");
  for (int i = 0; i < 6; i++) {
    Serial.print(mac_address[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
  return true;
}

void ESPNOWManager::setCallback(ESPNOWCallback callback) {
  user_callback_ = callback;
}

void ESPNOWManager::sendData(uint8_t* mac, uint8_t* data, int len) {
  esp_now_send(mac, data, len);
}

void ESPNOWManager::onDataReceived(const uint8_t* mac, const uint8_t* data, int len) {
  if (user_callback_ != nullptr) {
    user_callback_(const_cast<uint8_t*>(mac), const_cast<uint8_t*>(data), len);
  }
}