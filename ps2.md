# PS/2 Output for AAKBD

PS/2 output device mode for existing AAKBD keyboards. Supports scancode sets 1 and 2
(Set 3 is stubbed, disabled by default). Runtime USB/PS/2 detection — single binary
works with both.

## Current State

- [x] Device-mode byte tx/rx in `kk_ps2_device.c` (delay-based, generates CLK)
- [x] TX queue reuses `ps2_buffer[]` as ring buffer (non-blocking key sends)
- [x] Scancode Set 1 + Set 2 full tables
- [x] Set 1 break format (`| 0x80`), Set 2/3 break format (`F0` prefix)
- [x] Extended keys (E0 prefix) per set
- [x] Num Lock / Shift interaction for keypad navigation keys (Notes 1, 2)
- [x] Print Screen with Alt (Note 4)
- [x] Keypad / with Shift (Note 3)
- [x] Pause (no break code) / Break (Ctrl+Pause) (Note 5)
- [x] NKRO support (`ENABLE_PS2_NKRO`, default off)
- [x] Modifier tracking via `ps2_sent_modifier_flags` + `usb_keyboard_set_modifiers()` diff
- [x] `release_all_keys` iterates buffer (regular keys) + flag bitmask (modifiers)
- [x] Runtime USB/PS/2 detection in `qmk_main.c`
- [x] All PS/2 source files in `ps2/` directory
- [x] `ps2usb` binary identical to main (verified with local.mk)
- [x] Changes outside `ps2/` are minimal and guarded by `ENABLE_PS2_DEVICE`
- [x] Configurable device ID (`PS2_DEVICE_ID`, default `0xAB83`, `0x0000` = plain ACK)
- [x] Host commands fully implemented: Reset, Enable, Disable, Set Defaults, Set LEDs,
      Echo, Set Scancode, Set Rate, Read ID, Set All Keys (F7–FA), Set Some Keys
      (FB–FD, with proper command termination processing)

## Not Done

- [ ] **Set 3 full implementation** — scancode table exists but per-key 2-bit state
      (repeat/break), device-side typematic repeat, and break suppression are not
      implemented. Set 3 is disabled by default (`ENABLE_PS2_DEVICE_SET_3 0`);
      `F0 03` returns `RESEND`.
- [ ] **Regression build of all devices** — need to build modelf62, modelf50,
      ergodox, gmmkpro1 to confirm no breakage
- [ ] **ARM port** — all AVR-specific macros are in `kk_ps2_avr.h`; ARM would need
      equivalent macros (GPIO + delay + optional interrupt) in a new `avr` → `arm` variant
- [ ] **Set 1 problematic scancodes** — scancodes above `0x79` have break codes
      `>= 0xFA` which the keyboard driver interprets as 8042 responses (ACK `0xFA`,
      RESEND `0xFE`) rather than key events, causing lost releases. The values `0x60`
      and `0x61` break to `0xE0`/`0xE1` (reserved prefix bytes). F13–F24 (`0x64–0x76`)
      fall partly into these ranges. Documented in ps2scancodes.md but not actively
      handled — the keyboard relies on the host to handle translation correctly.

## Build

```
make DEVICE=modelf77 ENABLE_PS2_DEVICE=1
```

Size (atmega32u2, with local.mk):
- Without PS/2: ~21900 text + 32 data + 628 bss
- With PS/2:    ~25000 text + 34 data + 890 bss
