# Frame `0x08` — State of Charge (SoC)

## Observed Role

Register `0x08` carries the battery state of charge as a percentage. Known frames are 9 bytes long:

```text
86 12 08 01 SS CC 00 E0 F0
```

| Offset | Size | Description | Status |
|---:|---:|---|---|
| 0–1 | 2 bytes | Header / command (`86 12`) | Confirmed |
| 2 | 1 byte | Register `0x08` | Confirmed |
| 3 | 1 byte | Length (`0x01`) | Confirmed |
| 4 | 1 byte | SoC (`SS`) | Likely, in % |
| 5 | 1 byte | Checksum or related field (`CC`) | Likely |
| 6 | 1 byte | Fixed value `00` | Observed |
| 7–8 | 2 bytes | Frame terminator (`E0 F0`) | Confirmed |

## Captures and Checksum Hypothesis

```text
86 12 08 01 00 1B 00 E0 F0
86 12 08 01 20 3B 00 E0 F0
```

In both examples, byte 5 increases by the same amount as the SoC value. The simplest hypothesis is:

```text
CC = SS + 0x1B   (modulo 256)
```

It explains `00 → 1B` and `20 → 3B`, but requires more values before it can be considered the protocol checksum.

## Emulator Behavior

The emulator sends the requested percentage in byte 4 and clamps it to 100. Byte 5 is currently fixed at `44`; it should be replaced with the formula above once validated on the motorcycle.

