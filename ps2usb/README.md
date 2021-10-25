## PS/2 to USB Keyboard Converter

This subdirectory of [AAKBD](https://github.com/arkku/aakbd) contains the
components specific to the PS/2 to USB keyboard converter, `ps2usb`. See the
[main README](../README.md) for details about the entire USB keyboard
implementation.

## Hardware

The implementation has been tested with ATMEGA32U4 running at 5V with a
16 MHz crystal oscillator, in particular with an Arduino Pro Micro clone.

The pinout is as follows:

* PS/2 CLK – pin D0
* PS/2 DATA – pin D1
* status LED – pin B0
* error LED – pin D5

This also corresponds to an Arduino Pro Micro with the existing LEDs already
in place. On the Pro Micro, PS/2 CLK is "pin 3" and PS/2 DATA is "pin 2",
in their silly renumbering scheme. You may add an external pull-up resistor
between each of those pins and 5V (RAW), for example, 4.7KΩ or more. However,
the internal pull-ups are also enabled and in my experiments no keyboard
I tested needed the external resistors.

Connecting the LEDs is optional, just as long as nothing breaks if those
pins are driven high (i.e., unconnected is fine).

No other components are needed, apart from the physical connectors.

## Configuration

Set `DEVICE` to `ps2usb` either in `local.mk` (in the project root directory),
or on the command line (i.e., `make DEVICE=ps2usb`).

You can have two complementary `local.mk` files: one the the project root
directory, and `ps2usb/local.mk` specific to this PS/2 to USB converter.

For example:

``` Make
# root local.mk
DEVICE = ps2usb

MAX_KEY_ROLLOVER = 7

CONFIG_FLAGS =  \
	       -DUSB_MAX_KEY_ROLLOVER=$(MAX_KEY_ROLLOVER) \
	       -DUSB_VENDOR_ID=$(VENDOR_ID) \
	       -DUSB_PRODUCT_ID=$(PRODUCT_ID) \
	       -DMANUFACTURER_STRING='$(MANUFACTURER)' \
	       -DPRODUCT_STRING='$(PRODUCT)' \
	       -DKEYBOARD_POLL_INTERVAL_MS=2 \
	       -DENABLE_BOOTLOADER_SHORTCUT=1
```

``` Make
# ps2usb/local.mk

VENDOR_ID = 0x04B3U
PRODUCT_ID = 0x3020U

MANUFACTURER = "IBM Corp."
PRODUCT = "Model M Keyboard"
```

> This example configuration misuses an IBM keyboard's vendor and product id.
> However, is it really misuse if the actual keyboard _is_ made by IBM? ;>
> In any case, unauthorised use of ids must be limited to personal DIY
> projects, not any released products!

After creating this file, run `make` once to create
[layers.c](template_layers.c) and [macros.c](template_macros.c) from the
included templates. These files are created in this `ps2usb` directory.
If you wish, you can move one or both of them to the project root directory.

You may then edit `layers.c` and `macros.c` to configure the keyboard. Note
that while the template provides some examples, it is not enabled due to
`LAYER_COUNT` being defined as `0` in `layers.c`. To enable your layers, you
must increase `LAYER_COUNT` to the number of your highest layer. Layer numbers
start from 1 (layer 0 is the implicit "default layer", i.e., for this PS/2 to
USB converter it maps each PS/2 keycode to the corresponding USB keycode).

Recompile by running `make` again.
