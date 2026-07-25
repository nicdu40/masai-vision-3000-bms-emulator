# Frame `0x0B` — Unknown

## Observed Role

The exact purpose of register `0x0B` remains unknown.

```text
86 12 0B 02 3C 08 63 00 E0 F0
```

| Offset | Size | Description | Status |
|---:|---:|---|---|
| 0–1 | 2 bytes | Header / command (`86 12`) | Confirmed |
| 2 | 1 byte | Register `0x0B` | Confirmed |
| 3 | 1 byte | Length (`0x02`) | Confirmed |
| 4–5 | 2 bytes | Little-endian value | Format confirmed; meaning and unit unknown |
| 6 | 1 byte | Field derived from byte 4 | Likely |
| 7 | 1 byte | Observed fixed value `00` | Observed |
| 8–9 | 2 bytes | Frame terminator (`E0 F0`) | Confirmed |

## Captured Exchange

```text
Motorcycle → BMS: 86 12 0B 01 00 1E 00 E0 F0
BMS → Motorcycle: 86 12 0B 02 3C 08 63 00 E0 F0
BMS → Motorcycle: 86 12 0B 02 20 08 47 00 E0 F0
BMS → Motorcycle: 86 12 0B 02 1C 08 43 00 E0 F0
```

The raw values are `0x083C` (2108), `0x0820` (2080), and `0x081C` (2076). Byte 6 is exactly byte 4 plus `0x27`:

```text
3C + 27 = 63
20 + 27 = 47
1C + 27 = 43
```

This relationship holds for all three captures. Byte 5 is constant at `08`. The unit of the 16-bit value cannot be determined without synchronized voltage or cell measurements.

## Emulator Behavior

The emulator currently uses one of the observed values:

```text
86 12 0B 02 20 08 47 00 E0 F0
```

This value is fixed in the code; mapping it to a real measurement remains to be implemented.

