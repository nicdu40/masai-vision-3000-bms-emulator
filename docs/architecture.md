# Architecture

## Overview

The BMS Emulator is structured around a clean separation of concerns:

```
┌─────────────────────────────────────────────────┐
│                    main.cpp                      │
│  ┌──────────────┐  ┌────────────────────────┐  │
│  │ RS485 State  │  │ USB Interactive        │  │
│  │ Machine      │  │ Control                │  │
│  └──────┬───────┘  └────────────────────────┘  │
│         │                                       │
│         ▼                                       │
│  ┌──────────────┐  ┌────────────────────────┐  │
│  │ Query        │  │ TelnetLogger (lib/)    │  │
│  │ Processing   │  │ WiFi AP + Telnet       │  │
│  └──────────────┘  └────────────────────────┘  │
└─────────────────────────────────────────────────┘
```

## Components

### 1. RS485 Communication (UART2)

- **HardwareSerial** on UART2 (not SoftwareSerial)
- Pins remapped: GPIO5 (RX), GPIO4 (TX)
- DE/RE pins: GPIO14 (DE), GPIO12 (RE)
- Baud rate: 9600

### 2. State Machine

```
WAIT_START ──(0x86)──> READING ──(0xE0 0xF0)──> processQuery() ──> WAIT_START
                         │
                         └──(timeout 20ms)──> WAIT_START
```

### 3. TelnetLogger Library

- **WiFi AP** mode: `MasaiBMS_Logger` / `password123`
- **TCP Server** on port 23 (Telnet)
- Accepts 1 client at a time
- Broadcasts RS485 frames in hex format
- Only active when `DEBUG_MOTORCYCLE_FRAMES = true`

### 4. USB Interactive

- Serial commands via USB (115200 baud)
- Commands: `c` (charging), `t` (temp), `a` (amps), `s` (SoC)
- No String class usage — uses char buffers

## Memory Usage

| Mode | RAM | Flash |
|------|-----|-------|
| Debug OFF | 7.7% (25KB) | 28.3% (370KB) |
| Debug ON | 13.7% (45KB) | 57.3% (751KB) |
