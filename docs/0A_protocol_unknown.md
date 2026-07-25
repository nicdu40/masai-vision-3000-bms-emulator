# Frame `0x0A` — Unknown

## Observed Role

The purpose and unit of register `0x0A` are unknown.

```text
86 12 0A 02 DC 04 FE 00 E0 F0
```

| Offset | Size | Description | Status |
|---:|---:|---|---|
| 0–1 | 2 bytes | Header / command (`86 12`) | Confirmed |
| 2 | 1 byte | Register `0x0A` | Confirmed |
| 3 | 1 byte | Length (`0x02`) | Confirmed |
| 4–5 | 2 bytes | Little-endian value | Format confirmed; meaning and unit unknown |
| 6 | 1 byte | Value-related field | Observed |
| 7 | 1 byte | Fixed value `00` | Observed |
| 8–9 | 2 bytes | Frame terminator (`E0 F0`) | Confirmed |

## Captures

```text
86 12 0A 02 A0 02 C0 00 E0 F0  -> 0x02A0 = 672
86 12 0A 02 9C 02 BC 00 E0 F0  -> 0x029C = 668
86 12 0A 02 98 02 B8 00 E0 F0  -> 0x0298 = 664
86 12 0A 02 94 02 B4 00 E0 F0  -> 0x0294 = 660
86 12 0A 02 90 02 B0 00 E0 F0  -> 0x0290 = 656
```

Byte 6 follows byte 4 with an observed constant offset of `+0x20` (`A0 → C0`, `9C → BC`). It may be a partial checksum or a derived field; this is not established.

## Observed Request

```text
86 12 0A 01 00 1D 00 E0 F0
```

## Emulator Behavior

The emulator cycles byte 4 from 0 to 16 and uses `06 80 00` for the following bytes. No visible dashboard effect has been observed.

