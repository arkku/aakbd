## GMMK Pro rev1 Keyboard Firmware

This subdirectory of [AAKBD](https://github.com/arkku/aakbd) contains the
GMMK Pro rev1 (ISO and ANSI) keyboard firmware, ported from QMK but using
[tinyusb](../lib/tinyusb/) as the USB engine.

### Hardware Features

* STM32F303 ARM Cortex-M4F microcontroller (72 MHz, 32 KB RAM, 256 KB flash)
* USB HID keyboard via TinyUSB, DFU bootloader via `make dfu`
* AW20216S RGB LED driver (one LED per key + 16 edge LEDs)
* Rotary encoder (counter-clockwise and clockwise rotation mappable, but must
  bo done manually in `macros.c` because it isn't part of the key matrix)

## Setup

Clone the repository and initialize submodules:

```sh
git clone https://github.com/arkku/aakbd
cd aakbd
git submodule update --init
```

Requires `arm-none-eabi-gcc` (newlib-nano) toolchain to build.

## Building

```sh
make DEVICE=gmmkpro1
```

or

``` sh
cd gmmkpro1
make
```

## Configuration

Set options in `gmmkpro1/local.mk`:

``` Make
# Layout
ISO_LAYOUT ?= 1      # 1 = ISO (default), 0 = ANSI

SERIAL ?= AB12345678

CONFIG_FLAGS += -DSERIAL_NUMBER_STRING'"$(SERIAL)"'
```

## Customising

Edit `gmmkpro1/layers.c` to add or modify layers, and `gmmkpro1/macros.c`
for custom macro and hook behaviour. These files are Git-ignored and will
not be overwritten by updates. The `template_layers.c` and `template_macros.c`
files contain examples.

## RGB LEDs

The AW20216S driver is initialised automatically. From `macros.c`, control
LEDs manually:

``` c
rgb_led_set_by_keycode(USB_KEY_CAPS_LOCK, 255, 0, 0);

rgb_led_set(LED_EDGE_LEFT_1, 0xFF, 0xCC, 0x33);
```

The [template_macros.c](template_macros.c) file contains examples of how to use
the RGB LEDs for caps lock indicator, how to make a colour gradient on the side
LEDs, and how to react to keypresses (i.e., key lights up when pressed).

## Encoder

The rotary encoder uses C14/C15 for quadrature. Default action (from
`template_macros.c`): volume up/down on macOS, F21/F20 on Windows layer.
Override `handle_encoder_rotation()` in macros.c to customise.

## Uploading

Put the keyboard in DFU mode (hold the ESC key or encoder button while
plugging in, or use `dfu-util -e`), then:

``` sh
make DEVICE=gmmkpro1 SUDo=sudo dfu
```

Or directly with dfu-util (once in bootloader mode):

``` sh
dfu-util -d 0483:DF11 -a 0 -s 0x08000000:leave -D gmmkpro1.bin
```

## Note

This firmware is an experimental proof of concept. It may contain bugs, and
they are almost certainly my fault and not QMKs or anyone else's.
