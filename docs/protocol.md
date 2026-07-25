# RS485 Protocol

## Frame Format

```
[0x86] [reg] [len] [data...] [0xE0] [0xF0]
```

- **0x86**: Start byte (sync)
- **reg**: Register address
- **len**: Data length
- **data**: Register data
- **0xE0 0xF0**: End marker

## Register Map

| Register | Description | Response |
|----------|-------------|----------|
| 0x05 | Temperature | 5 bytes (temp × 5) |
| 0x07 | Current | 4 bytes (raw current) |
| 0x08 | State of Charge | 1 byte (0-100%) |
| 0x0A | Test register A | Counter 0-16 |
| 0x0B | Test register B | Fixed pattern |
| 0x0C | Charge/Discharge status | 2 bytes |
| 0x25 | No response | — |

## Checksum

```
checksum = (sum of bytes 0..N-1) - 0x86
```

## Example Query

```
Motorcycle → BMS:  86 12 05 05 E0 F0  (read temperature)
BMS → Motorcycle:  86 12 05 05 19 19 19 19 19 [CS] 00 E0 F0
```

Where `[CS]` is the calculated checksum.
