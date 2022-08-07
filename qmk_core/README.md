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

To port a QMK keyboard to AAKBD, first you need to check that it uses an AVR
controller (preferably ATMEGA32U4 or ATMEGA32U2, but others may work). Other
controllers have not (yet) been ported, so the USB implementation won't work.
In theory you could reimplement the entire `avrusb.c`, but for now let's assume
AVR.

The makefile (e.g., `mykeyboard.mk`) should have at least the following:

``` Make
QMK_PLATFORM = avr
include qmk_core/qmk_port.mk

DEVICE_OBJS = mykeyboard.o avrusb.o $(QMK_CORE_OBJS)
```

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
