## Brand New Model f50 Keyboard Alternative Firmware

This subdirectory of [AAKBD](https://github.com/arkku/aakbd) contains an
alternative firmware for the Brand New Model F50 keyboard. The underlying code
is ported from pandrew's QMK firmware, which (I believe) is based on xwhatit's
firmware. The F50-specific configuration is from NathanA's VIAL firmware, which
is currently what the keyboard ships with.

My modifications include:

* Faster scan rate when calibration results in unused pins (less than 0.9 ms
  to scan the whole keyboard, meaning 1000 Hz polling is feasible).
* Optional saving of calibration to EEPROM, resulting in faster startup times
  and recognition of keys held down on start. The theoretical danger here is
  that the calibration could drift so that the recalibration macro is not
  accessible. In this case the only solution would be to flash a firmware that
  disables using saved calibration (you can run `make ERASE_CALIBRATION=1` to
  build a firmware that erases the calibration on start).
* More options for configuring the keymap out of the box (e.g., split space
  and split enter).

## Layout

Since this keyboard doesn't come with keycaps, it doesn't have a definitive
layout. There aren't enough keys to do a TKL QWERTY layout, so I decided to
just map the keys to something that is easy to remember when remapping in
`layers.c`:

The two leftmost 3×5 blocks:

```
F1  F5  F9   F13 F17 F21
F2  F6  F10  F14 F18 F22
F3  F7  F11  F15 F19 F23
F4  F8  F12  F16 F20 F24
KPA KPB KPC  KPD KPE KPF
```

The `KPA` through `KPF` in the bottom row correspond to _numpad_ A–F keys,
which are probably unsupported and I have never seen a keyboard that actually
has them. They are just there so that it's easy to remember what to remap.

The righmost 4×5 block is a standard PC numpad. The keyboard ships with all
springs installed, so if you wish to use the standard numpad layout you need
to remove the springs from the bottom barrels of the Plus and Enter keys, as
well as from the right barrel of the 0/Ins key. To use individual keys, define
the `SPLIT_PAD_*` options in `local.mk` (see below).

The splits change the layout as follows:

* The lower half of Plus will become a Backspace key
* The upper half of Enter will become a numpad Equals key
* The right half of 0/Ins will become a 00 key (probably unsupported, but it's
  possible to macro it)

## Configuration

Set `DEVICE` to `modelf50` either in `local.mk` (in the project root directory),
or on the command line (i.e., `make DEVICE=modelf50`).

You probably also want to define the key layout with the following defines
(each with either the value 1 for yes, or 0 for no):

``` Make
# local.mk
SPLIT_PAD_ENTER ?= 0
SPLIT_PAD_PLUS ?= 0
SPLIT_PAD_ZERO ?= 0

# To enable the DFU programming interface
DEVICE_FLAGS += -DENABLE_DFU_INTERFACE=1

# To automatically save calibration (by default only via custom macro):
DEVICE_FLAGS += -DCAPSENSE_CAL_AUTOSAVE=1

# To enable solenoid support (config must be done via custom macros):
HAPTIC_ENABLE ?= 1
```

The `template_layers.c` goes together with `modelf77/template_macros.c`.

# Uploading

To upload the firmware, you can use the same tools as originally. Once you have
AAKBD firmware installed, and `-DENABLE_DFU_INTERFACE=1` enabled, you can also
use `dfu-util` and `dfu-programmer` to automate further:

``` sh
dfu-util -e && sleep 2 && dfu-programmer atmega32u2 erase && dfu-programmer atmega32u2 flash modelf50.hex && dfu-programmer atmega32u2 launch
```

## Note

This is a work in progress. Any problems are very likely to be my fault, and
not that of QMK or original firmware authors.
