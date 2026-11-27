# NANO ONE + AERO (O3ozon A-250): a second installation

A second, independent installation on the same C14 bus protocol, with a **NANO ONE** panel instead of NANO XS. It is kept in its own folder so nothing in the main AERO 2 + NANO XS firmware changes.

> ⚠️ Same warnings as the main [README](../README.md) apply, plus:
> - Verified on **one** installation: NANO ONE panel (firmware "wer. 20.01") + AERO controller whose info screen reads **"WENTYLACJA AERO5 v.57"** (it is not yet confirmed whether the controller hardware is the same as the AERO 2 in the main firmware).
> - NANO ONE register numbers are **different** from NANO XS. Do not mix the two configs.
> - Writes go to the panel only while the **"Разрешить запись" (write enable)** switch is on. The switch is always off after boot.

## Files

| Path | What it is |
|---|---|
| [esphome/nano-one-c14.yaml](esphome/nano-one-c14.yaml) | ESPHome config (M5Stack AtomS3 Lite + Tail485), entity names in Russian |
| [esphome/c14.h](esphome/c14.h) | read/write frame builders (iNext slot `0x70` → panel `0x29`) |

Secrets used: `wifi_ssid`, `wifi_password`, `recuperator_api_key`, `recuperator_ota_password`.

## Hardware

- M5Stack **AtomS3 Lite** + **Tail485** (auto direction, own DC-DC 9–24 V), Grove port G1 (RX) / G2 (TX).
- Powered from the bus terminal **U**, together with A/B/G. Wiring on the AERO board terminals (same colours as the panel cable): **35 U, 36 G, 37 B, 38 A**.
- Do not connect USB and bus power at the same time.

## Bus on this installation

| Address | Who |
|---|---|
| `0x29` (41) | NANO ONE panel, master ("MASTER MINI" on its C14 monitor screen) |
| `0x63` (99) | AERO controller |
| `0x70` (112) | iNext slot, polled with `F0 44 .. 29` and free: our board uses it |

Cycle ≈ **6.6 s**. Polls seen: `F0 44`, `E3 44`, `81 44`, `E2 44` (data `4D …`), `8B 44`; all poll frames to the controller carry **no data** (all zeros) on this installation.

### Controller frame `E4 21 xx 63`

| Byte | Meaning | Status |
|---:|---|---|
| 10–11 | intake temperature (after a ground heat exchanger here) | ✅ |
| 14–15 | **supply** air temperature ("Tnaw" on the panel info screen) | ✅ |
| 16–17 | **extract** air temperature ("Twyrzut") | ✅ |
| 18–19 | another temperature (~18 °C), not identified | ❓ |
| 24, 25 | always `00` here (on AERO 2 these are fan %) | ✅ |
| 26 | **active gear**: 0 = STOP, 1–3 = gear I–III | ✅ |
| 27 | `02` = fans running, `00` = stopped | ✅ |
| 28 | flags: `0x40` base, `0x20` = **bypass open** | ✅ |

### Panel status frame `E4 21 xx 29`

| Byte | Meaning | Status |
|---:|---|---|
| 5 | `0x50` day / `0x40` night (bit `0x10`) | ✅ |
| 7 | weekday (Mon = 0) | ✅ |
| 8–9 | clock | ✅ |
| 12–13 | room temperature | ✅ |
| 14–15 | **current** setpoint (switches day/night/away) | ✅ |
| 25 / 26 | rotating index 1..3 / value: year, month, day | ✅ |
| 27 | bit `0x08` toggles with season | ✅ |

### Panel settings frame `E4 2A xx 29` (~every 50 s)

| Byte | Meaning | Status |
|---:|---|---|
| 27 | bits 0–1: bypass mode (0 closed, 1 auto, 2 open); bit `0x08`: summer (Lato) | ✅ |
| 28 | gear code: `03` = I, `05` = II (`07` = III expected) | ✅ |

## NANO ONE registers (read/write via iNext, `A9 52/57 … 70`)

The panel misses about 1 in 3 requests: retry up to 5 times.

| Register | Meaning | Status |
|---:|---|---|
| 1 / 2 / 3 / 4 | setpoints ×0.1 °C: 200 / **day 210** / **night 200** / **away 160** | ✅ read (2, 3 changed together with the panel menu) |
| 6–11 | 1, 6, 1, 8, 10, 290 | ❓ |
| **21** | **Urlop (vacation)**: 1 = **fans STOP**, setpoint → away | ✅ read + write |
| 23–73 | **weekly schedule**, 7 registers per day, times in 15-min units (24 = 6:00, 33 = 8:15, 60 = 15:00, 92 = 23:00) | ✅ read |
| 30–35 | never answer | — |
| 85 | `1` the day after a filter change: maybe **filter-days counter** (as reg 85 in d-lebed's map) | ❓ |
| 108–110 | 4, 44, 5 | ❓ |
| **111** | **day-mode gear** 1–3 | ✅ read + write, applied immediately |
| **112** | **night-mode gear** 1–3 | ✅ read + write, applied immediately |
| 136 / 137 / 138 / 140 | 1 / 0 / 16 / 10: "poza domem" program (30 min / 30 min), secondary heater off, min. supply temp 16 °C, supply-temp correction gain 10 (match the service menu) | ✅ read |
| **169** | **filter cleaning period, days** ("Okres czyszcz. filtra"); the panel shows "remaining" = period − days elapsed | ✅ read (61→62→86 changed only this register); the firmware publishes it |
| 116–118 | 30 / 50 / 100 | ⚠️ write accepted, **no effect on the fans** |
| 130–135 | 30 / 60 / 100, 30 / 60 / 100 | ⚠️ write accepted, **no effect on the fans** |
| 197–204 | 30 / 60 / 80 / 100, 30 / 60 / 80 / 100 | ⚠️ write accepted, **no effect on the fans** |
| 240–247 | ASCII `COMPIT1_` | ✅ |
| 253 / 254 | 99 / 112 (AERO and iNext addresses) | ✅ |
| 162–167, 288, 300 | never answer | — |

"No effect" was tested by ear: gear I set to 60–100 % (and the gear toggled I→II→I so the panel re-applies it) stayed clearly quieter than gear II. So on this installation the **per-gear fan output is not in the panel registers** we can read.

## Service menu (NANO ONE, installer code 0199)

Code **0199** opens SERWIS (0000/0001 do not). The "Wentylacja" list for this controller has **no per-gear fan %**: min. supply temperature, "poza domem" program, secondary heater mode, correction gains, fan run-on, filter period, defrost threshold (5 °C) / hysteresis (4.0) / method (fan off), **PRESS input function** (here: *wietrzenie*/airing, so there is no dirty-filter pressure switch), outdoor temp from intake sensor, bypass permission. "Przywracanie nastaw" = factory reset, leave it alone.

This matches the AERO 5 manual (DTR v5.37): the controller drives the fans through **three relays R3/R4/R5 = gear 1/2/3** (R1 bypass, R2 ozonator, PWM outputs for heater SSRs) and has no 0–10 V fan output. So with AERO 5 smooth / 10-step speed is not possible over C14; it would need hardware changes (EC fan control signal) or a different controller (AERO 3/4/MF have per-gear % parameters).

## Reading the AERO controller directly

The controller **answers reads from the iNext slot** too: `E3 52 cs 70 …` → reply `F0 72 .. 63`, first try.

- Existing registers: **0–29** and **100–135** (everything else returns 9999).
- Values (identical at gear I and gear II, so these are configuration, not live data):
  0–17 = `0, 10, 20, 36, 0, 10, 10, 0, -20, 5, 1, 1, 40, 2, 1, 10, 1, 99` (17 = own address),
  18–26 = `16, 1, 0, 0, -20, -20, 0, 44, 0`, **101–103 = 323 / 326 / 321**, **121–123 = 716 / 719 / 713**.
- Nothing is written to the controller by this firmware.

⚠️ If you read the bus from a PC while the board runs, filter replies by sender byte 3: the board's own periodic panel reads show up on the bus too.

## Firmware behaviour

- Passive parsing: gear, temperatures, bypass, season, day/night, setpoint, links to panel and controller.
- Every 100 s, round-robin read of panel blocks 18 (Urlop), 108 (gears), 116 and 166 (filter period).
- Controls (only with the write switch on): day gear (111), night gear (112), Urlop (21).
- ESPHome 2026.11 builds internal IDs from the ASCII part of a name: two Cyrillic-only names of the same length collide, so one entity name carries "(filter)".
- Writes: one register per frame, the same slot repeated 6 times (no empty `00 00 0F 50` slots), only from HA, only with the write switch on; the panel's `F0 77` reply is checked, then the register is re-read.
- "Unknown flags" diagnostic sensors: status bytes with all decoded bits masked out, so any change means something new (filter alarm, fault…).

## Open questions

- Is the controller the same hardware as AERO 2? Compare the controller board labels / panel info screens. (If yours has per-gear % or 10 steps, it is not an AERO 5.)
- The filter "days elapsed" counter is not in the readable registers so far (maybe 162–165, which never answer); a day-to-day diff is planned.
- Filter alarm ("Wyczyść filtr") bits.
