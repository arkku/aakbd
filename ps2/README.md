# AAKBD PS/2 Support

## PS/2 Host Support

The [ps2usb](../ps2usb) firmware acts as a PS/2 host, i.e., the "computer",
and translates a PS/2 keyboard to USB. See the [README.md](../ps2usb/README.md)
in that directory for details.

## PS/2 Device Support

AAKBD keyboards running on AVR microcontrollers (e.g., ATMEGA32U2) can now
have a PS/2 keyboard *output*. It is mutually exclusive with the default USB
keyboard output - the presence of a PS/2 host is autodetected on start and then
either USB or PS/2 is enabled, never both at once. (Currently, at least.)

Like the PS/2 to USB transition period devices of old, it is possible to share
the same USB connector for dual use with a _passive_ USB to PS/2 adapter. The
early USB mice in particular used to come with that green dongle, I at least
seem to have plenty of them around.

### Configuration and Hardware

The Make option `ENABLE_PS2_DEVICE=1` (e.g., in the `local.mk` file) enables
PS/2 output support, but further configuration is needed (there are no default
values because available free pins differ between keyboards, but e.g., on the
xwhatsit-based keyboards pins B4 to B7 should be free (but check that they are
not elsewhere configured for LEDs or solenoid).

You must find two free GPIO pins _on the same port_ (the letter before the
pin name), e.g., `B6` and `B7` are both on port B. Connect these to the PS/2
connector's clock and data pins, then set the appropriate configuration in
`local.mk`, for example:

``` Make
# local.mk
ENABLE_PS2_DEVICE = 1

ifeq (1,$(ENABLE_PS2_DEVICE))
# B6 = PS/2 CLK, B7 = PS/2 DATA
DEVICE_FLAGS += -DPS2_PORT=B -DPS2_DATA_PIN=7 -DPS2_CLK_PIN=6
endif
```

Make sure you are not using these pins for anything else (e.g., LEDs or
solenoid control)! This includes checking default values for your keyboard
type (`config.h`, `*.mk`, etc.).

You also need to connect the PS/2 power and ground pins such that they will
power the microcontroller in the absence of USB. (Again, USB must not be
plugged in at the same time as PS/2, it's either or. Unless you hack it
somehow, in which case let me know.)

Do not put any pull-up resistors on the PS/2 lines (they are on the
host/computer side already and the autodetection is based on that).

You can put a low-value resistor in series with each PS/2 line (e.g., 22 Ω)
for protection, but it's not technically needed. I do recommend it if sharing
the same connector with USB D+/D- pins: the USB connector already has 22 Ω
pins on both lines, so if you add the same on PS/2 there will be a total of
44 Ω resistance between the USB and PS/2 pins, which should protect from any
momentary shorts. If you are worried, you can use even higher value resistors
between the PS/2 pins and the connector (but not between USB pins and the
USB connector, there 22 Ω is mandated and should be there already). Probably
even up to 470 Ω would work on PS/2 and be very safe on AVR against shorts, but
I just put the 22 Ω which definitely works and maintains good voltage levels.
(The strong pull-up of the PS/2 host and the series resistor form a voltage
divider, so too strong a series resistor might cause logic levels to be out
of spec and not recognized correctly.)


```
# PS/2 male connector (cable, looking into the connector from outside)

 Clock - 5 || 6 - NC
  GND - 3  ||  4 - +5V
   Data - 1  2 - NC

# PS/2 female connector (socket, looking into the socket from outside)

    NC - 6 || 5 - Clock
  +5V - 4  ||  3 - GND
     NC - 2  1 - Data
```

With this connection the keyboard should autodetect the host's pull-up on the
PS/2 pins when a computer is connected to the PS/2 port, which will make the
keyboard automatically choose PS/2 output and disable USB. If the pins are just
floating when not connected to PS/2, there is a risk of incorrectly going into
PS/2 mode when actually connected to USB. In that case, if you are not sharing
the USB connector with the PS/2 pins, you can add a physical switch to disable
PS/2 mode by shorting one or both PS/2 pins to ground when not in use. (This
can't be done when sharing the USB pins, but in my experience it's not a
problem in that case. If it is, let me know and I can add support for a PS/2
disable pin.)

This was developed and tested on a Model F77 keyboard, but in theory it should
work on any AVR-based keyboard. The AVR-specific code is quite well abstracted
so porting to ARM-based keyboards shouldn't be hard as such (only GPIO and
a hardware timer is needed), but electrically it might be more challenging if
the pins are not directly 5V tolerant.

#### PS/2 and USB on the Same Connector

It's possible to use PS/2 and USB on the same connector. This requires you to
use a _passive_ USB to PS/2 adapter (these used to come with USB keyboards and
mice in the late 90's and early 00's when USB was not yet quite so universal).
The connection is PS/2 Clock to USB Data+ and PS/2 Data to USB Data-. The USB
data lines should have 22 Ω resistors in series already. I recommend connecting
the PS/2 pins to the "outside" (towards the computer, not the keyboard) of
these resistors (you can optionally add another pair of similar resistors in
series with the PS/2 lines as well).

| PS/2 pin  | USB pin |
| --------- | ------- |
| 4 +5V     | 1 +5V   |
| 1 Data    | 2 Data- |
| 5 Clock   | 3 Data+ |
| 3 GND     | 4 GND   |

This should work because the PS/2 host pulls the pins up by default, and USB
pulls them down by default, so in theory we should always be able to detect
which one is connected and choose the correct protocol on boot. There are many
commercial mice and keyboards that support this same auto-selection.

**WARNING**: Do not jump to bootloader when connected to PS/2 in this way.
Now, in theory it should be harmless, but I can't know what the bootloader
decides to do on the shared pins, so better not try. The bootloader is purely
USB-based anyway, so it would do no good when plugged into PS/2. The firmware
does try to disable all jump to bootloader commands when PS/2 is active,
but it can't disable any kind of direct reset to bootloader (like bootmagic
"hold down Esc when plugged in", or a reset button / pad on the circuit board).
It's probably not a disaster if this happens by mistake, I did it several times
while developing this and nothing bad happened, but you might have a different
bootloader. Having the resistors in between the USB and PS/2 pins also helps
in case something tries to short a high pin to a low pin!

**IMPORTANT**: As always, **use at your own risk only**, no matter the
configuration or safety precautions there might be! There is absolutely
no warranty, express or implied! Just because I'm happy to plug this into my
MacBook Pro, doesn't mean you should risk yours. It's better to use a dedicated
PS/2 connector.

### Scancode Sets

Three scancode keysets are defined for PS/2 keyboards. The default is 2, and
it is always enabled. However, it has some quirks for historical reasons,
e.g., the Pause/Break key emits only a one-time press and can't be "held down"
in set 2 mode.

The scancode set 1 is basically just a "translated" set 2 and has the same
issues, and maybe some other ones on top of that.

The scancode set 3 is actually sensible and has no special cases, and allows
configuring repeat and typematic (make/break) behaviour either globally or
per key. However, it never caught on because many keyboards implement it poorly
or not at all, and thus operating systems are wary of using it (e.g., Microsoft
doesn't use it and has even removed the specs from their PS/2 documentation
already in the late 1990's).

(My `ps2usb` firmware in this same project does attempt to use set 3, and it
does work with quality keyboards like the IBM Model M.)

Anyway, this PS/2 output (device mode) supports all three scancode sets.
However, it is very likely that you don't actually need all three, e.g., for
use with any Microsoft operating system. Thus, it is possible to disable
set 1 and/or set 3. Disabling set 3 saves about 70 bytes of RAM, whereas
disabling set 1 saves only about 1 KB of firmware (ROM) size but no RAM. Set 2
can't be disabled because it's the default.

If you are low on space, you can disable the unnecessary sets with the flags
`ENABLE_PS2_DEVICE_SET_1` and `ENABLE_PS2_DEVICE_SET_3` (set either or both to
`0`).

``` Make
DEVICE_FLAGS += -DENABLE_PS2_DEVICE_SET_1=0 -DENABLE_PS2_DEVICE_SET_3=0
```

### PS/2 Output Enable Pin

If the PS/2 autodetection doesn't work, you can use a pin to force-enable PS/2.
This can be wired to a physical switch or such. The pin is **active low**,
i.e., short it to ground and it will force PS/2 mode. The pin needs to
be on the same port as the other PS/2 pins, and is controlled by defining
`PS2_ENABLE_PIN`. For example `-DPS2_ENABLE_PIN=5` with `-DPS2_PORT=B` means
the enable pin will be `B5`. (And so it begins.)

``` Make
DEVICE_FLAGS += -DPS2_ENABLE_PIN=5
```

### PS/2 Output Status Pin

It's possible to configure another pin on the same port as the other PS/2 pins
to act as a PS/2 output status indicator. It will pulse on start, and then go
high if PS/2 mode is enabled, low otherwise. For user-visible indicators it's
probably better to control the indicator from `macros.c` hooks (see below),
this was meant for debugging with a logic analyzer so I could see the exact
moment when the autodetection code starts running.

``` Make
DEVICE_FLAGS += -DPS2_STATUS_PIN=4
```

### Special Configuration for PS/2 Keyboard Mode

You may wish to have a different setup (e.g., key mappings) in USB vs PS/2
mode (especially as host computer operating system detection only works in
USB mode). You can check whether PS/2 output is initialized (this means the
keyboard has chosen PS/2 keyboard mode, it will not switch back to USB again)
in `macros.c`, e.g., in the `handle_reset()` function:

``` c
#if ENABLE_PS2_DEVICE
#include "ps2_output.h"
#endif

static inline void handle_reset(void) {
#if ENABLE_PS2_DEVICE
    if (ps2_output_is_initialized()) {
        enable_layer(PS2_LAYER);
    }
#endif
}
```

It would actually be quite possible to implement host operating system
detection for PS/2 because they will give different commands in different
order, but I am not sure of the usefulness because I suspect that most PS/2
users specifically intend to use it on vintage computers. On modern computers
USB is the better choice (trust me, I have implemented both in this firmware).
