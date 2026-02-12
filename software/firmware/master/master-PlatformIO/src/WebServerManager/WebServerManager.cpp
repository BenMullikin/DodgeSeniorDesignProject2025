#include "WebServerManager.h"

WebServerManager::WebServerManager() : server(80) {
}

bool WebServerManager::begin() {
  Serial.println("Initializing Web Server with HTTP Streaming...");
  
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS initialization failed!");
    return false;
  }
  Serial.println("LittleFS mounted successfully");
  
  // Start DNS server for captive portal
  dnsServer.start(53, "*", WiFi.softAPIP());
  
  setupRoutes();
  
  server.begin();
  Serial.println("Web server started on port 80");
  return true;
}

void WebServerManager::handleClient() {
  dnsServer.processNextRequest();
  server.handleClient();
}

void WebServerManager::setDataCallback(DataCallback callback) {
  data_callback_ = callback;
}

void WebServerManager::broadcastData(const String& data) {
  lastData = data;
  // Data will be sent when clients request it via polling
}

void WebServerManager::setupRoutes() {
  // Serve main page
  server.on("/", [this]() {
    serveFile("/index.html", "text/html");
  });
  
  // API endpoint to get latest data (polling)
  server.on("/api/data", HTTP_GET, [this]() {
    server.send(200, "application/json", lastData);
  });
  
  // API endpoint to send test data
  server.on("/api/send", HTTP_POST, [this]() {
    if (server.hasArg("plain")) {
      String data = server.arg("plain");
      if (data_callback_) {
        data_callback_(data);
      }
      server.send(200, "application/json", "{\"status\":\"received\"}");
    } else {
      server.send(400, "application/json", "{\"error\":\"No data\"}");
    }
  });
  
  // Serve other common captive portal URLs
  server.on("/generate_204", [this]() {
    serveFile("/index.html", "text/html");
  });
  
  server.on("/hotspot-detect.html", [this]() {
    serveFile("/index.html", "text/html");
  });
  
  // Catch-all handler
  server.onNotFound([this]() {
    serveFile("/index.html", "text/html");
  });
}

void WebServerManager::serveFile(const String& path, const String& contentType) {
  File file = LittleFS.open(path, "r");
  if (!file) {
    server.send(404, "text/plain", "File not found: " + path);
    return;
  }
  
  server.streamFile(file, contentType);
  file.close();
}