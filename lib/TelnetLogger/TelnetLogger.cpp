#include "TelnetLogger.h"

TelnetLogger::TelnetLogger(const char* ssid, const char* password, uint16_t port)
    : _ssid(ssid), _password(password), _port(port), _server(port) {}

void TelnetLogger::begin() {
    WiFi.softAP(_ssid, _password);
    _server.begin();
    _server.setNoDelay(true);

    Serial.print("[TELNET] AP active: ");
    Serial.println(_ssid);
    Serial.print("[TELNET] Server ready on port ");
    Serial.println(_port);
}

void TelnetLogger::handleClient() {
    if (_server.hasClient()) {
        if (_client.connected()) {
            // Already have a client — reject the new one
            WiFiClient reject = _server.available();
            reject.stop();
        } else {
            _client.stop();
            _client = _server.available();
            if (_client.connected()) {
                _client.println("=== CONNECTE AU BUS RS485 MASAI ===");
            }
        }
    }
}

void TelnetLogger::broadcast(const uint8_t* data, size_t len) {
    if (_client.connected()) {
        _client.write(data, len);
    }
}

void TelnetLogger::broadcast(const char* str) {
    if (_client.connected()) {
        _client.print(str);
    }
}

void TelnetLogger::broadcast(const String& str) {
    if (_client.connected()) {
        _client.print(str);
    }
}

bool TelnetLogger::isConnected() {
    return _client.connected();
}
