## Overview

AAKBD is a USB keyboard firmware. Remapping is expressed as *differences from the default named-key mapping*, not as complete positional layer definitions. Customisation is in C (`layers.c`, `macros.c`).

Supported devices: `ps2usb`, `ergodox`, `modelf77`, `modelf62`, `modelf50`.

## Build

Requires: `avr-gcc`, `avr-libc`, GNU `make`.

```sh
make DEVICE=ps2usb       # default
make DEVICE=ergodox
make DEVICE=modelf77
make clean
make distclean
```

Additional flash targets exist but should not be run automatically.

### Configuration

Place `local.mk` in root (Git-ignored). Device overrides in `DEVICE/local.mk`. Config is `-D` defines via `CONFIG_FLAGS`. Options in `usbkbd_config.h`. Example:
```make
CONFIG_FLAGS = -DUSB_MAX_KEY_ROLLOVER=10
```

DO NOT edit or delete user's `local.mk`, `layers.c`, `macros.c` without prompting.

### First-time setup

`make` copies `DEVICE/template_layers.c` → `DEVICE/layers.c` and `template_macros.c` → `macros.c`. Edit these (Git-ignored). Templates are inactive until `LAYER_COUNT > 0`.

`make .ccls` generates a `.ccls` file for clangd.

## Architecture

### Key processing pipeline

`Physical key → process_key() → layer resolution → preprocess_press() hook → execute_macro() or standard processing → usb_keyboard_press/release() → usb_keyboard_send_if_needed()`

### Core files

| File | Role |
|------|------|
| `keys.c` / `keys.h` | Key processing; includes `layers.c` and `macros.c` at compile time |
| `keycodes.h` | 16-bit keycodes: modifier combos, layer ops, macros, tap/hold |
| `layers.h` | `DEFINE_LAYER()` macro, PROGMEM storage |
| `macros.h` | Helpers for macros and hooks |
| `usbkbd.c` / `usbkbd.h` | USB HID report construction |
| `usbkbd_config.h` | Compile-time options |
| `usb_keys.h` | USB keycode enum |
| `avrusb.h` | Low-level AVR USB macros |
| `avrusb.c` | Low-level AVR USB driver (always use avrusb.h macros, no direct
device / register access!) |
| `main.h` | Interface each device main must implement |

### Device directories

Each device (e.g., `ps2usb/`, `ergodox/`): `DEVICE.c` (main), `DEVICE.mk` (build vars), `template_layers.c` / `template_macros.c` (copied to untracked `layers.c`/`macros.c`).

`qmk_core/` — ported QMK driver code (matrix scanning, debounce, I2C). Used by ErgoDox and Model F devices.

### Layer system

- Layer 0: implicit default (physical key → USB keycode, device-defined)
- Layers 1–31: user-defined overlays in PROGMEM (sparse, only mapped keys)
- `LAYER_COUNT` = highest used layer number
- Base layer (default 1): always active; layers below are suppressed
- Transparent = pass through to next lower active layer

```c
DEFINE_LAYER(1) {
    [KEY(CAPS_LOCK)] = CTRL_OR(ESC),
};
```

### Extended keycodes

`keycodes.h` defines 16-bit keycodes: `SHIFT(X)`, `CTRL_OR(ESC)`, `CMD_OR(ESC)`, `LAYER_TOGGLE(n)`, `LAYER_TOGGLE_HOLD(n)`, `LAYER_SET_BASE(n)`, `MACRO(name)`, `EXT(ENTER_BOOTLOADER)`, etc.

### Macros

`MACRO(name)` where `name` is in `enum macro` in `layers.c`. Implementation in `execute_macro()`:
```c
void execute_macro(uint8_t macro_number, bool is_release, uint8_t physical_key, uint8_t *restrict data);
```
`*data` persists one byte from press to release per key. Helpers: `register_key()`, `add_strong_modifiers()`, `add_weak_modifiers()`, `clear_strong_modifiers()`, `usb_keyboard_simulate_keypress()`.

### Hooks in macros.c

- `preprocess_press(keycode, physical_key, data)` → returns `keycode_t` (override or pass through)
- `postprocess_release(keycode, physical_key, data)`
- `layer_state_changed(layer, is_enabled)`
- `handle_tick(tick_10ms_count)` — ~10 ms timer
- `handle_reset()` — init / reset
- `keyboard_host_leds_changed(leds)`

### Memory constraints

AVR targets: ATMEGA32U4 (~2.5 KB RAM, 32 KB flash) or ATMEGA32U2 (1 KB RAM, 32 KB flash). Layers in PROGMEM via `DEFINE_LAYER`. Key buffer + modifier state + layer mask ≈ 100 bytes RAM.
