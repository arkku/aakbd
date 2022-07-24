## AAKBD

AAKBD is a USB keyboard implementation for AVR (e.g., ATMEGA32U4, ATMEGA32U2)
devices. One of the A's probably stands for Arbitrary, since it is designed to
make it simple to add arbitrary C code to be run in response to keypresses.
Features supported:

* remapping keys with an easy syntax (e.g., `[KEY(CAPS_LOCK)] = CMD_OR(ESC)`)
* layers (i.e., overlays that change the key map below them) - up to 31
  layers can be defined, not counting the default mapping which is always there
* combined modifiers (i.e., keys that work as multiple modifiers, like
  <kbd>Ctrl</kbd> <kbd>Alt</kbd> <kbd>Shift</kbd> <kbd>Cmd</kbd> all in
  one key)
* hold/tap modifier keys (i.e., a key that works as a modifier, such as
  <kbd>Ctrl</kbd>, when held down but sends a keypress, such as <kbd>Esc</kbd>,
  if tapped)
* hold/tap layer toggles (i.e., a key that toggles a layer when held down,
  but sends a keypress if tapped)
* modifier or layer keys (i.e., a key that works as a modifier when held
  down, but toggles a layer if tapped)
* keylock key (i.e., a key that locks _any_ other key down until either that
  key or the keylock key is pressed again)
* simulated Apple Fn key (if you are willing to pretend to be an Apple device,
  i.e., this is for personal use only)
* up to 128 arbitrary macros _per physical key_ with 1 byte of automatically
  saved state for each held key (and of course each macro can save and use any
  other state as well)
* hooks for easily overriding the keymap and/or layer system, with
  1 byte of automatically saved state for each held key (and, again, each
  hook is an arbitrary program that can save and use any other state info)
* simulated typing in both QWERTY and Dvorak layouts (i.e., you can `fprintf`
  text and the keyboard will simulate typing it, which basically allows using
  a text editor as a debug console)
* DFU-compatible reset (i.e., you can enter the bootloader for firmware update
  with a DFU_DETACH request, e.g., with `dfu-util -e`, and if you also use a
  DFU bootloader you can upload the firmware with that as well)
* USB boot protocol compatibility (including with a BIOS that does not request
  it)
* more than 6 keys + modifiers rollover without driver support (theoretically
  arbitrary rollover configured at compile time, but let's not call it NKRO
  since you almost certainly don't need or want that)

Now, that being said, for the regular user it may be better to choose one of
the established implementations with easy-to-use tools. These include
[QMK](https://github.com/qmk/qmk_firmware) and
[TMK](https://github.com/tmk/tmk_keyboard). However, for a more technically
inclined person, _AAKBD_ may be more straightforward. In particular, the
key maps (layers) are based on changing the default, named, keys rather than
on a positional matrix. So, for any keyboard that has some kind of natural
naming/mapping for the keys, remapping is simply a listing of the differences,
rather than a set of complete layers with all positions listed.

Currently this repository contains two implementations:
* A PS/2 to USB keyboard converter (which I made to convert an IBM Model
  M keyboard to USB, but it should work with any PS/2 keyboard)
* An alternative firmware for the OG [ErgoDox Ez](https://ergodox-ez.com)
  (which I happen to have from an old job, and decided to port as a proof of
  concept that this engine is reusable)

~ [Kimmo Kulovesi](https://arkku.dev/), 2021-10-10

## Hardware

The implementation has been tested with ATMEGA32U4 running at 5V with a
16 MHz crystal oscillator. The physical keyboard needs to be connected to
something other than USB, but apart from that there is no additional hardware
required beyond the oscillator and components required for the USB connector.
It is probably simplest to buy a pre-made breakout board with these components.

I have also tested with an ATMEGA32U2 that the keyboard shows up in the list
of USB devices, but I have not actually hooked up a keyboard to one. Porting
to other AVR devices should be relatively simple, and most of that code is
in [avrusb.h](avrusb.h).

An ISP programming device is needed unless the microcontroller is already set
up with a bootloader.

See the [PS/2 to USB converter README](ps2usb/README.md) for details about that
specific use.

For ErgoDox Ez, you need the original version of the keyboard. It contains an
ATMEGA32U4 using the Halfkay bootloader. You can simply flash the hex file
after compiling with `DEVICE=ergodox`. See the
[ErgoDox Ez firmware README](ergodox/README.md) for details.

## Software Requirements

For building, `avr-gcc`, `avr-libc`, GNU `make` and a shell to run it in are
required. For flashing the firmware, `avrdude` or some other program that can
upload it to the microcontroller. For resetting the device into bootloader
without key bindings (e.g., if you break your key bindings), it may be helpful
to use `dfu-util`.

## Configuration

It is recommended to create a file called `local.mk` where you set up the
configuration. The various options are documented mainly in
[usbkbd_config.h](usbkbd_config.h). An example local configuration might be:

``` Make
DEVICE = ps2usb

VENDOR_ID = 0x04B3U
PRODUCT_ID = 0x3020U

MANUFACTURER = "IBM Corp."
PRODUCT = "Model M Keyboard"

MAX_KEY_ROLLOVER = 7
POLL_INTERVAL = 2

CONFIG_FLAGS =  \
	       -DUSB_MAX_KEY_ROLLOVER=$(MAX_KEY_ROLLOVER) \
	       -DUSB_VENDOR_ID=$(VENDOR_ID) \
	       -DUSB_PRODUCT_ID=$(PRODUCT_ID) \
	       -DMANUFACTURER_STRING='$(MANUFACTURER)' \
	       -DPRODUCT_STRING='$(PRODUCT)' \
	       -DKEYBOARD_POLL_INTERVAL_MS=$(POLL_INTERVAL) \
	       -DENABLE_BOOTLOADER_SHORTCUT=1
```

> This example configuration misuses an IBM keyboard's vendor and product id.
> However, is it really misuse if the actual keyboard _is_ made by IBM? ;>
> In any case, unauthorised use of vendor ids must be limited to personal DIY
> projects, not used in any released products!

After creating this file, run `make` once to create
[layers.c](template_layers.c) and [macros.c](template_macros.c) from the
included templates. Note that these files reside in the target device
directory, e.g., `ps2usb/layers.c` and `ps2usb/macros.c` by default. If you
wish, you can move one or both of them to the project root directory in order
to share them between multiple keyboards.

You may then edit `layers.c` and `macros.c` to configure the keyboard. Note
that while the template provides some examples, it is not enabled due to
`LAYER_COUNT` being defined as `0` in `layers.c`. To enable your layers, you
must increase `LAYER_COUNT` to the number of your highest layer. Layer numbers
start from 1 (layer 0 is the implicit "default layer", e.g., for the PS/2 to
USB converter it maps each PS/2 keycode to the corresponding USB keycode).

Recompile by running `make` again.

Note that you can place an additional `local.mk` inside the `DEVICE` directory,
i.e., `ps2usb/local.mk`. Although, if you don't wish to do other customisation
than to set `DEVICE`, you can just run `make` from the device subdirectory,
i.e., running `make` in `ps2usb` will make the PS/2 to USB converter.

## Flashing Firmware

You can upload the produced firmware (such as `ps2usb.hex`) to the
microcontroller by either an ISP programmer device or by means of an existing
bootloader. I recommend using a bootloader, since that allows you to reset the
device with a keyboard shortcut and simply upload with:

``` sh
make upload PORT=/dev/ttyUSB0
```

Replace the port with the USB port of your device (e.g., on Mac it is
likely to be something like `PORT=/dev/cu.usbmodem14101`). You can also
specify `UPLOAD_PROTOCOL=avr109` in case it differs from the default
(which is `avr109`).

To intially get your device into bootloader mode, you need to short the reset
(`RST`) pin to ground. If the device already has a program on it, the time it
stays in the bootloader after reset may be extremely short. Reset it twice in
a row to extend the time.

With an ISP, hook up the device to the programmer and instead use:

``` sh
make burn BURNER=stk500v2 PORT=/dev/ttyUSB0 BPS=115200
```

The `PORT` and `BPS` parameters may be omitted when not needed, such as for
the AVR Dragon.

You can also use `make fuses` target with the same arguments as for `make
burn`. This programs the fuses on the device for 16 MHz crystal oscillator.

### Updating the Firmware

Once you have flashed the firmware onto the device once and have the keyboard
working, updating the firmware is much simpler than the initial upload
(provided that a bootloader is installed).

If your keyboard has both left and right Shift keys and as well as
Scroll Lock _mapped_, you can enter the bootloader by pressing
<kbd>Left Shift</kbd> <kbd>Scroll Lock</kbd> <kbd>Right Shift</kbd> in that
order. If you don't have these keys, or have remapped them in ways that make
this shortcut inconvenient, you can remap a key to `EXT(ENTER_BOOTLOADER)`
in one of your layers and use that instead. Or you can create a custom macro
that calls `jump_to_bootloader()` based on whatever conditions you like.

If you have `ENABLE_DFU_INTERFACE` set to `1`, you can also enter the
bootloader by issuing a DFU_DETACH request. One way to do this is by using
[dfu-util](http://dfu-util.sourceforge.net) and the command `dfu-util -e`.
It doesn't actually matter whether your bootloader supports DFU: this will
still enter the bootloader. Using a DFU bootloader just enables you to upload
the firmware via DFU as well.

## Key Mapping

### Key Map Files

The basic key mapping happens in [layers.c](template_layers.c). In the
beginning of the file you should define the number of layers to activate:

``` C
#define LAYER_COUNT 3
```

Defining `LAYER_COUNT` as `3` would enable three layers: 1, 2, and 3. You can
also define a `DEFAULT_BASE_LAYER`, but it is almost certainly best to keep
that as `1` (the default default).

Each layer is then defined with the `DEFINE_LAYER(number)` macro, e.g.:

``` C
DEFINE_LAYER(1) {
    [KEY(CAPS_LOCK)] = CMD_OR(ESC),
};
```

This would define layer 1 as otherwise pass-through (see below), but remap
the physical Caps Lock key to act as <kbd>Command</kbd> when held down, or
to send an <kbd>Esc</kbd> keystroke if only tapped.

There is also [macros.c](template_macros.c) that can be used for more
elaborate customisation (see below).

You can use the C pre-processor to `#define` names for your layers, etc.
While you can otherwise put arbitrary C code in this file, I strongly recommend
to instead put it in `macros.c`. Also note that the layers must be stored in
`PROGMEM`, since they do not fit in the microcontroller's RAM. The
`DEFINE_LAYER` macro does this for you, do not try to circumvent it.

If something doesn't work, the first thing to check is that `LAYER_COUNT` is
high enough, and `DEFAULT_BASE_LAYER` is 1 (or not defined).

### Layers

You can think of layers as "sheets" placed on top of the keys, in numerical
order. The highest-numbered active layer takes precedence, but if there is
no mapping for a key in that layer, that position is "transparent" and passes
through to the next-highest-numbered active layer. If there is no mapping in
any of the active layers, the default mapping is used.

There is an additional concept of "base layer". Simply put, the base layer
is always considered active (even if toggled by modifiers) and at the same
time _no layer below the base layer_ is considered active. If the base layer
is left at the default of 1, this simply means that layer 1 can't be turned
off (except by changing the base layer to 0, which is allowed but also
irreversible since layer 0 can't contain custom mappings). However, if you map
a key to change the base layer to a higher number, doing so disables all
layers below it. This can be useful if you have multiple toggleable layers but
also wish to have a key that replaces them all with a completely different
mapping while preserving the state of those toggles upon returning to the
normal base layer.

The syntax for defining a layer is `DEFINE_LAYER(number) { … };` with all
the key mappings inside the `{ }`. If you wish to define an empty layer for
whatever reason, you can use:

``` C
DEFINE_LAYER(2) {
    PASS,
};
```

That layer will then have no effect, since all keys will pass through it.
If you instead wish to make a layer that _blocks_ keys from having any
effect, you can use:

``` C
DEFINE_LAYER(2) {
    DISABLE_ALL_KEYS_NOT_DEFINED_BELOW,
};
```

This is shorthand for mapping all keys to the special keycode `NONE`. You can
(and almost certainly should) put exceptions to that below the line with
`DISABLE_ALL_KEYS_NOT_DEFINED_BELOW`. An example use case for this might be to
make a numpad layer for a tenkeyless (or shorter) keyboard and disable the
other letter keys to avoid accidental keypresses outside the numpad area.

### Keycodes and Key Maps

At the root of everything is the default mapping, i.e., the "physical key".
All remappings are based on the _physical_ keys. Importantly, if you have
remapped a key in one layer, it does not change the mappings of
other layers. For example, if you remap the physical <kbd>Caps Lock</kbd> to
<kbd>Ctrl</kbd> in one layer, mapping `[KEY(CAPS_LOCK)]` in another layer
will still affect that same physical key, and likewise mapping
`[KEY(CTRL)]` in another layer does not affect the remapped <kbd>Caps
Lock</kbd> even if it now acts as a <kbd>Ctrl</kbd>.

The syntax for mapping inside a layer definition is:

``` C
    [KEY(physical_key)] = remapped_key,
```

The list of physical keys depends on the physical keyboard. Every
implementation should _uniquely_ identify each physical key - this is important
for mappings to work correctly. If you want multiple keys to send the same
keycode, each physical key should be mapped to the same keycode in layers,
_not_ in the physical keyboard implementation. For the PS/2 to USB converter,
the physical key mappings are as per the PS/2 keycodes, which can be seen in
[ps2usb_keys.c](ps2usb/ps2usb_keys.c).

The list of available USB keycodes (the "plain" keys) is in
[usb_keys.h](usb_keys.h). For the plain keys you can either use the
key names with `USB_KEY_X` etc. from that file, or with the helper macro
`KEY(X)`, which adds the `USB_KEY_` prefix for you. It is a matter of
preference which you use.

It should also be understood that the term "modifiers" here refers to
the left and right Shift, Ctrl, Alt, and Cmd/Win/Meta/GUI keys. These are
technically keys like any other, and can be used as such in every single-key
context (i.e., remapped freely, even to non-modifier keys, and likewise
non-modifier keys can be remapped to become true modifier keys). However,
for extended keycodes (see below), these modifiers can also be applied in
combination with each other or with non-modifier keys in many contexts. Such
combinations are only usable as the remapping _target_, but the "source" must
always be only a single physical key.

For the remapped keys _only_, extended keycodes can be used. These are not
actual USB keys, but rather special keycodes that trigger certain
functionality of this program itself. They are defined in
[keycodes.h](keycodes.h).

Examples of extended keycodes include:

* modifiers + key – a combination of modifiers and plain key, e.g., `SHIFT(X)`,
   `CTRL(X)`, `ALT(X)`, `CMD(X)`, `CTRL_ALT(DEL)`
* multi-modifiers – a combination of modifiers only, e.g., `CTRL_ALT_SHIFT(CMD)`
  aka `KEY_HYPER`
* tap/hold key/modifier combos – acts as a modifier key when held down,
  but send another keystroke if tapped without pressing other keys, e.g.,
  `CTRL_OR(ESC)`
* layer on/off toggles – toggles a layer when released, e.g., `LAYER_TOGGLE(2)`
* layer hold on/off toggles – toggles a layer while held, e.g.,
  `LAYER_TOGGLE_HOLD(2)`
* "sticky" layer hold on/off toggles – toggles a layer while holding, but
  it remains toggled if no other key is pressed while holding, e.g.,
  `LAYER_TOGGLE_STICKY(2)`
* modifier and layer hold on/off toggle – a combination modifiers and layer
  toggle while holding, e.g., `CMD_SHIFT_AND_LAYER(2)`
* modifier on hold or layer on/off toggle on tap – acts as a modifier when held
  down but toggles layer if no key was pressed while held, e.g.,
  `CTRL_ALT_CMD_SHIFT_OR_LAYER(2)`
* layer on/off toggle hold or plain key on tap – toggles a a layer while
  holding, but instead sends a plain key press (no modifiers) if no other
  key was pressed while held, e.g., `LAYER_OR_PLAIN_KEY(2, KEY(ESC))`
* base layer change – changes the base layer, effectively disabling all layers
  with a lower number (but remembering their activation state), e.g.,
  `LAYER_SET_BASE(5)` (note that the new base layer should have a mapping
  such as `LAYER_SET_BASE(1)` to undo this or the change will be permanent)
* "exact" modifiers, i.e., modifier combinations that set the active set of
  modifiers to exactly that, removing any other modifiers present, e.g.,
  `EXACTLY_CMD` can be useful if you wish to have something like
  <kbd>Shift</kbd> <kbd>Ctrl</kbd> work as a <kbd>Command</kbd> key without
  including the Shift or Ctrl
* extended keys – predefined virtual keys, e.g., `EXT(ENTER_BOOTLOADER)`,
  `EXT(RESET_LAYERS)` and `EXT(HYPER)`
* the keylock key – actually another extended key, `EXT(KEYLOCK)`: press this
  key and then some other key to lock that other key down until either that
  same physical key or the keylock key is pressed again
* macros – macros defined in `layers.c` and `macros.c` (see below)

This is not a complete list – there are many more layer commands, including
enable or disable instead of hold, or to enable only one exact layer while
disabling others, etc. See [keycodes.h](keycodes.h) for details.

> As sidenote, the bit structure of the extended keycode is very similar to
> that used by QMK. However, this is not by design and the keycodes are not
> directly compatible – it just so happens that implementing a similar set of
> features in 16 bits dictates exactly how many bits are available for each
> field of the keycode. =)

## Macros

Macro keys in this context are essentially just a set of 128 reserved keycode
values, that are passed as an argument to a C function, which may do whatever
it wishes for each macro. The function also receives the keycode of the
physical key triggering the macro, which means that there can easily be
128 macros _per physical key_. The function also has one byte of data
available to easily persist state from the key press to key release. Of course
the function may also use its own static variables, or any other data source,
to further store and/or read state.

To define a macro, give it a name and add it to the `enum macro` list in
your `layers.c` file. Map it to a key as `MACRO(MACRO_DO_SOMETHING)`.
Then, in `macros.c`, handle that macro in the function

``` C
static
void execute_macro(uint8_t macro_number, bool is_release, uint8_t physical_key, uint8_t * restrict data)
```

The arguments of this function are as follows:

* `macro_number` – the number of the macro, i.e., the `MACRO_DO_SOMETHING`
* `is_release` – `true` if the key is being released, `false` if it is being
  pressed down
* `physical_key` – the plain keycode of the physical key that was pressed
  to trigger the macro (this allows the same macro to be assigned to multiple
  keys and produce a different result)
* `data` – a pointer to a single (1) byte of storage, which you can set the
  value of and it will persist from the key press (`is_release == false`) to
  to release (`is_release == true`)

The helper facilities available to macros are mostly listed in `macros.h`.
However, since this is an arbitrary program, you can call any C function,
use inline assembly, etc. The file `usbkbd_config.h` has various lower level
functions, such as simulated keypresses (bypassing the key processing system)
and simulated typing (if enabled at compile type) where you can
`fprintf(usb_kbd_type, "any text");` and the keyboard will "type" it.

Some of the more important functions are:
* `register_key(uint8_t usbkey, bool is_release)` – register the given USB key
  as pressed (`is_release == false`) or released (`is_release == true`)
* `add_strong_modifiers(uint8_t mask)` / `remove_strong_modifiers(uint8_t
  mask)` / `clear_strong_modifiers()` – manipulate the set of active "strong"
  modifiers (which remain active until cleared)
* `add_weak_modifiers(uint8_t mask)` – add "weak" modifiers that only affect
  the next keypress
* `usb_keyboard_simulate_keypress(uint8_t key, uint8_t modifiers)` – simulate
  the press and release of a key with the given modifiers (this completely
  bypasses the key processing state, i.e., all other keys are released)

#### Macro Example

For example, here is one macro that I defined in order to produce the letters
Ä and Ö on the US Dvorak layout on my iPad – since that layout has no key that
would directly produce these, the macro has to trigger a dead umlaut by
pressing <kbd>Alt</kbd><kbd>U</kbd> and then either <kbd>Shift</kbd>
<kbd>A</kbd> or <kbd>Shift</kbd> <kbd>O</kbd>. Let's call this macro
`MACRO_IPAD_A_O`:

``` C
// layers.c
enum macro {
    // …
    MACRO_IPAD_A_O,
}
```

It also needs to be mapped to the <kbd>A</kbd> and <kbd>O</kbd> keys:

``` C
// layers.c
DEFINE_LAYER(IPAD_LAYER) {
    [KEY(A)] = MACRO(MACRO_IPAD_A_O),
    [KEY(DVORAK_O)] = MACRO(MACRO_IPAD_A_O),
};
```

Note that I'm mapping `KEY(DVORAK_O)` instead of `KEY(O)`, because the
physical keys are named according to QWERTY. The key A is in the same place
for both layouts.

Then, in `macros.c`, handle that macro:

``` C
static void
execute_macro(
    uint8_t macro_number,
    bool is_release,
    uint8_t physical_key,
    uint8_t * restrict data)
{
    const enum macro macro = macro_number;

    switch (macro) {
    // …
    case MACRO_IPAD_A_O:
        if (!is_release) {
            const bool is_dvorak = !is_layer_active(DVORAK_ON_QWERTY_LAYER);
            const uint8_t mods = strong_modifiers_mask();

            if ((mods & (ALT_BIT | ALTGR_BIT)) && !(mods & (CMD_BIT | RIGHT_CMD_BIT | CTRL_BIT | RIGHT_CTRL_BIT))) {
                usb_keyboard_simulate_keypress(
                    is_dvorak ? KEY(DVORAK_U) : KEY(U),
                    ALT_BIT
                );
                clear_strong_modifiers();
                add_weak_modifiers(SHIFT_BIT);
            }
            if (physical_key == KEY(A)) {
                *data = physical_key;
            } else {
                *data = is_dvorak ? KEY(DVORAK_O) : KEY(O);
            }
        }
        register_key(*data, is_release);
        break;
    }
}
```

Basically what happens here is:

* on keypress (`!is_release`) check whether Alt (but not Cmd or
  Ctrl) is active
* if Alt is active, simulate pressing and releasing <kbd>Alt</kbd><kbd>U</kbd>
  (`usb_keyboard_simulate_keypress`) and activate Shift for the next
  keypress only (`add_weak_modifiers`) while clearing any active modifiers
  (`clear_strong_modifiers`)
* either <kbd>A</kbd> or <kbd>O</kbd> is pressed with `register_key`,
  depending on the physical key used to activate the macro
* the state of the (custom, not shown) `DVORAK_ON_QWERTY_LAYER` layer is
  used to determine whether O and U should be pressed according to QWERTY or
  Dvorak layouts
* on release the same key (stored in `*data` on press) is released, also with
  `register_key`

While this is a niche example to be sure, it demonstrates how a macro can
do multiple key presses, use modifiers, and change its behaviour based on the
physical key and active layer state.

## Custom Key Processing

Apart from macros, there are also functions that can be used for custom key
processing that would be unwieldy to handle through the keycode system. The
philosophy here is that for programmers (the target audience) it is simpler to
write a few lines of code than to learn some custom domain-specific macro
language to accomplish the same thing.

> Note: The key processing facilities are not available unless `LAYER_COUNT > 0`.
> If you wish to use only these functions without any layer-based mapping,
> just `#define LAYER_COUNT 1` and `DEFINE_LAYER(1) { PASS };`

### Key Press and Release

``` C
static inline keycode_t
preprocess_press(keycode_t keycode, uint8_t physical_key, uint8_t * restrict data) {
    return keycode;
}
```

The `preprocess_press` function is called after an extended keycode has been
resolved from the currently active layers. It can alter the keycode as it
chooses and return the changed keycode. Returning `NONE` from here causes the
key press to have no further effect. There is one byte of data available that
persists from this call to `postprocess_release`, if set.

``` C
static inline void
postprocess_release(keycode_t keycode, uint8_t physical_key, uint8_t data) {
}
```

The `postprocess_release` function is called after the key has been released.
The `keycode` will be the same that was returned by `preprocess_press`
for the corresponding press, and `data` will contain the byte written to
`*data` there.

### Layer State

``` C
static inline void
layer_state_changed(uint8_t layer, bool is_enabled) {
    if (layer == DVORAK_LAYER) {
        if (is_enabled) {
            add_override_leds_on(LED_SCROLL_LOCK);
        } else {
            remove_override_leds_on(LED_SCROLL_LOCK);
        }
    }
}
```

The `layer_state_changed` function is called when a layer becomes active or
inactive. This includes the effects of setting the base layer, as well as
resetting layers or the entire key processing system. The example shown above
shows how to make an LED indicate the state of a layer. In this example the
LED is only overridden on, not off, i.e., if the computer/host tells the
keyboard to turn on the Scroll Lock LED, it can be on regardless of the
layer state. It would be possible to additionally `add_override_leds_off`
to prevent this.

### Time-based Events

``` C
static inline void
handle_tick(uint8_t tick_10ms_count) {
}
```

The `handle_tick` function is called approximately every 10 milliseconds. This
is not an interrupt, so long macros, etc. can delay the consecutive calls.
Nothing in the default implementation is time-based (other than USB idle
reports), but this function can be used to implement timeouts etc. An example
would be to set some `static` variable from a macro or `preprocess_press`
and then do something here based on that and the elapsed time.

The current `tick_10ms_count` can be obtained outside this function by calling
`current_10ms_tick_count()`. Note that it is an 8-bit value, i.e., it overflows
every 2.56 seconds. You can of course keep track of these rollovers in this
function and have local timers longer than that.

All current devices implemented also use only timers 0 and 1 of the AVR. So
you can use other hardware timers to implement your own timing.

### Debugging

As tip for debugging what is going on, consider something like the following
code in `postprocess_release`:

``` C
if (is_debug_mode) {
    fprintf_P(usb_kbd_type, PSTR(" %04x"), keycode);
}
```

and in `layer_state_changed`:

``` C
if (is_debug_mode) {
    fprintf_P(usb_kbd_type, PSTR(" @%d=%d"), layer, is_enabled);
}
```

(And add some macro to toggle `bool is_debug_mode`, or hardcode it to `true`.)

This makes the keyboard type the keycode, i.e., go into a text editor and
you should see the keycodes for the keys as they are released. You can
of course add any other relevant info to the printout. You may also consider
blocking the actual keys from registering in debug mode, e.g., with the
following in `preprocess_press`:

```
if (is_debug_mode && !is_extended_keycode(keycode)) {
    return NONE;
}
```

This blocks non-extended keys, but allows extended keycodes (i.e., the ones
you have customised) to work. (Those keys will also then print `00ff` from
`postprocess_release`, so you may consider checking `keycode != NONE` there.)

## Architecture

A very simplified overview of the program is as follows:

* [main.h](main.h) – each "main program" (e.g., `ps2usb.c`) needs to implement
  these facilities, as well as reading the physical keyboard and calling
  the key procesing and USB facilities outlined below.
* [keys.c](keys.c) – this is the key processing component (which includes
  `macros.c` and `layers.c`), and is invoked by calling `process_key(key,
  is_release)`. The main function also needs to poll `keys_led_state()` to
  set the physical LEDs to match the desired state (from USB host and custom
  overrides). See [keys.h](keys.h) for the complete set of functions.
* [usbkbd.c](usbkbd.c) – the USB keyboard implementation. This handles all
  of the USB setup and communication (with help from [avrusb.h](avrusb.h)).
  The main program calls `usb_init()` to set up, while `keys.c` calls
  functions like `usb_keyboard_press(key)` and `usb_keyboard_release(key)` to
  activate keypresses. It also calls `usb_keyboard_send_if_needed()` to
  send updates to the computer/host.

The main programs reside in different subdirectories, e.g., `ps2usb`:

* [ps2usb.c](ps2usb/ps2usb.c) – the PS/2 to USB converter's main program,
  which handles the physical keyboard setup, and calling all the init and
  USB/key processing functions
* [ps2usb_keys.c](ps2usb/ps2usb_keys.c) – this is only used by `ps2usb.c` to
  convert from PS/2 keycodes to the corresponding USB keycodes, in order to
  give each key a unique default mapping.
* [kk_ps2.c](ps2usb/kk_ps2.c) – again, only used by `ps2usb.c` to communicate
  with the PS/2 keyboard. This is reused from my
  [PS/2 to serial mouse converter](https://github.com/arkku/mouseps2serial).
  PS/2 is implemented by bit banging and interrupts rather than the USART,
  mainly because in that other project the USART was needed for the RS-232
  serial port. However, this also means the USART is free on this project
  as well, e.g., to use as a debug console.

The main program is selected either with the `DEVICE` argument to `make` or
from the `local.mk` file, e.g., `make DEVICE=ps2usb` targets the PS/2 to USB
converter. There can be a separate `local.mk` file inside the device directory,
i.e., `ps2usb/local.mk`.

For keyboards ported from QMK, such as the [ErgoDox Ez](ergodox), it mostly
suffices to import `config.h` and `matrix.c` files for that keyboard, create a
keymap to define the basic mapping from the matrix positions to _unique_
keycodes (e.g., [keymap.c](ergodox/keymap.c)), and a main program that mostly
just needs to control the keyboard's LEDs (e.g., [ergodox.c](ergodox/ergodox.c)).
In theory things should Just Work if the keyboard itself is running on an
ATMEGA32U4 device. Of course there are likely to be some adjustments needed to
configuration, e.g., disable any non-supported feature.

## About Rollover

When it comes to USB keyboards, there's often talk about key rollover (KRO),
and in particular "NKRO", i.e., the ability to press any number (_n_) keys down
at the same time. NKRO is often contrasted with 6KRO (6-key rollover) of the
USB "boot protocol", which is used by simple USB hosts like the BIOS and some
devices that don't run a full general-purpose OS. However, I think that 6KRO is
often misunderstood to imply 2KRO, which is a typical limitation of
many key matrix -based physical keyboards. This is not the case.

First, let's define "rollover": in this context it means that _any_ combination
of that many keys can be simultaneously recognised. Most keyboards physically
wire the keys into a matrix such that each key is connected to one row and
one column. When the key is pressed, both the row and the column show as
having a keypress. In case of one or two simultaneously pressed keys, the
keys are simply the ones at the positions where the "pressed" rows and columns
intersect. However, pressing a third key connected to the same row or column
as one of the other two keys can make it ambiguous which keys in that shared
row/column in the simple matrix implementation. Hence, such keyboards often
have _some_ combinations of only 3 keys that can't be detected, i.e.,
2-key rollover (2KRO) since the maximum number of keys where _any_ combination
can be detected is 2.

> Modern keyboards tend to add diodes to the matrix to prevent this issue, or
at least arrange the electrical matrix in such ways that those worst cases are
very unlikely to occur in practice. Meanwhile older and simpler keyboards may
use a simple matrix that closely follows the physical key layout, which can
lead to some common combinations being impossible to detect. For example, the
IBM Model M keyboard cannot recognise <kbd>Shift</kbd> <kbd>Caps Lock</kbd>
<kbd>S</kbd>, which can be a problem if you remap <kbd>Caps Lock</kbd> to
<kbd>Ctrl</kbd> or <kbd>Cmd</kbd>.

Completely distinct from the limitations of the physical keyboard, we have the
USB boot protocol limitation of 6KRO. Or, more accurately, any combination of
all eight modifiers (left/right Shift/Ctrl/Alt/Cmd) and then any 6 other keys.
So, if the physical keyboard can support the combination, even the boot protocol
allows pressing one non-modifier key with each finger of one hand, one
more non-modifier key on top of that, _and_ any combination of modifiers.
Personally I find it hard to imagine any _real_ situation where more than this
is needed; in gaming the other hand tends to be on the mouse, and furthermore
the key bindings often include Shift and Ctrl, which do not count towards the
6 keys limit. Perhaps if two people play on a single keyboard, but how common
is that with USB devices where you can just plug in more controllers?

> I do fondly remember that Star Control for DOS came with a "Key jammin'"
program to try which keys can be held down without jamming other keys. Each
player needed to potentially press four keys at once, so for two players it
was somewhat challenging, but not impossible, to find a suitable combination
that is still ergonomic enough to be playable. But nowadays you can
play Star Control 2 Super Melee in
[The Ur-Quan Masters](http://sc2.sourceforge.net) with multiple controllers.

Anyway, I think the desire for NKRO is more likely to be the desire for a
keyboard that doesn't have 2KRO matrix limitations, but those limitations
have become confused with the boot protocol modifiers + 6KRO. Of course gaming
keyboard manufacturers are happy to take advantage of this and pretend that
NKRO is important for gaming, which it isn't. (Even Star Control 2, two players
on one keyboard, would work with 8-10KRO. And, yes, I consider that the
ultimate benchmark.)

This is why this project doesn't bother implementing true NKRO, which would
increase memory use, the size of every USB report packet, and vastly complicate
things. The rollover _is_ configurable, however. You can get modifiers + 7KRO
"for free" since the 7th key uses the normally unused reserved byte of the
boot protocol report. The default `USB_MAX_KEY_ROLLOVER` is set to 10, i.e.,
modifiers + 10KRO, which really "should be enough for anybody", given that you
can press one non-modifier key per finger and any combination of modifiers.

Ten keys with modifiers should even cover the most complex macros that press
many virtual keys with one physical key, especially since most such combinations
involve modifiers, e.g., the <kbd>Hyper</kbd> key of Shift Ctrl Alt Cmd is
_only_ modifiers and thus "free" even in boot protocol mode. The cost of
10KRO over the 6KRO is only 3 extra bytes in the report, and it is compatible
with the boot protocol until more than 6 keys are pressed at once.
If you really think you need more rollover, you can set `USB_MAX_KEY_ROLLOVER`
higher, but I seriously doubt the need beyond bragging rights.

> As an aside, it would indeed be possible to implement NKRO using a bitmap
report for all keys, rather than just the modifiers, but I hope the above
has convinced you that it is not worth it.

Oh, and if you are wondering about boot protocol compatibility, this keyboard
defaults to the "report" (non-boot) protocol unless the host (e.g., BIOS)
specifically requests boot protocol. I believe this to be as
specified. However, some BIOSes do not request the boot protocol. Yet, as
long as they can handle more than 8 bytes in the report, the first 8 bytes
are compatible with the boot protocol even in report protocol mode. To get
similar compatibility with a bitmap NKRO, these bytes would typically be
wasted. If you encounter a BIOS that does not work, the options are to
decrease `USB_MAX_KEY_ROLLOVER` to 7 (or 6 if you need `ENABLE_APPLE_FN_KEY`),
or to map a key to `EXT(TOGGLE_BOOT_PROTOCOL)` and use it to manually switch
to the boot protocol when needed.
