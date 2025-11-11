#ifndef ESPNOW_MANAGER_H
#define ESPNOW_MANAGER_H

#include <esp_now.h>
#include <WiFi.h>
#include <functional>

typedef std::function<void(uint8_t* mac, uint8_t* data, int len)> ESPNOWCallback;

class ESPNOWManager {
public:
  ESPNOWManager();
  
  bool begin();
  bool addPeer(uint8_t* mac_address);
  void setCallback(ESPNOWCallback callback);
  void sendData(uint8_t* mac, uint8_t* data, int len);
  
private:
  static void onDataReceived(const uint8_t* mac, const uint8_t* data, int len);
  static ESPNOWCallback user_callback_;
};

#endif