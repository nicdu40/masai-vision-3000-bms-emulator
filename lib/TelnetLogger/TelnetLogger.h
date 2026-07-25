#pragma once

#include <WiFi.h>
#include <Arduino.h>

class TelnetLogger {
public:
    TelnetLogger(const char* ssid, const char* password, uint16_t port = 23);

    void begin();
    void handleClient();
    void broadcast(const uint8_t* data, size_t len);
    void broadcast(const char* str);
    void broadcast(const String& str);
    bool isConnected();

private:
    const char* _ssid;
    const char* _password;
    uint16_t _port;
    WiFiServer _server;
    WiFiClient _client;
};
