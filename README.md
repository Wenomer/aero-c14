# aero-c14

ESPHome firmware that connects an **AERO 2** heat-recovery ventilation controller with a **NANO XS** room panel to Home Assistant over RS485 (C14 protocol). The board listens to the bus, shows speed, mode and exhaust temperature in HA, and lets you change the day/night speeds, the day/night mode and power.

> ## ⚠️ Read this first
>
> - **Verified only on an AERO 2 controller with a NANO XS panel** (a single installation). On other models (AERO 3/4/5, NANO ONE, NANO Color, etc.) frames, addresses and register numbers differ. Do not run this firmware on them based on this repository, and do not carry registers over from other people's documents.
> - This is an **unofficial** project; the protocol was worked out by observing the bus. The manufacturer does not support it and did not publish it.
> - The board **writes to the live ventilation bus**. Writes work, but you do this at your own risk; neither the author nor the repository accepts any liability (see [LICENSE](LICENSE)).
> - The board writes to the bus **only on a command from HA** and only to the verified registers 6–9 (power, day/night, speeds) with allowed values. There is no "write enable" switch; the safeguards are in the code: a register/value whitelist, no writes during the first 60 s after boot, at most one frame per bus window.
> - Work on the controller board with the power off: mains 230 V is present on the other side of the board.

## Features

| Feature | Status |
|---|---|
| Read: active speed, day speed, night speed, day/night mode, on/off, exhaust temperature, fans | ✅ works |
| Write day and night speed (HA sliders, registers 8 and 9) | ✅ works |
| Switch day/night (register 7) and on/off (register 6) from HA | ✅ works, verified from HA |
| Other panel registers (0–40) | ❔ purpose mostly unknown, see [REGISTERS.md](docs/REGISTERS.md) |
| Supply, outdoor and extract temperatures | ❌ the controller reports "no data" (sensors absent or not connected) |

## Repository contents

| Path | What it is |
|---|---|
| [esphome/aero-c14.yaml](esphome/aero-c14.yaml) | firmware config (XIAO ESP32-C6 + Waveshare 3485) |
| [docs/FLASHING.md](docs/FLASHING.md) | **what to flash and where**: wiring, flashing, HA Builder, usage |
| [docs/PROTOCOL.md](docs/PROTOCOL.md) | description of the C14 protocol on this bus |
| [docs/REGISTERS.md](docs/REGISTERS.md) | known and unknown registers |
| [nano-one/](nano-one/README.md) | a second installation: **NANO ONE** panel + AERO ("AERO5 v.57"), separate config and register map |

## Quick start

1. Connect the RS485 module to the board and to the controller's A and B terminals: [docs/FLASHING.md](docs/FLASHING.md#1-wiring).
2. Create three secrets (`wifi_ssid`, `wifi_password`, `aero_c14__api_key`): [docs/FLASHING.md](docs/FLASHING.md#2-secrets).
3. In the ESPHome Builder create a device `aero-c14`, paste `esphome/aero-c14.yaml` and install it: [docs/FLASHING.md](docs/FLASHING.md#3-create-the-device-in-the-esphome-builder).
4. Home Assistant discovers the device by itself.
5. Change speeds, mode and power with the HA tiles (on change the board sends the write to the bus itself).

## How it works (short)

- Bus: RS485, 9600 8N1, 30-byte frames. The NANO XS panel (address `0x3E`) is the master and polls the other devices about every 4 seconds; the AERO 2 controller (`0x63`) replies.
- Day and night speeds are stored in the panel as registers 8 and 9 and broadcast in the status frame (bytes 17 and 18).
- Writing: in the ~0.9 s pause after the `F0 44` poll the board sends a write frame on behalf of an iNext module (`0x70`); the panel acknowledges it and applies the value. The panel stays the master, so its buttons work as before.

Details: [docs/PROTOCOL.md](docs/PROTOCOL.md).

## Sources

The protocol was studied on a real bus and cross-checked against other people's write-ups ([d-lebed/esphome-awenta-auros](https://github.com/d-lebed/esphome-awenta-auros), [Pioszym/ha-c14](https://github.com/Pioszym/ha-c14)). All links and model differences: [docs/PROTOCOL.md](docs/PROTOCOL.md#9-sources-and-differences-from-other-models).

## License

[MIT](LICENSE).
