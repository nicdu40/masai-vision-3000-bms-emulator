# Troubleshooting

## Build Issues

### "SoftwareSerial not found"
This project uses `HardwareSerial` (UART2) on ESP32. Do not include `SoftwareSerial.h`.

### "WiFi.h not found"
Ensure you're building for ESP32, not ESP8266. The `platformio.ini` should have:
```ini
platform = espressif32
board = esp32dev
```

## Runtime Issues

### No RS485 communication
1. Check wiring: GPIO5 → RO, GPIO4 → DI, GPIO14 → DE, GPIO12 → RE
2. Verify MAX485 power (VCC/GND)
3. Check baud rate (9600)

### Telnet not working
1. Set `DEBUG_MOTORCYCLE_FRAMES = true` in `src/main.cpp`
2. Connect to WiFi: `MasaiBMS_Logger` / `password123`
3. Telnet to `192.168.4.1` port `23`
4. Only 1 client can connect at a time

### Upload fails (port busy)
Close the Serial Monitor before uploading.

### GPIO16 issues
Do NOT use GPIO16 for UART — it's an RTC pin with pull-ups that interfere with UART RX.
