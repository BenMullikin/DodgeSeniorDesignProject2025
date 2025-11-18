#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include <WebServer.h>
#include <DNSServer.h>
#include <LittleFS.h>

class WebServerManager {
public:
  WebServerManager();
  
  bool begin();
  void handleClient();
  
private:
  WebServer server;
  DNSServer dnsServer;
  
  void setupRoutes();
  void serveFile(const String& path, const String& contentType);
};

#endif