#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

class WebServerManager {
public:
  WebServerManager(uint16_t port = 80);
  
  bool begin();
  void setCaptivePortal(bool enable);
  
private:
  AsyncWebServer server_;
  bool captive_portal_enabled_;
  
  void setupRoutes();
  void setupCaptivePortal();
  String getContentType(const String& filename);
};

#endif