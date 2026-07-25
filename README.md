# BMS Emulator - Masai Vision 3000

ESP32-based BMS (Battery Management System) emulator for Masai Vision 3000 motorcycles.

## Overview

This project emulates a BMS communication protocol over RS485, allowing a motorcycle dashboard to communicate with a simulated battery pack. It uses an ESP32 with HardwareSerial (UART2) instead of SoftwareSerial for reliable communication.

## Features

- **RS485 BMS Emulation** — Responds to register queries (temperature, current, SoC, charge status)
- **HardwareSerial UART2** — Uses GPIO5 (RX) / GPIO4 (TX) remapped on UART2
- **WiFi AP + Telnet** — Optional debug logging via Telnet (port 23) when `DEBUG_MOTORCYCLE_FRAMES = true`
- **USB Interactive** — Serial commands to adjust simulator parameters
- **Clean Architecture** — TelnetLogger library in `lib/` for separation of concerns

## Hardware

| Component | Pin | Function |
|-----------|-----|----------|
| MAX485 RO | GPIO5 | RS485 RX (UART2) |
| MAX485 DI | GPIO4 | RS485 TX (UART2) |
| MAX485 DE | GPIO14 | Driver Enable |
| MAX485 RE | GPIO12 | Receiver Enable |

## Getting Started

### Prerequisites

- [PlatformIO](https://platformio.org/) (VS Code extension or CLI)
- ESP32 development board (38-pin)

### Build & Flash

```bash
# Build
pio run

# Flash (close Serial Monitor first)
pio run -t upload

# Monitor
pio run -t monitor
```

### USB Commands

| Command | Description |
|---------|-------------|
| `c1` / `c0` | Enable/disable charging mode |
| `t25` | Set temperature to 25°C |
| `a150` | Set current to 150A |
| `s80` | Set SoC to 80% |

### Telnet Debug (optional)

Set `DEBUG_MOTORCYCLE_FRAMES = true` in `src/main.cpp`, then:

1. Connect to WiFi: `MasaiBMS_Logger` / `password123`
2. Telnet to `192.168.4.1` port `23`
3. RS485 frames appear in hex format

## Project Structure

```
masai_esp32/
├── platformio.ini          # PlatformIO configuration
├── src/
│   └── main.cpp            # Main BMS emulator code
├── lib/
│   └── TelnetLogger/       # WiFi AP + Telnet server library
│       ├── TelnetLogger.h
│       └── TelnetLogger.cpp
├── docs/                   # Documentation
│   ├── architecture.md
│   ├── protocol.md
│   └── troubleshooting.md
├── test/                   # Unit tests
└── README.md
```

## License

MIT
