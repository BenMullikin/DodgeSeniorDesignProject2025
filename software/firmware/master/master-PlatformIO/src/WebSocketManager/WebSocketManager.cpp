#include "WebSocketManager.h"

WSConnectCallback WebSocketManager::connect_callback_ = nullptr;
WSDisconnectCallback WebSocketManager::disconnect_callback_ = nullptr;
WSMessageCallback WebSocketManager::message_callback_ = nullptr;

WebSocketManager::WebSocketManager(uint16_t port) : web_socket_(port) {
}

void WebSocketManager::begin() {
  web_socket_.begin();
  web_socket_.onEvent(WebSocketManager::onWebSocketEvent);
  Serial.println("WebSocket server started");
}

void WebSocketManager::loop() {
  web_socket_.loop();
}

void WebSocketManager::broadcast(const String& message) {
  // Create a non-const copy for the WebSockets library
  String messageCopy = message;
  web_socket_.broadcastTXT(messageCopy);
}

void WebSocketManager::sendToClient(uint8_t num, const String& message) {
  // Create a non-const copy for the WebSockets library
  String messageCopy = message;
  web_socket_.sendTXT(num, messageCopy);
}

void WebSocketManager::setConnectCallback(WSConnectCallback callback) {
  connect_callback_ = callback;
}

void WebSocketManager::setDisconnectCallback(WSDisconnectCallback callback) {
  disconnect_callback_ = callback;
}

void WebSocketManager::setMessageCallback(WSMessageCallback callback) {
  message_callback_ = callback;
}

int WebSocketManager::getConnectedClients() {
  return web_socket_.connectedClients();
}

void WebSocketManager::onWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      if (connect_callback_ != nullptr) {
        connect_callback_(num);
      }
      break;
      
    case WStype_DISCONNECTED:
      if (disconnect_callback_ != nullptr) {
        disconnect_callback_(num);
      }
      break;
      
    case WStype_TEXT:
      if (message_callback_ != nullptr) {
        message_callback_(num, payload, length);
      }
      break;
      
    default:
      break;
  }
}