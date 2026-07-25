# Frame `86 A2 70` — Contrôleur Info (Kickstand)

## Observed Role

Frame `86 A2 70` is sent by the motorcycle controller (not the BMS). It carries controller status information, including the kickstand state.

```text
86 A2 70 04 XX 00 1C 1A 00 00 01 YY E0 F0
```

| Offset | Size | Description | Status |
|---:|---:|---|---|
| 0–1 | 2 bytes | Header | `86 A2` |
| 2 | 1 byte | Register | `0x70` |
| 3 | 1 byte | Length | `0x04` |
| 4 | 1 byte | Kickstand state (`XX`) | Confirmed |
| 5 | 1 byte | Reserved | `00` |
| 6–7 | 2 bytes | Unknown | `1C 1A` |
| 8–9 | 2 bytes | Unknown | `00 00` |
| 10 | 1 byte | Unknown | `01` |
| 11 | 1 byte | Checksum / derived field (`YY`) | Likely |
| 12–13 | 2 bytes | Frame terminator | `E0 F0` |

## Observed Values

| Kickstand | `XX` | `YY` | Frame |
|---|---|---|---|
| Down | `20` | `6C` | `86 A2 70 04 20 00 1C 1A 00 00 01 6C E0 F0` |
| Up | `21` | `6D` | `86 A2 70 04 21 00 1C 1A 00 00 01 6D E0 F0` |

The checksum field `YY` increases by 1 when `XX` goes from `20` to `21`, suggesting `YY = XX + 0x4C` or a simple additive relationship.

## Notes

- This frame is **not** a BMS response — it comes from the motorcycle's controller (ECU).
- It is observed on the RS485 bus even when no BMS is connected (`READ_FRAME_ONLY = true`).
- The kickstand state may be used by the BMS or dashboard for safety interlocks (e.g., prevent charging/discharging while the kickstand is down).

## Emulator Behavior

The emulator currently only **reads** this frame to track the kickstand state in a `kickstandState` variable (0=down, 1=up). It does not transmit or emulate this frame.
