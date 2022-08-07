## Brand New Model F77 Keyboard Alternative Firmware

This subdirectory of [AAKBD](https://github.com/arkku/aakbd) contains an
alternative firmware for the Brand New Model F77 keyboard, ported from
pandrew's QMK firmware, which (I believe) is based on xwhatit's firmware.

## Configuration

Set `DEVICE` to `modelf77` either in `local.mk` (in the project root directory),
or on the command line (i.e., `make DEVICE=modelf77`).

You probably also want to define the key layout with the following defines
(each with either the value 1 for yes, or 0 for no):

``` Make
# local.mk
ISO_LAYOUT ?= 1
SPLIT_RIGHT_SHIFT ?= 1
SPLIT_BACKSPACE ?= 0
SHORT_SPACE ?= 0
SPLIT_ENTER ?= 0
RIGHT_MODIFIERS_ARE_ARROWS ?= 0 # Only affects layers.c from the template
RIGHT_BLOCK_IS_NUMPAD ?= 1      # "

DEVICE_FLAGS += -DENABLE_DFU_INTERFACE=1
DEVICE_FLAGS += -DRIGHT_BLOCK_IS_NUMPAD=$(RIGHT_BLOCK_IS_NUMPAD)
DEVICE_FLAGS += -DRIGHT_MODIFIERS_ARE_ARROWS=$(RIGHT_MODIFIERS_ARE_ARROWS)
```

If `ISO_LAYOUT` is `0`, then ANSI layout is used. If a split is `0`, then that
key is not split. Short space splits on the right.

You can also configure `SPLIT_LEFT_SHIFT` and `ISO_ENTER` independently of
the `ISO_LAYOUT` setting, but by default both are controlled by that.

You can also experiment with the pandrew QMK firmware utility that _may_ be
compatible with this (unknown). For that, add the following to your `local.mk`:

``` Make
DEVICE_FLAGS += -DENABLE_GENERIC_HID_ENDPOINT=1
```

# Uploading

To upload the firmware, you can use the same tools as originally. Once you have
AAKBD firmware installed, and `-DENABLE_DFU_INTERFACE=1` enabled, you can also
use `dfu-util` and `dfu-programmer` to automate further:

``` sh
dfu-util -e && sleep 2 && dfu-programmer atmega32u2 erase && dfu-programmer atmega32u2 flash modelf77.hex && dfu-programmer atmega32u2 launch
```

## Note

This is a work in progress. Any problems are very likely to be my fault, and
not that of QMK or original firmware authors.

I recommend not setting `ENABLE_GENERIC_HID_ENDPOINT` for now if you use
Windows OS, it doesn't seem to work there at the moment.
