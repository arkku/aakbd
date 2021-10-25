## Brand New Model F62 Keyboard Alternative Firmware

This subdirectory of [AAKBD](https://github.com/arkku/aakbd) contains an
alternative firmware for the Brand New Model F62 keyboard, ported from
pandrew's QMK firmware, which (I believe) is based on xwhatit's firmware.

## Configuration

Set `DEVICE` to `modelf62` either in `local.mk` (in the project root directory),
or on the command line (i.e., `make DEVICE=modelf62`).

You probably also want to define the key layout with the following defines
(each with either the value 1 for yes, or 0 for no):

``` Make
# local.mk
ISO_LAYOUT = 1
SPLIT_BACKSPACE = 0
SPLIT_RIGHT_SHIFT = 1
```

If `ISO_LAYOUT` is `0`, then ANSI layout is used. If a split is `0`, then that
key will be non-split. I did not implement the short spacebar layouts, you
probably have to make a new directory and provide a different `keymap.c` file
there for that. You can probably otherwise copy this `modelf62` directory.

You can also experiment with the pandrew QMK firmware utility that _may_ be
compatible with this (unknown). For that, add the following to your `local.mk`:

``` Make
DEVICE_FLAGS += -DENABLE_GENERIC_HID_ENDPOINT=1 -DENABLE_DFU_INTERFACE=1
```

## Note

This is a work in progress.
