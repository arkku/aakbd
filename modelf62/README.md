## Brand New Model F62 Keyboard Alternative Firmware

This subdirectory of [AAKBD](https://github.com/arkku/aakbd) contains an
alternative firmware for the Brand New Model F62 keyboard, ported from
pandrew's QMK firmware, which (I believe) is based on xwhatit's firmware.

My modifications include:

* Faster scan rate when calibration results in unused pins (less than 0.9 ms
  to scan the whole keyboard, meaning 1000 Hz polling is feasible).
* Optional saving of calibration to EEPROM, resulting in faster startup times
  and recognition of keys held down on start. The theoretical danger here is
  that the calibration could drift so that the recalibration macro is not
  accessible. In this case the only solution would be to flash a firmware that
  disables using saved calibration.
* More options for configuring the keymap out of the box (e.g., split space
  and split enter).

## Configuration

Set `DEVICE` to `modelf62` either in `local.mk` (in the project root directory),
or on the command line (i.e., `make DEVICE=modelf62`).

You probably also want to define the key layout with the following defines
(each with either the value 1 for yes, or 0 for no):

``` Make
# local.mk
ISO_LAYOUT ?= 1
SPLIT_RIGHT_SHIFT ?= 1
SPLIT_BACKSPACE ?= 1
SHORT_SPACE ?= 0
SPLIT_ENTER ?= 0
RIGHT_MODIFIERS_ARE_ARROWS ?= 0 # Only affects layers.c from the template

DEVICE_FLAGS += -DENABLE_DFU_INTERFACE=1
DEVICE_FLAGS += -DRIGHT_MODIFIERS_ARE_ARROWS=$(RIGHT_MODIFIERS_ARE_ARROWS)

# To automatically save calibration (by default only via custom macro):
DEVICE_FLAGS += -DCAPSENSE_CAL_AUTOSAVE=1
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

The `template_layers.c` goes together with `modelf77/template_macros.c`.

# Uploading

To upload the firmware, you can use the same tools as originally. Once you have
AAKBD firmware installed, and `-DENABLE_DFU_INTERFACE=1` enabled, you can also
use `dfu-util` and `dfu-programmer` to automate further:

``` sh
dfu-util -e && sleep 2 && dfu-programmer atmega32u2 erase && dfu-programmer atmega32u2 flash modelf62.hex && dfu-programmer atmega32u2 launch
```

## Note

This is a work in progress. Any problems are very likely to be my fault, and
not that of QMK or original firmware authors.

I recommend not setting `ENABLE_GENERIC_HID_ENDPOINT` for now if you use
Windows OS, it doesn't seem to work there at the moment.
