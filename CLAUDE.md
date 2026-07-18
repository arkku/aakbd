## Overview

AAKBD is a USB and PS/2 keyboard firmware. Remapping is expressed as *differences from the default named-key mapping*, not as complete positional layer definitions. Customisation is in C (`layers.c`, `macros.c`, `layers_$(MODEL).c`, `macros_$(MODEL).c`).

Supported devices: `ps2usb`, `ergodox`, `modelf77`, `modelf62`, `modelf50`, `gmmkpro1`, `fext`.

## Build

```sh
make DEVICE=modelf77        # AVR
make DEVICE=gmmkpro1        # ARM (requires git submodule update --init)
make MODEL=vial             # Enable Vial
make clean
make distclean
```

Do not run `make upload`, `make dfu`, `make burn` or other hardware writes!

### Configuration

Place `local.mk` in project root. Device overrides in `DEVICE/local.mk`. Config is `-D` defines via `CONFIG_FLAGS`. Options in `usbkbd_config.h`. Example:
```make
CONFIG_FLAGS = -DUSB_MAX_KEY_ROLLOVER=10
```

**Do not** edit or delete user's `local.mk`, `layers.c`, `macros.c`, `layers_*.c`, `macros_*.c` without prompting. They are gitignored so changes are unrecoverable! Even if prompted, never overwrite large parts without taking a backup.

Everything must work with sane defaults without these files present. (But do not delete or move them out of the way on your own for testing, they are sacred user input.)

Any required configuration options must be defined in **one place** and used
from there throughout. If there can be no safe default guaranteed to work,
always **fail at compile time** - do not guard with `#ifndef` if the default
might be incorrect (e.g., if you don't know GPIO pin assignments or USB
endpoint configuration, you cannot just assume and not failing at compile time
would be horrible footgun).

## Makefile System

### Hierarchy (most general → most specific)

A makefile should only know about things ABOVE it in this list. `local.mk` and `{device}/local.mk` sit at their respective level as user overrides.

1. **Root `Makefile`** — all AAKBD firmwares. Defines `OBJS = $(OBJ) usbkbd_descriptors.o usbkbd.o keys.o $(DEVICE_OBJS) $(PLATFORM_OBJS)`. Default `ARCH = avr`.
2. **`arch/{arch}/{arch}-common.mk`** — architecture-specific (e.g., `arch/arm/arm-common.mk`, `arch/avr/avr-common.mk`). Sets `ARCH`, toolchain, MCU flags, `PLATFORM_OBJS`. Must NOT reference QMK.
3. **`arch/arm/stm32-common.mk`** — STM32 MCU family specifics. Sets `MCU_FAMILY`, `MCU_OBJS`, linker script, CMSIS paths. Includes `arm-common.mk`.
4. **`qmk_core/qmk_port.mk`** — QMK port bridge. Defines `QMK_CORE_OBJS` (keyboard.o, qmk_main.o, matrix_common.o, debounce, etc.), `COMMON_HEADERS`, vpath for QMK sources, `DEVICE_FLAGS`. Derives `QMK_PLATFORM` from `ARCH`.
5. **`qmk_core/platforms/{arch}/qmk_{arch}.mk`** — Platform compiler flags and dep rules (included automatically by `qmk_port.mk` via `$(PLATFORM_DIR)/qmk_$(QMK_PLATFORM).mk`).
6. **`{device}/{device}.mk`** — Keyboard-specific. Sets `MCU_FAMILY`/`BOOTLOADER_TYPE`, `DEVICE_OBJS`, `DEVICE_FLAGS`, includes the architecture and QMK layers above.

Makefiles must be kept up to date with changes to files/dependencies.

### Variables

| Variable | Set by | Value | Notes |
|----------|--------|-------|-------|
| `ARCH` | `{arch}-common.mk` | `avr` / `arm` | Default `avr` in root Makefile; overridden by `arm-common.mk` |
| `QMK_PLATFORM` | `qmk_port.mk` | `$(ARCH)` | Derived automatically: `QMK_PLATFORM ?= $(ARCH)` |
| `MCU_FAMILY` | device `.mk` or `stm32-common.mk` | e.g., `stm32f3` | ARM only; selects MCU-specific startup code |
| `PLATFORM_OBJS` | `{arch}-common.mk` | AVR: `avrusb.o`; ARM: `syscalls.o $(MCU_OBJS) $(TINYUSB_OBJS)` | Added to root `OBJS` |
| `DEVICE_OBJS` | device `.mk` | Device-specific objects | May include `$(QMK_CORE_OBJS)` filtered |
| `BOOTLOADER_TYPE` | device `.mk` | `dfu` / `halfkay` | Set in device `.mk` where `QMK_PLATFORM` is used |
| `DEVICE_FLAGS` | device `.mk` / `qmk_port.mk` | Compiler flags | Prepend-only via `+=` |
| `CC_FLAGS` | `Makefile` / `arm-common.mk` | Include paths, arch flags | Overridden by ARM; AVR uses `AVR_FLAGS` |
| `CFLAGS` | Root `Makefile` | `$(CUSTOM_FLAGS) $(CC_FLAGS) $(DEVICE_FLAGS) $(CONFIG_FLAGS)` | Final compiler flags |
| `MODEL` | User (command line / `local.mk`) | Free-form string | Picks `layers_$(MODEL).c` / `macros_$(MODEL).c`; change forces clean rebuild |

### Build variable chain

`DEVICE_FLAGS` → `CC_FLAGS` → `CFLAGS` (in root `Makefile`). `DEVICE_FLAGS` can be overridden via `local.mk` or command line. `CONFIG_FLAGS` provides user-facing `-D` definitions.

### Include chain

```
device.mk → arch/{arch}/{arch}-common.mk → qmk_port.mk → qmk_{arch}.mk
```

For ARM with STM32: `device.mk → stm32-common.mk → arm-common.mk → qmk_port.mk → qmk_arm.mk`

### Dependency rules

All `.o` files must have dependencies defined in the one most relevant makefile (e.g., `qmk_port.mk` for platform-independent QMK files). Always maintain these dependencies when adding/removing header `#include`s and when adding new `.c` files. Use `$(COMMON_HEADERS)` where appropriate; it is ok if it adds more dependencies than are actually used.

## File Layout

### Root files

These must remain compatible with all architectures and devices. Deviations from this must go into more specific directories.

| File | Role |
|------|------|
| `keys.c` / `keys.h` | Key processing; includes `layers.c` and `macros.c` at compile time |
| `keycodes.h` | 16-bit extended keycodes: modifier combos, layer ops, macros, tap/hold |
| `layers.h` | `DEFINE_LAYER()` macro, PROGMEM storage |
| `macros.h` | Helpers for macros and hooks |
| `usbkbd.c` / `usbkbd.h` | USB HID report construction; includes PS/2 output calls for simulated typing |
| `usbkbd_config.h` | Compile-time options |
| `usb_keys.h` | USB keycode enum |
| `generic_hid.h` | Generic HID endpoint support (default disabled) |
| `main.h` | Interface each device main must implement |

### `ps2/` — PS/2 protocol implementation

Shared PS/2 host and device mode code. Referenced via `-Ips2` when either is enabled.

| File | Role |
|------|------|
| `kk_ps2_device.c/h` | PS/2 device-mode (delay-based) bit-banging I/O, 11-clock send / 12-clock receive |
| `kk_ps2_host.c/h` | PS/2 host-mode (interrupt-based) — moved from `ps2usb/` |
| `kk_ps2_avr.h` | AVR pin macros and helpers (CLOCK/DATA GPIO) |
| `kk_ps2.h` | Shared command/reply constants |
| `ps2_keys.h` | PS/2 scancode enum (shared between host and device modes) |
| `ps2_output.c/h` | PS/2 device-mode output orchestration: async command handler, key event queue, scancode set management, repeat |
| `usb2ps2_keys.c/h` | USB→PS/2 scancode tables for sets 1/2/3 |
| `ps2_output_test.c` | Unit tests for `ps2_output.c` |
| `kk_ps2_device_test.c` | Edge-by-edge device I/O tests |
| `gen_test_runner.sh` | Auto-generates test runner scanning `test_*` functions |
| `Makefile` | Build rules for both test binaries |
| `ps2_keys.h` | PS/2 scancode enum (shared between host and device modes) |

#### Testing PS/2 output (on host, no hardware needed)

```sh
make -C ps2 test          # ps2_output.c unit tests
make -C ps2 device_test   # kk_ps2_device_test.c unit tests
```

The test outputs contain only errors and final result, **do not grep or tail**.

#### Testing Vial output (on host, no hardware needed)

```sh
make -C vial test
```

The test outputs contain only errors and final result, **do not grep or tail**.

### `arch/{arch}/` — Architecture-specific

Do not reference QMK- or device-specific things from these files - these are universal for the architecture.

| Directory | Contents |
|-----------|----------|
| `arch/avr/` | AVR USB driver (`avrusb.c/h`), timer helpers, `avr-common.mk` (sets `ARCH = avr`, `PLATFORM_OBJS = avrusb.o`) |
| `arch/arm/` | ARM toolchain, TinyUSB bridge (`tinyusb.c/h`), USB config (`tusb_config.h`), `arm-common.mk` (sets `ARCH = arm`, `PLATFORM_OBJS`, DFU targets) |
| `arch/arm/stm32f3/` | STM32F3 clock init, vector table, linker script, TinyUSB hardware setup |

### `qmk_core/` — QMK code and porting helpers

Files to assist in porting keyboards from QMK. Use direct copies from QMK where possible, note in top comment if modified.

| Directory | Contents |
|-----------|----------|
| `qmk_core/platforms/avr/` | AVR platform: timer, suspend, I2C, SPI, EEPROM, `qmk_avr.mk`, `qmk_port.c` |
| `qmk_core/platforms/arm/` | ARM platform: GPIO macros, DWT timer, SPI, DFU bootloader, EEPROM stubs, `qmk_arm.mk`, `qmk_port.c` |
| `qmk_core/drivers/` | Hardware drivers (AW20216S RGB LED, etc.) |
| `qmk_core/debounce/` | Debounce algorithms (selected by `DEBOUNCE_TYPE`) |

`qmk_core/qmk_main.c` is the `main()` file. QMK provides keyboard hardware config (matrix scanning, drivers), whereas AAKBD provides the USB hardware and keyboard software layers.

### `lib/` — Unmodified vendor originals

No modifications allowed! Include LICENSE if adding a new library.

| Directory | Contents |
|-----------|----------|
| `lib/CMSIS/` | ARM CMSIS-Core (Apache 2.0) |
| `lib/STM32F3xx/` | STM32F3 CMSIS (ST BSD) |
| `lib/tinyusb/` | TinyUSB USB stack (git submodule, MIT) |

### `DEVICE/` — Keyboard-specific

Each device (e.g., `ps2usb/`, `ergodox/`, `gmmkpro1/`, `modelf77/`):

| File | Role |
|------|------|
| `README.md` | Human-readable info about the device and its configuration |
| `DEVICE.c` | Main program |
| `DEVICE.mk` | Build variables; includes architecture + QMK layers |
| `config.h` | Matrix pins and other QMK config |
| `template_layers.c` / `template_macros.c` | Example files, copied to untracked `layers.c`/`macros.c` |
| `keymap.c` | QMK keymap — matrix positions → unique keycodes |
| `led_map.c` / `led_map.h` | Keycode-to-LED mapping (RGB keyboards) |

## Key Processing

### Pipeline

```
Physical key → process_key() → layer resolution → preprocess_press() hook → execute_macro() or standard processing → usb_keyboard_press/release() → usb_keyboard_send_if_needed()
```

## Vial

- **`vial/vial.c`** / **`vial/vial.h`** — Vial protocol handler, bootloader magic, unlock
- **`vial/vial_keys.c`** – Vial/QMK keypresses handler (mainly called by `keys.c`)
- **`vial/dynamic_keymap.c/h`** — EEPROM layout: keymaps, combos, tap dance, encoders, macros
- **`vial/via_handler.c`** — VIA command dispatch
- **`vial/qmk_translate.c`** – AAKBD/QMK bidirectional keycode translation
- **`DEVICE/keymap.c`** MUST define: `vial_unlock_combo_rows/cols/len` (unlock keys, same half), `vial_keyboard_uid`, `vial_default_layout_options`

Vial stores a number of dynamic layers in EEPROM or flash (`VIAL_LAYER_COUNT`), in addition there can be static layers (`STATIC_LAYER_1` etc.) that appear in Vial GUI as read-only, in `layers_vial.c`. EEPROM layers store QMK keycodes which are translated to AAKBD at the read/write interface, static layers store AAKBD keycodes that are translated to QMK keycodes for Vial GUI. Several QMK keycodes translate to the same `EXTENDED(QMK_KEYCODE)` - the processing of this keycode falls through to `vial_process_qmk_keycode(…)`, which loads the original QMK keycode from EEPROM and processes that.

### PS/2 output

In PS/2 device mode (`ENABLE_PS2_DEVICE=1`), key events are queued via
`ps2_press_key()` / `ps2_release_key()` and processed asynchronously by
`ps2_output_task()`. The queue holds 10 events (5 key press+release
pairs). `ps2_output_task()` drains the queue, sends scancodes, handles repeat,
and processes incoming host commands - it must be called frequently, e.g., if
waiting 10 ms it should be done in 1 ms increments while calling the task.

### Layer system

- Layer 0: implicit default (physical key → *unique* USB keycode, device-defined)
- Layers 1–31: user-defined overlays in PROGMEM (sparse, only mapped keys)
- `LAYER_COUNT` = highest used layer number (inclusive, layer 0 is not counted)
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

```c
void execute_macro(uint8_t macro_number, bool is_release, uint8_t physical_key, uint8_t *restrict data);
```

`*data` persists one byte from press to release per key. Helpers: `register_key()`, `add_strong_modifiers()`, `add_weak_modifiers()`, `clear_strong_modifiers()`, `usb_keyboard_simulate_keypress()`.

### Hooks in macros.c

- `preprocess_press(keycode, physical_key, layer, data, row, col)` → returns `keycode_t` (override or pass through)
- `postprocess_release(keycode, physical_key, data)`
- `layer_state_changed(layer, is_enabled)`
- `handle_tick(tick_10ms_count)` — ~10 ms timer
- `handle_reset()` — init / reset
- `keyboard_host_leds_changed(leds)`

### Memory constraints

| Target | Flash | RAM | Notes |
|--------|-------|-----|-------|
| ATMEGA32U4 | 32 KB | ~2.5 KB | Layers in PROGMEM |
| ATMEGA32U2 | 32 KB | 1 KB | AVR: key buffer + modifier state + layer mask ≈ 100 bytes RAM |
| STM32F303CC | 256 KB | 32 KB | No PROGMEM needed — const in `.rodata` |

## Coding Rules

- All architectures and devices must continue to work at all times (but no need to test building every device during focused development, just do a regression test once all other tasks are done and verified)
- No magic numbers - define well-named macros (in exactly one place)
- Fail at compile time for any unsupported/non-sensical configuration, do not silently correct it
- Fail at compile time if some hardware-dependent configuration is not set, do not assume defaults if they can be wrong (e.g., pin assignments, MCU type) - the only exception is that "extra" features (like rotary encoders or RGB lighting) can default to absent when the only downside is that optional feature not being active
- Each configuration option (or its default value) should be set in one file and if it is needed elsewhere, then that one header should be included
- Do not depend on order of header inclusion - include headers recursively if needed
- Each `*.o` file should have its dependencies defined in exactly one makefile, the one nearest to the actual location
- Do not hide issues by forcing configuration options where the issue doesn't occur, unless those configuration options are inherently part of the device spec
- Do *not* cast unused arguments to `(void)`
- C-code formatting: `.clang-format` at project root

