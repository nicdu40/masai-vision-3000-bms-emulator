# Frame `0x05` — BMS Temperatures

## Overview

Frame `0x05` carries five temperature readings in a 13-byte BMS response:

```text
86 12 05 05 T1 T2 T3 T4 T5 CS1 CS2 E0 F0
```

| Offset | Size | Description | Status |
|---:|---:|---|---|
| 0 | 1 byte | Header | `0x86` |
| 1 | 1 byte | Command | `0x12` |
| 2 | 1 byte | Register | `0x05` |
| 3 | 1 byte | Temperature-sensor count | `0x05` |
| 4–8 | 5 bytes | Temperatures T1–T5 | Direct °C values |
| 9 | 1 byte | Checksum low byte (`CS1`) | Confirmed by captures |
| 10 | 1 byte | Checksum high byte (`CS2`) | Confirmed by captures |
| 11–12 | 2 bytes | Frame terminator | `E0 F0` |

## Temperature Encoding

Each temperature byte is sent directly in degrees Celsius. No additional encoding has been observed.

```text
0x1B = 27 °C     0x1C = 28 °C     0x1D = 29 °C
0x2E = 46 °C     0x31 = 49 °C     0x32 = 50 °C     0x33 = 51 °C
```

For example:

```text
86 12 05 05 31 2E 32 33 32 12 01 E0 F0
```

represents 49 °C, 46 °C, 50 °C, 51 °C, and 50 °C.

## Captures

```text
86 12 05 05 1B 1B 1B 1D 1D A7 00 E0 F0
86 12 05 05 1C 1C 1C 1D 1D AA 00 E0 F0
86 12 05 05 1C 1B 1C 1D 1D A9 00 E0 F0

86 12 05 05 31 2E 32 33 32 12 01 E0 F0
86 12 05 05 31 2F 32 33 32 13 01 E0 F0
86 12 05 05 31 2F 32 33 33 14 01 E0 F0
```

## Checksum

The two checksum bytes match the unsigned 16-bit sum of bytes 1 through 8 (the `0x86` header is excluded), encoded little-endian:

```text
checksum = frame[1] + frame[2] + ... + frame[8]
CS1 = checksum & 0xFF
CS2 = checksum >> 8
```

For example, the first capture has a sum of `0x00A7`, giving `A7 00`. The first high-temperature capture has a sum of `0x0112`, giving `12 01`.

## Emulator Behavior

The emulator sends the selected temperature to all five sensor fields. It calculates `CS1` from bytes 1–8; `CS2` should also be populated with the high byte when the sum exceeds `0xFF`.

