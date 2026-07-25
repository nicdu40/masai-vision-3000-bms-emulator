# Frame `0x07` — BMS Current

## Observed Role

Frame `0x07` carries a current measurement. BMS responses are 10 bytes long:

```text
86 12 07 02 LL HH XX YY E0 F0
```

| Offset | Size | Description | Status |
|---:|---:|---|---|
| 0–1 | 2 bytes | Header / command (`86 12`) | Confirmed |
| 2 | 1 byte | Register `0x07` | Confirmed |
| 3 | 1 byte | Length (`0x02`) | Confirmed |
| 4–5 | 2 bytes | Signed little-endian current value | Likely |
| 6–7 | 2 bytes | Current-related fields | Unknown |
| 8–9 | 2 bytes | Frame terminator (`E0 F0`) | Confirmed |

## Current Encoding

Bytes 4–5 are likely a little-endian `int16_t` value with a resolution of 0.1 A:

```text
86 12 07 02 F6 FF 10 02 E0 F0
            └─────┘
            0xFFF6 = -10 → -1.0 A
```

Captured frames contain both negative and positive values, consistent with a charge/discharge convention. The exact direction shown by the dashboard still needs to be confirmed on the motorcycle.

## Available Captures

```text
86 12 07 02 18 FC 66 01 E0 F0
86 12 07 02 F6 FF 10 02 E0 F0
86 12 07 02 00 00 1B 00 E0 F0
86 12 07 02 C4 FF DE 01 E0 F0
86 12 07 02 4A 01 66 00 E0 F0
86 12 07 02 FC F4 0B 02 E0 F0
```

## Current Emulator Behavior

The emulator applies a display scale factor of 2, then encodes the result in tenths of an ampere:

```text
rawCurrent = ±(desiredCurrentAmps × 2 × 10)
```

- Discharging: negative value
- Charging: positive value

For example, a configured current of 20 A is encoded as ±400 (`0xFE70` or `0x0190`), which is intended to display as 40 A on the dashboard.

Bytes 6–7 are currently compatibility values selected by the emulator:

| Condition | Bytes 6–7 |
|---|---|
| Current magnitude ≤ 1 A | `10 02` |
| Current magnitude > 1 A | `66 01` |

They are not a documented checksum algorithm and should be validated with additional captures.
