## ErgoDox Ez Alternative AAKBD Firmware

This subdirectory of [AAKBD](https://github.com/arkku/aakbd) contains an
alternative firmware for the ErgoDox Ez keyboard. The included
[layers.c](template_layers.c) template defines the default keymap
pretty much identically to the ErgoDox original, except the third layer
(with mouse and media keys) is missing since AAKBD does not currently
implement that.

Much of the code in this directory is from the
[QMK](https://github.com/qmk/qmk_firmware) project. I have mostly just
deleted stuff from it – any bugs caused by that are almost certainly
my fault.

## Configuration

Set `DEVICE` to `ergodox` either in `local.mk` (in the project root directory),
or on the command line (i.e., `make DEVICE=ergodox`).

You can have two complementary `local.mk` files: one the the project root
directory, and `ergodox/local.mk` specific to ErgoDox Ez.

For example:

``` Make
DEVICE = ergodox
DEBOUNCE = 8
POLL_INTERVAL = 1

CONFIG_FLAGS =  \
               -DUSB_VENDOR_ID=$(VENDOR_ID) \
               -DUSB_PRODUCT_ID=$(PRODUCT_ID) \
               -DMANUFACTURER_STRING='$(MANUFACTURER)' \
               -DPRODUCT_STRING='$(PRODUCT)' \
               -DKEYBOARD_POLL_INTERVAL_MS=$(POLL_INTERVAL) \
               -DENABLE_BOOTLOADER_SHORTCUT=0 \
               -DENABLE_RESET_SHORTCUT=0 \
               -DENABLE_DEBUG_SHORTCUT=0 \
               -DENABLE_SIMULATED_TYPING=1 \
               -DDVORAK_MAPPINGS=1
```

After creating this file, run `make` once to create
[layers.c](template_layers.c) and [macros.c](template_macros.c) from the
included templates. These files are created in this `ergodox` directory.

You may then edit `layers.c` and `macros.c` to configure the keyboard.

Note the `DEBOUNCE` option – if you experience bouncing keys (i.e.,
repeated keypresses when you pressed it only once), increase this value.
However, that also means you can't double-tap a key faster than this
number of milliseconds.

Recompile by running `make` again.

## Flashing Firmware

Use [Wally](https://www.zsa.io/wally/) to flash the firmware. After flashing
this firmware, you need to use a key bound to `EXT(ENTER_BOOTLOADER)` or
press the reset button to flash again. The reset button is deeply recessed
in the top right corner of the right side keyboard module – press it with a
straightened paperclip. If you change the USB vendor and/or product id, Wally
may not recognise the keyboard. You can work around that by plugging it in
while pressing the reset button.
