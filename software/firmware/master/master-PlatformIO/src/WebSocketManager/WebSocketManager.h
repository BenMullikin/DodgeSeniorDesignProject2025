#ifndef WEBSOCKET_MANAGER_H
#define WEBSOCKET_MANAGER_H

#include <WebSocketsServer.h>
#include <functional>

typedef std::function<void(uint8_t num)> WSConnectCallback;
typedef std::function<void(uint8_t num)> WSDisconnectCallback;
typedef std::function<void(uint8_t num, uint8_t* payload, size_t length)> WSMessageCallback;

class WebSocketManager {
public:
  WebSocketManager(uint16_t port = 81);
  
  void begin();
  void loop();
  void broadcast(const String& message);
  void sendToClient(uint8_t num, const String& message);
  
  void setConnectCallback(WSConnectCallback callback);
  void setDisconnectCallback(WSDisconnectCallback callback);
  void setMessageCallback(WSMessageCallback callback);
  
  int getConnectedClients();

private:
  static void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
  
  WebSocketsServer web_socket_;
  static WSConnectCallback connect_callback_;
  static WSDisconnectCallback disconnect_callback_;
  static WSMessageCallback message_callback_;
};

#endif