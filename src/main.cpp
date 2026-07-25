#include <Arduino.h>
#include "TelnetLogger.h"

// -------------------- WiFi / Telnet config --------------------
// Telnet logging is only active when DEBUG_MOTORCYCLE_FRAMES is true
const char* WIFI_SSID     = "MasaiBMS_Logger";
const char* WIFI_PASSWORD = "password123";
TelnetLogger telnetLogger(WIFI_SSID, WIFI_PASSWORD, 23);

// -------------------- RS485 pins (MAX485) --------------------
// UART2 remapped to original GPIO pins (GPIO5/GPIO4)
// ESP32 HardwareSerial allows any GPIO for UART RX/TX
#define RS485_RX_PIN    5    // GPIO5 (D1) -> RO
#define RS485_TX_PIN    4    // GPIO4 (D2) -> DI
#define DE_PIN          14   // GPIO14 (D5) -> DE
#define RE_PIN          12   // GPIO12 (D6) -> RE

// -------------------- Serial links --------------------
// Use UART2 (HardwareSerial) instead of SoftwareSerial
// UART0 (Serial) is reserved for USB debugging
HardwareSerial rs485Serial(2);

// -------------------- Debug config --------------------
static const bool DEBUG_MOTORCYCLE_FRAMES = false;

// -------------------- Frame settings --------------------
static const size_t MAX_FRAME_LEN = 64;
static uint8_t rxBuffer[MAX_FRAME_LEN];
static size_t rxIndex = 0;
static const uint32_t FRAME_TIMEOUT_MS = 20;
static uint32_t lastByteAt = 0;

// -------------------- Simulator params --------------------
static bool isCharging = false;
static uint8_t desiredTemperatureC = 20; // °C
static float desiredCurrentAmps = 50.0f; // A
static uint8_t desiredSocPercent = 50;   // %

// -------------------- State machine --------------------
enum State {
  WAIT_START,
  READING
};
static State currentState = WAIT_START;

// -------------------- Dynamic values for test registers --------------------
static uint8_t testRegister0AValue = 0;
static uint8_t testRegister0BValue = 0;

// -------------------- Forward declarations --------------------
static void rs485Listen();
static void rs485Transmit();
static void sendBmsResponse(const uint8_t *response, size_t len);
static void sendSocResponse(uint8_t desiredSoc);
static void sendTempResponse(uint8_t desiredTemp);
static void sendCurrentResponse(float currentAmps, bool isCharging);
static void debugPrintMotorcycleFrame(const uint8_t *frame, size_t len);
static void processQuery(uint8_t *frame, size_t len);
static void handleUsbInteractive();

// -------------------- RS485 helpers --------------------
static inline void rs485Listen() {
  digitalWrite(DE_PIN, LOW);
  digitalWrite(RE_PIN, LOW);
  delayMicroseconds(150);
}

static inline void rs485Transmit() {
  digitalWrite(DE_PIN, HIGH);
  digitalWrite(RE_PIN, HIGH);
  delayMicroseconds(150);
}

// -------------------- USB interactive control (without String) --------------------
static void handleUsbInteractive() {
  static char line[32];
  static size_t idx = 0;

  while (Serial.available() > 0) {
    char ch = (char)Serial.read();

    if (ch == '\r') continue;

    if (ch == '\n') {
      line[idx] = '\0';
      idx = 0;

      if (line[0] == '\0') return;

      char cmd = line[0];
      long val = 0;

      if (strlen(line) > 1) {
        val = atol(line + 1);
      }

      switch (cmd) {
        case 'c':
        case 'C':
          isCharging = (val != 0);
          Serial.print("[INTERACTIVE] Charging mode: ");
          Serial.println(isCharging ? "ON" : "OFF");
          break;

        case 't':
        case 'T':
          if (val < 0) val = 0;
          if (val > 99) val = 99;
          desiredTemperatureC = (uint8_t)val;
          Serial.print("[INTERACTIVE] Temperature set to: ");
          Serial.print(desiredTemperatureC);
          Serial.println(" °C");
          break;

        case 'a':
        case 'A':
          if (val < 0) val = 0;
          if (val > 700) val = 700;
          desiredCurrentAmps = (float)val;
          Serial.print("[INTERACTIVE] Current set to: ");
          Serial.print(desiredCurrentAmps);
          Serial.println(" A");
          break;

        case 's':
        case 'S':
          if (val < 0) val = 0;
          if (val > 100) val = 100;
          desiredSocPercent = (uint8_t)val;
          Serial.print("[INTERACTIVE] SoC set to: ");
          Serial.print(desiredSocPercent);
          Serial.println(" %");
          break;

        default:
          Serial.print("[INTERACTIVE] Unknown command: ");
          Serial.println(cmd);
          break;
      }

      return;
    } else {
      if (idx < sizeof(line) - 1) {
        line[idx++] = ch;
      }
    }
  }
}

// -------------------- Response functions --------------------
static void sendSocResponse(uint8_t desiredSoc) {
  if (desiredSoc > 100) desiredSoc = 100;

  uint8_t resp[] = {
    0x86, 0x12, 0x08, 0x01,
    desiredSoc,
    0x44, 0x00,
    0xE0, 0xF0
  };
  sendBmsResponse(resp, sizeof(resp));
}

// Generic checksum calculation with a 0x86 offset.
uint8_t calculateChecksum(uint8_t *frame, uint8_t length) {
    uint16_t sum = 0;
    for (uint8_t i = 0; i < length; i++) {
        sum += frame[i];
    }
    return (uint8_t)(sum - 0x86);
}

static void sendTempResponse(uint8_t desiredTemp) {
  // Prepare the frame with a temporary 0x00 checksum value.
  uint8_t resp[] = {
    0x86, 0x12, 0x05, 0x05,
    desiredTemp, desiredTemp, desiredTemp, desiredTemp, desiredTemp,
    0x00, 0x00, // [9] = temporary checksum, [10] = state (0x00)
    0xE0, 0xF0
  };

  // Calculate the checksum over every byte before the checksum field (indices 0 to 8).
  resp[9] = calculateChecksum(resp, 9);

  // Send the complete frame.
  sendBmsResponse(resp, sizeof(resp));
}

static void sendCurrentResponse(float currentAmps, bool isCharging) {
  // Multiply by 2 so that 150 A sent is displayed as 300 A (gauge scaling adjustment).
  float adjustedAmps = currentAmps * 2.0f;

  int16_t rawCurrent = (int16_t)(- (adjustedAmps * 10.0f));

  if (isCharging) {
    rawCurrent = (int16_t)(adjustedAmps * 10.0f);
  }

  uint8_t byte4 = (uint8_t)(rawCurrent & 0xFF);
  uint8_t byte5 = (uint8_t)((rawCurrent >> 8) & 0xFF);

  uint8_t byte6 = (abs(currentAmps) > 1.0f) ? 0x66 : 0x10;
  uint8_t byte7 = (abs(currentAmps) > 1.0f) ? 0x01 : 0x02;

  uint8_t resp[] = {
    0x86, 0x12, 0x07, 0x02,
    byte4,
    byte5,
    byte6,
    byte7,
    0xE0, 0xF0
  };

  sendBmsResponse(resp, sizeof(resp));
}

// -------------------- Debug --------------------
static void debugPrintMotorcycleFrame(const uint8_t *frame, size_t len) {
  if (!DEBUG_MOTORCYCLE_FRAMES) return;

  Serial.print("[MOTORCYCLE->BMS] Len=");
  Serial.print(len);
  Serial.print(" RAW: ");
  for (size_t i = 0; i < len; i++) {
    if (frame[i] < 16) Serial.print("0");
    Serial.print(frame[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
}

// -------------------- Query processing --------------------
static void processQuery(uint8_t *frame, size_t len) {
  debugPrintMotorcycleFrame(frame, len);

  if (len < 4) return;
  if (frame[0] != 0x86) return;

  if (frame[1] == 0x12) {
    uint8_t reg = frame[2];

    if (reg == 0x25) {
      if (DEBUG_MOTORCYCLE_FRAMES) Serial.println("[MOTORCYCLE->BMS] reg=0x25 (no response expected)");
      return;
    }

    if (DEBUG_MOTORCYCLE_FRAMES) {
      Serial.print("[MOTORCYCLE->BMS] reg=0x");
      if (reg < 16) Serial.print("0");
      Serial.print(reg, HEX);
      Serial.print(" => ");
      switch (reg) {
        case 0x05: Serial.println("expected response: temperature"); break;
        case 0x07: Serial.println("expected response: current"); break;
        case 0x08: Serial.println("expected response: state of charge"); break;
        case 0x0A: Serial.println("expected response: unknown register 0x0A"); break;
        case 0x0B: Serial.println("expected response: unknown register 0x0B"); break;
        case 0x0C: Serial.println("expected response: charge/discharge status"); break;
        default:   Serial.println("unknown response / fallback");
      }
    }

    switch (reg) {
      case 0x05: {
        sendTempResponse(desiredTemperatureC);
        break;
      }

      case 0x07: {
        sendCurrentResponse(desiredCurrentAmps, isCharging);
        break;
      }

      case 0x08: {
        sendSocResponse(desiredSocPercent);
        break;
      }

      case 0x0A: {
        testRegister0AValue++;
        if (testRegister0AValue > 16) testRegister0AValue = 0;
        uint8_t responseValue = testRegister0AValue;

        uint8_t resp[] = {
          0x86, 0x12, 0x0A, 0x02,
          responseValue,
          0x06, 0x80, 0x00,
          0xE0, 0xF0
        };

        sendBmsResponse(resp, sizeof(resp));
        break;
      }

      case 0x0B: {
        testRegister0BValue++;
        uint8_t resp[] = {
          0x86, 0x12, 0x0B, 0x02,
          0x20, 0x08, 0x47, 0x00,
          0xE0, 0xF0
        };

        sendBmsResponse(resp, sizeof(resp));
        break;
      }

      case 0x0C: {
        if (isCharging) {
          uint8_t resp[] = { 0x86, 0x12, 0x0C, 0x02, 0x02, 0x00, 0x22, 0x00, 0xE0, 0xF0 };
          sendBmsResponse(resp, sizeof(resp));
        } else {
          uint8_t resp[] = { 0x86, 0x12, 0x0C, 0x02, 0x00, 0x00, 0x20, 0x00, 0xE0, 0xF0 };
          sendBmsResponse(resp, sizeof(resp));
        }
        break;
      }

      default:
        break;
    }
    return;
  }
}

// -------------------- RS485 send --------------------
static void sendBmsResponse(const uint8_t *response, size_t len) {
  if (!response || len == 0) return;

  rs485Transmit();
  rs485Serial.write(response, (uint8_t)len);
  rs485Serial.flush();
  rs485Listen();
}

// -------------------- Setup --------------------
void setup() {
  pinMode(DE_PIN, OUTPUT);
  pinMode(RE_PIN, OUTPUT);
  rs485Listen();
  delay(2);
  Serial.begin(115200);
  rs485Serial.begin(9600, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);

  // Start WiFi AP + Telnet server only in debug mode
  if (DEBUG_MOTORCYCLE_FRAMES) {
    telnetLogger.begin();
  }

  Serial.println("\n=========================================");
  Serial.println("--- BMS EMULATOR - MASAI VISION 3000 ---");
  Serial.println("=========================================\n");
}

// -------------------- Loop --------------------
void loop() {
  rs485Listen();
  handleUsbInteractive();

  // Handle Telnet client only in debug mode
  if (DEBUG_MOTORCYCLE_FRAMES) {
    telnetLogger.handleClient();
  }

  while (rs485Serial.available() > 0) {
    uint8_t b = (uint8_t)rs485Serial.read();
    lastByteAt = millis();

    switch (currentState) {
      case WAIT_START:
        if (b == 0x86) {
          rxIndex = 0;
          rxBuffer[rxIndex++] = b;
          currentState = READING;
        }
        break;

      case READING:
        if (rxIndex < MAX_FRAME_LEN) {
          rxBuffer[rxIndex++] = b;

          if (rxIndex >= 2 &&
              rxBuffer[rxIndex - 2] == 0xE0 &&
              rxBuffer[rxIndex - 1] == 0xF0) {

            // Broadcast raw frame to Telnet clients (debug only)
            if (DEBUG_MOTORCYCLE_FRAMES && telnetLogger.isConnected()) {
              char hexBuf[8];
              for (size_t i = 0; i < rxIndex; i++) {
                sprintf(hexBuf, "%02X ", rxBuffer[i]);
                telnetLogger.broadcast(hexBuf);
              }
              telnetLogger.broadcast("\r\n");
            }

            processQuery(rxBuffer, rxIndex);
            currentState = WAIT_START;
            rxIndex = 0;
          }
        } else {
          currentState = WAIT_START;
          rxIndex = 0;
        }
        break;
    }
  }

  if (currentState == READING) {
    if (millis() - lastByteAt > FRAME_TIMEOUT_MS) {
      currentState = WAIT_START;
      rxIndex = 0;
    }
  }
}
