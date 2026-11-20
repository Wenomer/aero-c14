# What to flash and where: build, flash, wiring

## What you need

| Item | Model in this installation |
|---|---|
| Board | Seeed XIAO ESP32-C6 |
| RS485 transceiver | Waveshare 3485 (3.3 V, pins RO, DI, RSE) |
| Wires | 5 Dupont wires; twisted pair to the bus (A, B) |
| Software | ESPHome 2026.10 (`pip install esphome`) or the ESPHome Builder in Home Assistant |

> ⚠️ The firmware is built for **AERO 2 + NANO XS panel**. It has not been tried with other models, see the [README](../README.md).

## 1. Wiring

| Waveshare 3485 module | XIAO ESP32-C6 | Note |
|---|---|---|
| VCC | 3V3 | 3.3 V supply |
| GND | GND | |
| RO | D7 (GPIO17) | receive (RX) |
| DI | D8 (GPIO19) | transmit (TX) |
| RSE (DE+RE) | D9 (GPIO20) | receive/transmit switch |

Connect **only A and B** to the bus (terminals "PANEL NANO XS, RS485 C14" on the AERO 2 controller board): module A to A, B to B. If no frames show up, swap A and B.

⚠️ Do **not** connect the **U** terminal (bus power). Work with the ventilation **powered off**; mains 230 V is on the other side of the board.

Before connecting you can measure the voltage on A and B relative to ground: normally about 1.7 V and 1.5 V, and 0.5–2 V between A and B (the bus is "alive").

## 2. Secrets

The config uses three secrets (names are in the header of `esphome/aero-c14.yaml`): `wifi_ssid`, `wifi_password`, `aero_c14__api_key` (key: `openssl rand -base64 32`).

Add them to the builder's `secrets.yaml`.

## 3. Create the device in the ESPHome Builder

1. Make sure the three secrets are in the builder's `secrets.yaml`. The API key also encrypts the over-the-air update (there is no separate OTA password), and for an already-installed board it **must match the key on the board**. An ESPHome version with OTA encryption is required (2026.10 or newer).
2. Create a device named **`aero-c14`** in the builder (same as `name:` in the config) and paste the contents of `esphome/aero-c14.yaml` over its default config.
3. Click **Install**. For a new board choose a USB install; later updates go **Wirelessly**.
4. After the board joins Wi-Fi, HA discovers it by itself (device "AERO C14").

⚠️ In the config the fallback AP password is a placeholder `CHANGE_ME_AP_PASSWORD` (in the `wifi: ap:` block). **Replace it with your own** (8+ characters) before flashing; do not commit your own password to the repository. If the board cannot reach Wi-Fi it starts this fallback access point `aero-c14 fallback`; connect to it and open `http://192.168.4.1`.

## 4. What appears in Home Assistant

| Entity | What it is |
|---|---|
| Ventilation speed | active speed, 1–10 |
| Ventilation day speed / night speed | stored day and night speeds |
| Ventilation mode | "Day" or "Night" |
| Ventilation on | ventilation is running |
| Exhaust air temperature | exhaust air temperature, °C |
| Supply fan percent / Exhaust fan percent | current fan speeds, %. The "supply" and "exhaust" labels come from the AERO 3B documentation and are **not verified on AERO 2**: here both values are always equal |
| **Day speed control / Night speed control** | sliders 1–10: write the speed to registers 8 and 9 |
| **Ventilation power** | ventilation on/off switch (register 6) |
| **Ventilation mode control** | "Day"/"Night" selector (register 7) |
| C14 write status, C14 last reply | write diagnostics |

### Changing speed, mode and power from HA

There is no "lock" switch: a write goes out immediately on command. Firmware safeguards: only registers 6–9 with allowed values, and writes are blocked for the first 60 seconds after the board boots.

1. Move the **Day speed control** / **Night speed control** slider, toggle **Ventilation power**, or pick a mode in **Ventilation mode control**. The board sends the write in the nearest window (up to ~4 seconds) and the slider returns to the actual value read from the bus.
2. If the write did not go through (no window within 30 seconds), "C14 write status" shows a message and the slider returns to its previous value.

## 5. If something goes wrong

- No frames: check A/B, the power of the ventilation and the panel, and `RSE` on D9. The board log should show `uart_debug` lines with 30-byte frames.
- Frames are there but look like garbage: swap A and B.
- Write "not accepted" (something odd in "C14 write status" and the speed does not change): the panel may be in a different state or window; retry and look at `C14 last reply`.
- Restoring the previous state: speeds can be set back with the buttons on the panel itself, which remains the bus master.
- The board breaks nothing if you disconnect it: the panel and controller work as before.
