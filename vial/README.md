## Vial for AAKBD

TL;DR: The AAKBD keyboard firmware has some potentially interesting features,
but has been hard to configure. Now it can be configured via Vial, while
still getting some advantages over regular Vial/QMK firmware.

### Introduction

[Vial](https://vial.rocks) is an open source
[fork of QMK](https://github.com/vial-kb/vial-qmk). It makes keyboards running
QMK configurable on the fly without reflashing firmware, either through a
website or the [Vial app](https://get.vial.today/download/).

[AAKBD](https://github.com/arkku/aakbd/) is a keyboard firmware that is _not_
a fork of QMK, but implements its own key mapping and layers system from
scratch. However, AAKBD is originally meant for programmers and others who
prefer to tweak things in code and text files, rather than easy (but
restrictive) GUI apps, in exchange for arbitrary configurability. This has
meant that AAKBD's target audience has been extremely limited.

People were asking me to port some of AAKBD's advanced features to Vial to make
them more accessible to non-programmers. I didn't. Instead, I ported Vial to
AAKBD. Basically, with Vial support enabled, you can use the Vial website or
app to configure AAKBD on the fly, exactly as you would with the real Vial
firmware.

Most of the layer stuff and modifier+key mappings were already supported by
AAKBD, but I also ported practically all keycodes supported by the Vial GUI
configurator to AAKBD. The only things missing, to my knowledge, are
non-keyboard stuff like mouse keys, MIDI, etc. (but let me know if I missed
something that you need).

> Much of this is currently quite untested since I don't use most of the Vial
> or QMK special features myself, but I did try to test everything manually at
> least once, and I had an AI write thousands of lines of unit tests.
> **Let me know if you find bugs or missing features!**

I have also added things that Vial does not have:

* Real Apple Fn / Globe key support
* Activating layers automatically based on which operating system the host
  computer runs
* Extra read-only layers (use them if you want, otherwise ignore – they are
  free extra layers you can use but don't have to, they take no slots away
  from your own layers)
* Override Num Lock / Caps Lock / Scroll Lock LED state based on which layer is
  active (e.g., for simulated Num Lock, or just to indicate some special layer
  is active by lighting something like Scroll Lock LED)
* PS/2 output on some devices (requires hardware mod)

Most of these extra features are just things AAKBD had but did not directly map
to Vial/QMK keycodes. See below for details on how to use them.

### Extra Read-Only Layers

In addition to providing a number of on-the-fly configurable layers, AAKBD
Vial provides extra read-only layers that can be accessed just like any Vial
layer (e.g., map `MO(4)` to a key to activate layer number 4 while the key is
held down). There is no harm in having them if you don't want them – just
don't map a key to access the extra layers.

These layers can be provided "for free" because they live in differente memory
than the normal Vial layers: Vial layers live in the rewritable EEPROM, while
the extra layers live in the program space flash ROM (which requires a
firmware update in bootloader mode to change). As such, these extra layers do
not take any space from the actual Vial layers.

By default, I have provided a simulated "Fn layer" (e.g., the number row keys
are mapped to F1–F12), and second "Fn + Space layer" (activated from the
"Fn layer" by also holding down <kbd>Space</kbd>) that contains some system
commands like keyboard reset.

**Note**: Since the Vial GUI does not know about these layers, it will allow
you to try to edit them, but will probably get confused when the edits do not
get saved. You can recognize these extra layers from the key with the
"Read-Only Layer" key (<kbd>Enter</kbd>) on them (that key does nothing
special if pressed, it's just a visual hack I made to communicate this in
the GUI).

### Host Operating System Layer Switching

If you use your keyboard on multiple different operating systems, e.g., macOS
and Windows, you may want some different key mappings based on which OS you are
in. You can of course accomplish this manually, but AAKBD has "OS
fingerprinting" that attempts to detect which type of operating system the
keyboard is plugged into.

If you wish to take advantage of this, you can do it as follows:

* Pick a layer (other than number 0) that you want to auto-activate on a
  specific OS
* Put your key mappings there, leave the rest of the keys transparent (down
  arrow)
* From Vial "User" keys tab, put one of the "Mac Layer", "Win Layer" or
  "Linux Layer" on any key on that layer, such that you don't want any special
  mapping for the key (e.g., <kbd>Enter</kbd> or such where the default key
  is ok)

Now, when you unplug the keyboard and plug it back in on the operating system
matching the key you put on the layer, that layer should automatically
activate!

You can put multiple operating systems on the same layer, e.g., if you want
Windows and Linux to auto-activate the same layer, you can put both keys on the
same layer. You can also put the same OS key on multiple layers – they will all
activate.

### LED Overrides

There are four special keycodes in the Vial "User" tab that can be added to
your custom layers to control the keyboard Num Lock, Caps Lock and Scroll Lock
LED states:

* **Num LED On** – Lights the Num Lock LED when the layer is active
* **Num LED Off** – Forces the Num Lock LED _off_ when the layer is active
  (e.g., if you want to make a layer that maps the keypad to the arrows and
  other alternate functions regardless of the computer's Num Lock state, which
  is useful for macOS where there is no "Num Lock off" state)
* **Caps LED On** – Lights the Caps Lock LED when the layer is active
* **Scroll LED On** – Lights the Scroll Lock LED when the layer is active

You can put these on any key on the layer (and you can put more than one of
these per layer if you want). That key will work as the _default_ function
of that key (so put these on keys that you don't remap on the layers below).

### Pre-Built Firmwares

I've pre-compiled various firmware option combinations for convenience. You can
identify the settings based on the words in the file name. For most users I
recommend simply this:

1. pick `iso` or `ansi` depending on your physical keyboard (but doesn't matter
   that much on reconfigurable keyboards like the Model F)
2. pick `applefn` if you use the keyboard with Apple devices (there are no
   major downsides for picking this in any case, so pick it if uncertain)
3. do **not** pick `secure` when trying this out for the first time unless your
   keyboard has a physical reset key you can easily access – the `secure`
   option makes it harder to flash another firmware (to prevent malware from
   doing so), so test first without it and then switch to `secure` later if
   you want it (note that the **matrix tester** is only enabled on `secure`
   builds to prevent key logging by malicious apps)

The rest of the variants are all such that you'll know if you want them, and
you can always change later, so just stick with these basics if uncertain. For
example, if you have US layout Model F77 and you only use it on Windows, pick
`modelf77_ansi.hex`, or if you have a Finnish layout Model F62 and use it on
both Windows and macOS, pick `modelf62_iso_applefn.hex`.

Below is a more thorough list of variants:

* `iso` vs `ansi` – the physical key layout: pick ANSI for US and ISO for most
  of the rest of the world, matching the keyboard you have (for Model F
  keyboards it doesn't actually matter: you can configure it dynamically in
  Vial, including specific variants like split Space, split Backspace,
  split Enter, etc.)
* `applefn` – if you use the keyboard on Apple devices, pick this: it enables
  an actual <kbd>Fn/Globe</kbd> key (aka "Apple Fn") and has practically no
  downside on non-Apple devices. It can be mapped from the "User" tab –
  "Fn" is the Apple Fn key, and "Layer Fn" also toggles to a simulated Fn
  layer (which makes things like <kbd>Fn</kbd> + <kbd>1</kbd> produce
  <kbd>F1</kbd>, while automatically releasing <kbd>Apple Fn</kbd> so that
  it doesn't become <kbd>Fn</kbd> + <kbd>F1</kbd>)
* `dvorak` – if you use Dvorak or a Dvorak-based keyboard layout (you know if
  you do), this just makes the debug macros etc. all type in Dvorak layout
* `secure` – this enables Vial secure mode, which means you have to unlock the
  keyboard to edit macros, reset to bootloader programmatically (you can do it
  without unlocking by keyboard shortcuts), or use the "Matrix tester" (views
  pressed keys in Vial GUI) – all of these are security features to prevent
  malware from flashing evil firmware, or programming a macro that will do
  something bad, or from logging your keystrokes by running the matrix tester
  in the background (i.e., good safety measures in theory but less
  convenient, and it may be harder to update the firmware in case of problems).
  **Note:** The "matrix tester" feature is _only_ enabled in `secure` builds,
  because that is the most exploitable security flaw.
* `secure_dfu` – like `secure`, except that it enables the DFU interface,
  which allows resetting to bootloader programmatically without unlocking
  (but the macros and matrix tester are still blocked, giving most of the
  benefits of `secure` with less chance of having to open up the keyboard to
  press a reset button if you tweak the code and lock yourself out of
  firmware updates)
* `minimal` – this disables most Vial features that consume either EEPROM
  space from dynamic layers (e.g., combos and tap dance) or complicate
  the key processing (e.g., auto-shift) – this variant sometimes has more
  dynamic layers and in theory the key processing is less bloated so it may
  be a few microseconds faster (but you will not detect any difference,
  it is orders of magnitude below debouncing and USB poll rate delays)
* `solenoid` – on Model F keyboards this variant has support for a solenoid
  (i.e., haptic feedback - requires extra hardware installed in the keyboard),
  it is on the default pins B6 and B7, but I don't actually have a solenoid
  myself so I have not tested this (let me know if it doesn't work and we can
  figure out how to fix it)

### PS/2 Output

AAKBD supports [PS/2 output](../ps2) in addition to USB, at least on the
AVR-based keyboards (which are currently the majority of AAKBD's supported
devices).

PS/2 output requires hardware modification, although on many keyboards it can
be as simple as connecting the wires of a PS/2 connector to a pin header on the
side of the keyboard.

**Do not install a PS/2 firmware if you don't have the hardware modification.**
It's not dangerous as such, but if the presence of PS/2 host is misdetected,
you may be stuck unable to get into USB mode. If this happens anyway, the
solution is then to plug in the keyboard while shorting out the programming
pads on the controller, so there is always a way out, but you may have to open
up the keyboard for it. (With the USB-only firmwares you should always be able
to reset to the bootloader from Vial menu, or with DFU, or with a key combo
if you have mapped one.)

The pre-built firmwares with PS/2 output have:

* PS/2 Data on pin `B6`
* PS/2 Clock on pin `B7`

On the Model F Labs xwhatsit-based controllers these are the same pins on the
6-pin header on the side as the solenoid pins. You will also need +5V and
ground.

The pre-built firmwares have multiple PS/2 variants:

* `ps2full` – Full PS/2 feature set: all scancode sets and legacy corner cases
  pretty much indistinguishable from IBM Model M PS/2 keyboard
* `ps2set23` – Scancode sets 2 (the default) and 3 (the sensible one) – this is
  the best choice if you use it with a non-Microsoft OS but don't need the
  extremely legacy set 1
* `ps2mini` – Only the default scancode set 2 (used by Windows / MS-DOS, many
  later commercial PS/2 keyboards only support this), and no legacy corner
  cases

The more PS/2 features you have, the less space is left for Vial features, so
progressively more Vial features (like combos and auto-shift) are disabled when
PS/2 features are added. If you are one of the very few people actually
interested in using the PS/2 output and are missing some feature, let me know
and I may be able to shuffle the build options around (personally I don't care
about any of the Vial combo/tap dance/autoshift features so I just picked
based on how much space they take).

### Compiling custom versions

Setting `VIAL_ENABLE=1` on the make command line enables Vial support. So,
essentially just `make VIAL_ENABLE=1` is sufficient (+ add your usual
configuration flags either on command-line or `local.mk` as desired). However,
this compilation does not use the usual `layer.c` and `macros.c`, but rather
`layers_vial.c` and `macros_vial.c`. Both files still work the same way as
without Vial, but the layer numbering and count need to use `STATIC_LAYER_1`
as the first layer (and `STATIC_LAYER_2` as the second, etc.), and then also
`#define LAYER_COUNT MY_HIGHESTS_LAYER` where `MY_HIGHEST_LAYER` is the
highest-numbered static layer you defined. This is because the layer numbering
depends on the number of Vial layers, which can change depending on config
(it gives the maximum that will fit in the EEPROM).

You can change the static (read-only) layers by creating your own
[layers_vial.c](../modelf77/template_layers_vial.c) file. The main difference
to the normal (non-Vial) AAKBD configuration is that you need to start your
layer numbering not from 1, but from `STATIC_LAYER_1` (the value of which
depends on how many dynamic Vial layers there are), and similarly set
the `LAYER_COUNT` to the last `STATIC_LAYER_n` that you used (not to the
number of static layers). Otherwise everything works the same as normal
AAKBD `layers.c`.

Note that the special keycodes I've detailed above are actually just macros
implemented in [template_macros_vial.c](../xwhatsit/template_macros_vial.c),
so if you make a custom `macros_vial.c` you have to copy them over from the
template to preserve that functionality. (But as a bonus, you can see they
are really easy to implement, so you can add your own similar functionality
using the same patterns.)

### Supported keyboards

Vial is currently supported on all of AAKBD's keyboards that have been ported
from QMK, which is to say all commercial keyboards (but excluding the original
PS/2 to USB adapter that started AAKBD).

At the time of writing those keyboards are only the ones that I personally
have, since I want to test all of them before releasing.

If your keyboard is not supported but you know it runs the real Vial firmware
(or even just QMK), and you would like to use it with AAKBD, let me know! For
most keyboards it is easy to port them to AAKBD, but I just need someone who
has that keyboard to test it.

