## IBM Model FEXT Keyboard Firmware

This subdirectory of [AAKBD](https://github.com/arkku/aakbd) contains an
alternative firmware for the IBM Model M keyboard converted to a model F
(Model MF?) using a "universal" (ATMEGA32U4-based) FEXT controller. This is
largely based on the same firmware as for the brand new Model F keyboards.

## Configuration

Set `DEVICE` to `fext` either in `local.mk` (in the project root directory),
or on the command line (i.e., `make DEVICE=fext`).

You probably also want to define the key layout with the following defines
(each with either the value 1 for yes, or 0 for no):

``` Make
# local.mk
ISO_LAYOUT ?= 1
SPLIT_RIGHT_SHIFT ?= 0
SPLIT_BACKSPACE ?= 0
SPLIT_ENTER ?= 0

DEVICE_FLAGS += -DENABLE_DFU_INTERFACE=1

# To automatically save calibration (by default only via custom macro):
DEVICE_FLAGS += -DCAPSENSE_CAL_AUTOSAVE=1

# To enable solenoid support (config must be done via custom macros):
HAPTIC_ENABLE ?= 1
```

If `ISO_LAYOUT` is `0`, then ANSI layout is used. If a split is `0`, then that
key is not split. Short space splits on the right.

You can also configure `SPLIT_LEFT_SHIFT` and `ISO_ENTER` independently of
the `ISO_LAYOUT` setting, but by default both are controlled by that.

# Uploading

To upload the firmware, you can use the same tools as originally. Once you have
AAKBD firmware installed, and `-DENABLE_DFU_INTERFACE=1` enabled, you can also
use `dfu-util` to enter the bootloader:

``` sh
dfu-util -e
```

## Note

This is a work in progress and currently untested. Any problems are very
likely to be my fault, and not that of QMK or original firmware authors.

I recommend not setting `ENABLE_GENERIC_HID_ENDPOINT` for now if you use
Windows OS, it doesn't seem to work there at the moment.
