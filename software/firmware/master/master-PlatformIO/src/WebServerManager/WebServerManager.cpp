#include "WebServerManager.h"

WebServerManager::WebServerManager(uint16_t port) 
  : server_(port), captive_portal_enabled_(true) {
}

bool WebServerManager::begin() {
  Serial.println("Initializing Web Server...");
  
  if (!LittleFS.begin()) {
    Serial.println("LittleFS initialization failed!");
    return false;
  }
  
  setupRoutes();
  
  if (captive_portal_enabled_) {
    setupCaptivePortal();
  }
  
  server_.begin();
  Serial.println("Web server started"); // Removed port() call
  return true;
}

void WebServerManager::setCaptivePortal(bool enable) {
  captive_portal_enabled_ = enable;
}

void WebServerManager::setupRoutes() {
  server_.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
  
  server_.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", "{\"status\":\"online\"}");
  });
}

void WebServerManager::setupCaptivePortal() {
  server_.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect("/");
  });
  
  server_.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect("/");
  });
  
  server_.onNotFound([](AsyncWebServerRequest *request) {
    Serial.println("Captive portal redirect: " + String(request->url().c_str()));
    request->send(LittleFS, "/index.html", "text/html");
  });
}

String WebServerManager::getContentType(const String& filename) {
  if (filename.endsWith(".html")) return "text/html";
  if (filename.endsWith(".css")) return "text/css";
  if (filename.endsWith(".js")) return "application/javascript";
  if (filename.endsWith(".png")) return "image/png";
  if (filename.endsWith(".jpg")) return "image/jpeg";
  return "text/plain";
}