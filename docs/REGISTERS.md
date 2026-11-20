# C14 registers (NANO XS panel)

Registers are read and written with the `R` and `W` commands in the iNext window (frame format: [PROTOCOL.md](PROTOCOL.md)).
The value in a frame is sent as `value + 2000`; a register that does not exist replies `5D 5F` (9999).

> ⚠️ **Register numbering differs between models.** In other people's documentation (NANO COLOR 2 + AERO 4A) register 80 is "fan mode"; NANO XS has no register 80. Do not carry registers over from other documents.
> ⚠️ **Do not write to unknown registers.** Panel registers can include service settings and factory reset. The value `0F 50` in a slot is **0**, not "empty". The safe way to study registers is reading (the `R` command), not writing.

## What is known

| What | Register | Status |
|---|---|---|
| Day speed (1–10) | **8** | ✅ read and write |
| Night speed (1–10) | **9** | ✅ read and write |
| Day/night mode | **7**: `1` = day, `0` = night | ✅ read and write |
| Ventilation on/off | **6**: `1` = on, `0` = off | ✅ read and write |

## Registers 0–40 (snapshot of 2026-11-19: day 4, night 1, ventilation on, "day" mode)

| Reg | Value | Status | Notes |
|---:|---:|---|---|
| 0 | 0 | ❔ unknown | Purpose unknown. |
| 1 | 62 | ❓ hypothesis | 62 = 0x3E: matches the sender address of the panel (master) frames. Looks like "panel address on the C14 network". |
| 2 | 1 | ❔ unknown | In the NANO COLOR 2 docs register 2 = role on the C14 network, but this is not verified on NANO XS. |
| 3 | 0 | ❔ unknown | Purpose unknown. |
| 4 | 1 | ❔ unknown | Purpose unknown. |
| 5 | 1 | ❔ unknown | Purpose unknown. |
| 6 | 1 | ✅ confirmed | **Ventilation on/off**: 1 on, 0 off. Found by comparing scans; writing 0 stops the fans, writing 1 brings them back. |
| 7 | 1 | ✅ confirmed | **Day/night mode**: 1 day, 0 night. Found by comparing scans; writing it switches the panel mode (the active speed changes to the speed of the selected mode). |
| 8 | 4 | ✅ confirmed | **Day speed**, step 1–10. Read and write verified on live ventilation. |
| 9 | 1 | ✅ confirmed | **Night speed**, step 1–10. Read and write verified (slider write: 1→2→1). |
| 10 | 8 | ❔ unknown | Purpose unknown. |
| 11 | 0 | ❔ unknown | Purpose unknown. |
| 12 | 0 | ❔ unknown | Purpose unknown. |
| 13 | 20 | ❔ unknown | Purpose unknown. |
| 14 | 30 | ❔ unknown | Purpose unknown. |
| 15 | 1 | ❔ unknown | Purpose unknown. |
| 16 | 220 | ❓ hypothesis | 220: looks like 22.0 °C (setpoint in 0.1 °C). Not verified. |
| 17 | 1 | ❔ unknown | Purpose unknown. |
| 18 | 30 | ❔ unknown | Purpose unknown. |
| 19 | 50 | ❔ unknown | Purpose unknown. |
| 20 | 0 | ❔ unknown | Purpose unknown. |
| 21 | 112 | ❓ hypothesis | 112 = 0x70: iNext module address. Not verified (possibly a table of network device addresses). |
| 22 | 99 | ❓ hypothesis | 99 = 0x63: AERO controller address. Not verified. |
| 23 | 41 | ❓ hypothesis | 41 = 0x29: panel address from other models' documentation. Not verified. |
| 24 | 1 | ❔ unknown | Purpose unknown. |
| 25 | 11 | ❔ unknown | Purpose unknown. |
| 26 | 12 | ❔ unknown | Purpose unknown. |
| 27 | 13 | ❔ unknown | Purpose unknown. |
| 28 | 0 | ❔ unknown | Purpose unknown. |
| 29 | 0 | ❔ unknown | Purpose unknown. |
| 30 | 0 | ❔ unknown | Purpose unknown. |
| 31 | 0 | ❔ unknown | Purpose unknown. |
| 32 | 5 | ❔ unknown | Purpose unknown. |
| 33 | 800 | ❔ unknown | Purpose unknown. |
| 34 | 0 | ❔ unknown | Purpose unknown. |
| 35 | 0 | ❔ unknown | Purpose unknown. |
| 36 | 0 | ❔ unknown | Purpose unknown. |
| 37 | 0 | ❔ unknown | Purpose unknown. |
| 38 | 0 | ❔ unknown | Purpose unknown. |
| 39 | 0 | ❔ unknown | Purpose unknown. |
| 40 | 36 | ❔ unknown | Purpose unknown. |

Registers **41–233**, scanned 6 per request, returned `5D 5F`, i.e. they do not exist. Anything above 233 was not scanned.

## How the registers were found

By scanning registers 0–40 with reads (the `R` command) before and after changing a single setting on the panel; the register that changed is the one sought.

In practice: send `R` requests in the iNext window (6 registers per request, see [PROTOCOL.md](PROTOCOL.md#7-reading-and-writing-registers-inext-window)) until 0–40 are covered, and save the values. Change one setting on the panel (for example switch day/night), scan again and compare the two states. Always change one thing at a time, otherwise it is unclear which register is responsible for what.
