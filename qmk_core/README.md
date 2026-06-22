# Quantum Mechanical Keyboard Firmware

The original QMK firmware is available at: <https://github.com/qmk/qmk_firmware>

The files in this directory are mostly copied or adapted from there, in order
to facilitate porting keyboards from QMK firmware to AAKBD. QMK provides the
hardware access (matrix scanning, EEPROM, jump to bootloader, LEDs, RGB,
haptics), while AAKBD provides the actual keyboard engine (layers, macros, USB
implementation).

Thus, AAKBD does not use QMK for the actual key processing. This is done mostly
"for science", since QMK is an established system that can do almost anything
one could ever want a keyboard to do, and has a large community and relatively
easy-to-use tooling going for it. I highly recommend using QMK instead of AAKBD
unless you happen to be of the very small minority that actually prefers to
define their keyboard by writing custom C code. In that case, AAKBD _may_ be
easier to start developing with, specifically because it is not as full of
features to begin with.

AAKBD also may have some (arguable) advantages in its USB implementation
efficiency (e.g., uses less USB bandwidth, due to less bloat from rarely-needed
features). And the custom code does allow completely customizable things like
the keyboard can report the actual serial number of your keyboard, pretend to
be an Apple keyboard for the Apple Fn key to work correctly, identify the host
computer's OS fingerprint to automatically toggle Windows/macOS/Linux layers
based on which computer it is plugged in to, custom RGB effects, etc.

## Porting Keyboards from QMK to AAKBD

To port a QMK keyboard to AAKBD, the controller should be AVR (preferably
ATMEGA32U4 or ATMEGA32U2) or ARM (STM32F3 with TinyUSB). The USB
implementation uses `arch/avr/avrusb.c` for AVR and `arch/arm/tinyusb.c` for
ARM. It should hopefully be quite easy at this point to add support for other
microcontroller types, but currently they are limited by the keyboards
I actually have to develop on.

### AVR

For AVR, the makefile (e.g., `mykeyboard.mk`) should have at least:

``` Make
BOOTLOADER_TYPE = dfu

include arch/avr/avr-common.mk
include qmk_core/qmk_port.mk

DEVICE_OBJS = mykeyboard.o matrix.o $(QMK_CORE_OBJS)
```

(`avr-common.mk` sets `ARCH = avr` and provides `avrusb.o` via `PLATFORM_OBJS`.)

See `ergodox` and `modelf77` for examples.

### ARM

For ARM, the makefile should additionally include the architecture build rules:

``` Make
MCU_FAMILY = stm32f3
BOOTLOADER_TYPE = dfu

include arch/arm/stm32-common.mk
include qmk_core/qmk_port.mk

DEVICE_OBJS = mykeyboard.o matrix_gpio.o $(QMK_CORE_OBJS)
```

(`stm32-common.mk` includes `arm-common.mk`, which sets `ARCH = arm` and provides
the TinyUSB objects via `PLATFORM_OBJS`. `QMK_PLATFORM` is derived from `ARCH`
automatically.)

See `gmmkpro1` for an example.

### Infrastructure

Then you need to port these files from QMK:

* `config.h` – the keyboard hardware configuration (you probably need some
  changes to disable things that are not used by AAKBD)
* `matrix.c` – ideally it should work without changes (if the keyboard has no
  custom matrix, just add `matrix_gpio.o` to the DEVICE_OBJS to use the default
  GPIO-based matrix, which is already provided)
* `keymap.c` – this needs to be created specifically for AAKBD: you must define
  only a single layer that has a _unique_ plain keycode (i.e., no commands
  or macros) for every key (this defines a unique name for every key, which is
  then referenced in `layers.c` for remapping)
* `layers.c` – define your custom keymaps and layers here
* `macros.c` – define macros and other custom hooks (e.g., RGB effects, host OS
  fingerprint handling, custom keypress processing)
