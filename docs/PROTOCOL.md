# C14 protocol on the AERO 2 + NANO XS bus

This description comes from **observing the live bus** of one installation (AERO 2 controller, NANO XS panel) and from cross-checking other people's protocol write-ups (links at the bottom). It is not official documentation.

Legend: ✅ verified on this bus; 🔸 taken from descriptions of other models and matched observations; ❓ hypothesis, not verified.

## 1. Physical layer

- RS485, **9600 baud, 8N1**. Only lines **A** and **B** are connected (terminals "PANEL NANO XS, RS485 C14" on the controller board). Line **U** is bus power and must not be connected to the transceiver; the common ground G was not needed for receiving. ✅
- At idle on the bus: A ≈ 1.7 V, B ≈ 1.5 V relative to ground, with 0.5–2 V jumping between A and B. ✅
- All devices on the bus (controller, panel, iNext) are connected in parallel (A–A, B–B), twisted-pair cable up to 200 m. 🔸
- The 3.3 V transceiver (Waveshare 3485) has DE and RE tied to a single pin RSE. While RSE = 1 (transmit) the receiver is off, so **the board does not hear its own transmission**. ✅

## 2. Frame

All frames are exactly **30 bytes**. ✅

| Byte | Meaning |
|---:|---|
| 0 | recipient address + `0x80` (for example `E4` = 100, `E3` = 99, `F0` = 112, `A9` = 41) ✅ |
| 1 | command, an ASCII character (see below) ✅ |
| 2 | checksum ✅ |
| 3 | sender address ✅ |
| 4–28 | data; unused bytes are filled with `0x7E` (or zeros) ✅ |
| 29 | terminator `0x23` ✅ |

**Checksum:** the sum of all frame bytes except byte 2, modulo 128 (`sum & 0x7F`). It matched on every analysed frame (hundreds). ✅

**Commands (byte 1):**

| Code | Char | Meaning |
|---|---|---|
| `0x21` | `!` | status / telemetry ✅ |
| `0x44` | `D` | poll, broadcast and service frames ✅ |
| `0x52` / `0x72` | `R` / `r` | register read request / reply ✅ (verified) |
| `0x57` / `0x77` | `W` / `w` | register write / acknowledgement ✅ (verified) |
| `0x54` / `0x74` | `T` / `t` | telemetry read (not used in this installation) 🔸 |

## 3. Addresses on this bus

| Address | Who | Basis |
|---|---|---|
| `0x3E` (62) | NANO XS panel, bus **master** | it sends all frames; register 1 = 62 ✅ |
| `0x63` (99) | AERO 2 controller | the only one replying with `E4 21 … 63` frames ✅ |
| `0x70` (112) | iNext slot | our board "pretends" to be it for reads and writes ✅ |
| 100, 41, 1 | recipients of broadcast frames | `E4`, `A9`, `81` in byte 0 |

There is a single master on the bus. Without a master (the panel) the bus goes silent, so the panel must not be disconnected.

## 4. Exchange cycle (~4 seconds)

The panel sends frames in a cycle and the controller replies to one of them. Observed order:

| Frame | From | Content |
|---|---|---|
| `F0 44 … 3E` | panel | iNext poll (empty), followed by a **~0.9 s silence window** ✅ |
| `E3 44 … 3E` | panel | request to the controller (contains undeciphered settings) ✅ |
| `E4 21 … 63` | controller | reply: temperature, fans ✅ |
| `A9 44 … 3E`, `81 44 … 3E` | panel | empty service frames (heartbeat) ✅ |
| `E4 21 … 3E` | panel | status: speeds, on, day/night ✅ |

## 5. Panel status frame: `E4 21 xx 3E …`

| Byte | Meaning | Status |
|---:|---|---|
| 16 | active speed in percent (`step × 10`: `0A`…`64`) | ✅ |
| 17 | **day speed** (`step × 10`) = register 8 × 10 | ✅ |
| 18 | **night speed** (`step × 10`) = register 9 × 10 | ✅ |
| 19, bit `0x01` | ventilation on (`1`) / off (`0`) | ✅ |
| 19, bit `0x02` | "day" mode (`1`) / "night" (`0`) | ✅ |
| 19, bit `0x40` | always 1 | ❓ |
| 12–15 | `0F 50 0F 50` = "0 °C": the panel has no temperature sensor in this frame | ✅ |
| others | constant (`34 40 00 00 00 7E 7E 00 … 50 …`) | ❓ |

Byte 16 always equals byte 17 in "day" mode and byte 18 in "night" mode.

## 6. Controller frame: `E4 21 xx 63 …`

| Byte | Meaning | Status |
|---:|---|---|
| 16–17 | exhaust temperature: `((b16·128 + b17) − 2000) / 10` °C | ✅ |
| 12–15 | `5D 5F` = "no sensor" (supply/outdoor/extract; sensors absent or not connected) | ✅ |
| 24, 25 | current fan speeds, % (per AERO 3B docs: 24 supply, 25 exhaust) | ✅ values; ❓ which is which (equal on our bus) |
| 8 | matches the fan speed in % in observations | ❓ |
| 26, 27, 28 | statuses (`00`/`08`/`10` in our observations); on other models these are mode, status and bypass | ❓ |

Temperature: the value is stored in two 7-bit bytes (`H·128 + L`), offset 2000, step 0.1 °C. This is the same formula as other C14 regulators. 🔸

## 7. Reading and writing registers (iNext window)

The controller and panel do not accept commands at any moment. After the `F0 44 … 3E` poll frame the panel stays silent for ~0.9 s. A frame of our own can be inserted into this window on behalf of iNext (`0x70`). The board waits for the `F0 44` frame, waits for it to end (50 ms UART pause), turns the transmitter on and sends the frame (31 ms at 9600 baud). The panel's reply arrives in the same cycle. ✅

### Request frame (30 bytes)

| Byte | Meaning |
|---:|---|
| 0 | `0x80 + panel address` = `BE` |
| 1 | `0x57` (`W`, write) or `0x52` (`R`, read) |
| 2 | checksum |
| 3 | `0x70` (iNext) |
| 4 | `00` |
| 5–28 | **6 slots of 4 bytes**: `[reg_hi, reg_lo, val_hi, val_lo]` |
| 29 | `23` |

- Register: `reg_hi = reg / 128`, `reg_lo = reg % 128`.
- Value: `raw = value + 2000`, bytes `raw / 128` and `raw % 128`.
- **Empty slot** `00 00 0F 50`: this is how other implementations fill unused slots, and on our bus an empty write changed nothing. ⚠️ But `0F 50` in a slot with a real register means the value **0**, not "nothing".
- In a read request (`R`) the values in the slots are ignored; `0F 50` is sent.

### Panel reply

Frame `F0 77 …` (to a write) or `F0 72 …` (to a read): the same 6 slots, but with values from the panel. For a write the panel returns the accepted value (if the write is accepted it equals the one sent). If the register does not exist the value is `5D 5F` (9999).

Example (reading registers 76–81, which do not exist on NANO XS):
```
request: BE 52 xx 70 00  00 4C 0F 50  00 4D 0F 50  00 4E 0F 50  00 4F 0F 50  00 50 0F 50  00 51 0F 50  23
reply:   F0 72 00 3E 7E  00 4C 5D 5F  00 4D 5D 5F  00 4E 5D 5F  00 4F 5D 5F  00 50 5D 5F  00 51 5D 5F  23
```
Example of writing register 8 = 4: reply `F0 77 0C 3E 7E 00 08 0F 54 …` (value `0F 54` = 2004 → 4). ✅

### What worked and what did not

- Reading registers 0–40 works; registers above 40 returned `5D 5F`. ✅
- Writing registers 8 and 9 changes the day and night speeds. The panel itself broadcasts them in its frame and the controller applies them. ✅
- The "fan mode" register from the NANO COLOR 2 documentation (80) does not exist on NANO XS. ✅
- Day/night mode (register 7) and on/off (register 6): read and write work; writing 0 to register 6 stops the fans. ✅

## 8. What we do not do

- We do not replace the master (the panel keeps working as before).
- We write nothing to the bus except the verified registers 6–9 on a command from HA: a register/value whitelist, no writes during the first 60 s after boot, one frame per bus window.
- The HA controls write only the verified registers 6–9 and only allowed values (0 or 1 for 6 and 7, 1–10 for 8 and 9).

## 9. Sources and differences from other models

There are write-ups of the same protocol for other combinations online. They **partly match** our bus (frame format, checksum, temperature formula, iNext window), but register numbering and byte values differ.

- [d-lebed/esphome-awenta-auros](https://github.com/d-lebed/esphome-awenta-auros): NANO COLOR 2 + AERO 4A, register read and write in the iNext window (`docs/PROTOCOL.md`, `docs/REGISTERS.md`).
- [Pioszym/ha-c14](https://github.com/Pioszym/ha-c14): AERO 3B + NANO Color, master emulation and frame analysis (`PROTOCOL.md`).
