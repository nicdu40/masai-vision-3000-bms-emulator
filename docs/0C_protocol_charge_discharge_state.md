# Frame `0x0C` — Charge/Discharge State

## Observed Role

Frame `0x0C` indicates the battery operating state: idle, discharging, or charging.

```text
86 12 0C 02 EE 00 CC 00 E0 F0
```

| Offset | Size | Description | Status |
|---:|---:|---|---|
| 0–1 | 2 bytes | Header / command (`86 12`) | Confirmed |
| 2 | 1 byte | Register `0x0C` | Confirmed |
| 3 | 1 byte | Length (`0x02`) | Confirmed |
| 4 | 1 byte | State (`EE`) | Confirmed by captures |
| 5 | 1 byte | Reserved, `00` | Observed |
| 6 | 1 byte | Derived field / checksum (`CC`) | Likely |
| 7 | 1 byte | Reserved, `00` | Observed |
| 8–9 | 2 bytes | Frame terminator (`E0 F0`) | Confirmed |

## Observed Values

| State | `EE` | BMS → Motorcycle Frame |
|---|---:|---|
| Idle | `00` | `86 12 0C 02 00 00 20 00 E0 F0` |
| Discharging | `01` | `86 12 0C 02 01 00 21 00 E0 F0` |
| Charging | `02` | `86 12 0C 02 02 00 22 00 E0 F0` |

The captures are consistent with `CC = EE + 0x20`. This is verified for the three known states, but its checksum status still needs confirmation.

## Observed Request

```text
86 12 0C 01 00 1F 00 E0 F0
```

## Emulator Behavior

The emulator sends `EE = 02` when charging mode is enabled and `EE = 00` otherwise. It does not yet generate the distinct discharging state (`EE = 01`).
