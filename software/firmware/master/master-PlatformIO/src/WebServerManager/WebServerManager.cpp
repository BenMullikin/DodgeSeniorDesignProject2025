#include "WebServerManager.h"

WebServerManager::WebServerManager() : server(80) {
}

bool WebServerManager::begin() {
  Serial.println("Initializing Web Server...");
  
  // Initialize LittleFS
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

void WebServerManager::setupRoutes() {
  // Serve main page
  server.on("/", [this]() {
    serveFile("/index.html", "text/html");
  });
  
  // Serve other common captive portal URLs
  server.on("/generate_204", [this]() {
    serveFile("/index.html", "text/html");
  });
  
  server.on("/hotspot-detect.html", [this]() {
    serveFile("/index.html", "text/html");
  });
  
  // API endpoint for testing
  server.on("/api/hello", HTTP_GET, [this]() {
    server.send(200, "application/json", "{\"message\":\"Hello from ESP32!\"}");
  });
  
  // Catch-all handler - serve files from LittleFS or redirect to index.html
  server.onNotFound([this]() {
    String path = server.uri();
    
    // Try to serve the requested file
    if (LittleFS.exists(path)) {
      if (path.endsWith(".css")) {
        serveFile(path, "text/css");
      } else if (path.endsWith(".js")) {
        serveFile(path, "application/javascript");
      } else if (path.endsWith(".png")) {
        serveFile(path, "image/png");
      } else if (path.endsWith(".jpg")) {
        serveFile(path, "image/jpeg");
      } else {
        serveFile(path, "text/plain");
      }
    } else {
      // File not found, serve index.html (captive portal behavior)
      Serial.println("File not found, serving index.html: " + path);
      serveFile("/index.html", "text/html");
    }
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