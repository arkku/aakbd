## Brand New Model F77 Keyboard Alternative Firmware

This subdirectory of [AAKBD](https://github.com/arkku/aakbd) contains an
alternative firmware for the Brand New Model F77 keyboard, ported from
pandrew's QMK firmware, which is based on xwhatsit's firmware.

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
SPLIT_ENTER ?= 1
RIGHT_BLOCK_TYPE ?= 1           # Only affects the template_layers.c
RIGHT_MODIFIERS_ARE_ARROWS ?= 0 # "

DEVICE_FLAGS += -DENABLE_DFU_INTERFACE=1
DEVICE_FLAGS += -DRIGHT_BLOCK_TYPE=$(RIGHT_BLOCK_TYPE)
DEVICE_FLAGS += -DRIGHT_MODIFIERS_ARE_ARROWS=$(RIGHT_MODIFIERS_ARE_ARROWS)

# To automatically save calibration (by default only via custom macro):
DEVICE_FLAGS += -DCAPSENSE_CAL_AUTOSAVE=1

# To enable solenoid support (config must be done via custom macros):
HAPTIC_ENABLE ?= 1

# If you have added LEDs, define which pins they are on:
DEVICE_FLAGS += -DLED_NUM_LOCK_PIN=B5 -DLED_CAPS_LOCK_PIN=B4
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

## Uploading

To upload the firmware, you can use the same tools as originally. Once you have
AAKBD firmware installed, and `-DENABLE_DFU_INTERFACE=1` enabled, you can also
use `dfu-util` and `dfu-programmer` to automate further:

``` sh
dfu-util -e && sleep 2 && dfu-programmer atmega32u2 erase && dfu-programmer atmega32u2 flash modelf77.hex && dfu-programmer atmega32u2 launch
```

The above is also doable with `make dfu`.

## Calibration

The capacitive sensing matrix will auto-calibrate when powered up. Make sure
you are not holding down any keys when powering up (the calibration takes about
a quarter of a second, so the window is very small).

If you observe phantom keypresses, you may need to adjust the debounce time
in `local.mk`, e.g.:

``` Make
DEBOUNCE = 10
```

If this doesn't work or you are experiencing missed keypresses, it may be that
your specific keyboard differs sufficiently from mine that the threshold offset
is wrong.

You can adjust it in the keyboard's [config.h](config.h) file:

``` c
#define CAPSENSE_CAL_THRESHOLD_OFFSET 20
```

Increasing this offset makes keypresses less sensitive, but making it too
insensitive can cause missed keypresses or require a longer debounce duration
because the lower sensitivity may cause the key to keep bouncing between
pressed and not pressed states more. Lowering this offset makes keypresses more
sensitive, i.e., if keys are not registering at all (and the hardware is fine),
you can try lower values here.

Please report to me if you had to adjust this value – the current values are
determined empirically from my own keyboards and there may be sample variance.

### Experimental Eager Debounce

If you are _not_ experiencing any issues and wish to try to squeeze a couple of
milliseconds more responsiveness out of the keyboard, you can try changing the
debounce algorithm to eager:

``` Make
DEBOUNCE_TYPE = sym_eager_lone_press_g
```

This skips debouncing time entirely when no keys are currently held down
(i.e., no crosstalk) and exactly one new key is pressed. So, in normal typing
when you press one key at a time, the response is faster by the `DEBOUNCE`
duration. However, in my experience this needs significantly longer `DEBOUNCE`
(e.g. 10 ms or more) to avoid phantom presses when hitting the same key
repeatedly (my manual test is to press every key three times rapidly and verify
that exactly three symbols appear, then repeat this a few times per row).

So, the trade-off is lone-key responsiveness vs multi-key responsiveness. To
be honest, it's probably better to stick with the default `sym_defer_g`, which
is also the QMK default and used by a lot of keyboards, but if it the idea of
a few milliseconds extra delay for each keypress annoys you, this can help.

> Note that for gaming, where you typically hold down multiple keys at once,
> this does _not_ help at all, and indeed makes it worse if you have to
> increase the debounce time.

## Note

This is a work in progress. Any problems are very likely to be my fault, and
not that of QMK or original firmware authors.

I recommend not setting `ENABLE_GENERIC_HID_ENDPOINT` for now if you use
Windows OS, it doesn't seem to work there at the moment.

I got tired of maintaining a separate `template_layers.c` that no-one uses,
so the template is now actually my own `layers.c`. It is complicated, so
you will probably want to tweak it unless you are primarily a Mac user who
also uses Windows and also the Dvorak layout with Finnish additions.
