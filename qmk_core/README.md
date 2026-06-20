Quantum Mechanical Keyboard Firmware
====================================
This is a keyboard firmware library with some useful features for Atmel AVR and Cortex-M.

The full source code is available here: <https://github.com/qmk/qmk_firmware>

License
-------
**GPLv2** or later.

AAKBD Note
----------

AAKBD does not use QMK for the actual key processing, but parts of the firmware
(and the platform-specific abstraction layers) are included to ease porting
matrix-based keyboards from QMK to AAKBD. This is done mostly "for science",
since QMK is an established system that can do pretty much anything one could
ever want a keyboard to do, and has a large community and relatively
easy-to-use tooling going for it. I highly recommend using QMK instead of AAKBD
unless you happen to be of the very small minority that actually prefers to
define their keyboard by writing custom C code. In that case, AAKBD _may_ be
easier to start developing with, specifically because it is not as full of
features to begin with.


Porting Keyboards
-----------------

To port a QMK keyboard to AAKBD, the controller should be AVR (preferably
ATMEGA32U4 or ATMEGA32U2) or ARM (STM32F3 with TinyUSB). The USB
implementation uses `arch/avr/avrusb.c` for AVR or `arch/arm/tinyusb.c` for ARM.

For AVR, the makefile (e.g., `mykeyboard.mk`) should have at least:

``` Make
include arch/avr/avr-common.mk
include qmk_core/qmk_port.mk

DEVICE_OBJS = mykeyboard.o $(QMK_CORE_OBJS)
```

(`avr-common.mk` sets `ARCH = avr` and provides `avrusb.o` via `PLATFORM_OBJS`.)

For ARM, the makefile should additionally include the architecture build rules:

``` Make
MCU_FAMILY = stm32f3
include arch/arm/stm32-common.mk

BOOTLOADER_TYPE = dfu
include qmk_core/qmk_port.mk

DEVICE_OBJS = mykeyboard.o matrix_gpio.o ... $(filter-out matrix.o keymap.o, $(QMK_CORE_OBJS))
```

(`stm32-common.mk` includes `arm-common.mk`, which sets `ARCH = arm` and provides
the TinyUSB objects via `PLATFORM_OBJS`. `QMK_PLATFORM` is derived from `ARCH`
automatically.)

Then you need to port these files from QMK:

* `config.h` – the keyboard hardware configuration (you probably need some
  changes to disable things that are not used by AAKBD)
* `matrix.c` – ideally it should work without changes
* `keymap.c` – this needs to be created specifically for AAKBD: you must define
  only a single layer that has a _unique_ plain keycode (i.e., no commands
  or macros) for every key
* `layers.c` – use this to remap keys as desired, using the keycodes from
  `keymap.c` as the basis (which is why they have to be unique)

I recommend looking at `ergodox` and `modelf77` for examples. Good luck!
