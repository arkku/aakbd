/**
 * usbkbd_config.h USB HID keyboard configuration.
 *
 * This header contains a list of available configuration options.
 *
 * The best place to configure these is to create a file called `local.mk`
 * and in that file put `CONFIG_FLAGS =` followed by the options with a `-D`
 * prefix for each, for example:
 *********************************************************************
        CONFIG_FLAGS = \
            -DUSB_MAX_KEY_ROLLOVER=10 \
            -DUSB_VENDOR_ID=0x16C0 \
            -DUSB_MANUFACTURER_STRING='"Manufacturer"'
 *********************************************************************
 *
 * Note that you need to escape double quotes in that file, such as with '.
 *
 * Copyright (c) 2021-2026 Kimmo Kulovesi, https://arkku.dev/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KK_USBKBD_CONFIG_H
#define KK_USBKBD_CONFIG_H

#ifndef VIAL_ENABLE
#define VIAL_ENABLE 0
#endif

#if VIAL_ENABLE
#include "vial_config.h"
#endif

#ifndef USB_MAX_KEY_ROLLOVER
/// The maximum number of keys to include in the report. The boot protocol will
/// be supported anyway, which has 6KRO. We can gain 7KRO for free by using the
/// reserved byte in the boot protocol report when not in boot protocol mode.
/// Every extra key past 7 costs 1 byte of report size. I don't think there are
/// (m)any realistic scenarios where even 6 is needed since any of the
/// modifiers (Shift, Ctrl, Alt, Cmd/Win) don't count towards this limit.
#define USB_MAX_KEY_ROLLOVER        10
#endif
#ifndef MAX_KEY_ROLLOVER
/// The internal maximum key rollover to keep track of. This can be higher than
/// `USB_MAX_KEY_ROLLOVER` – in this case we still correctly track key releases
/// even though they can't be reported. This costs only a few bytes of memory,
/// so 10 seems like a reasonable number (one key per finger + modifiers).
#if USB_MAX_KEY_ROLLOVER > 10
#define MAX_KEY_ROLLOVER            USB_MAX_KEY_ROLLOVER
#else
#define MAX_KEY_ROLLOVER            10
#endif
#endif
#ifndef MAX_POWER_CONSUMPTION_MA
/// Maximum power consumption in mA to report. Some USB hosts may disable the
/// device if we exceed this, but there shouldn't be any problem using less
/// than we requested, as long as the device has the current available.
#define MAX_POWER_CONSUMPTION_MA    100
#endif
#ifndef KEYBOARD_POLL_INTERVAL_MS
/// Poll interval in milliseconds.
#define KEYBOARD_POLL_INTERVAL_MS   2
#endif
#ifndef KEYBOARD_UPDATE_IDLE_MS
/// Initial keyboard idle update interval in milliseconds. The HID spec
/// recommends 500 ms. Note that the host can change this, so the initial
/// value is of fairly little consequence.
#define KEYBOARD_UPDATE_IDLE_MS     500
#endif
#ifndef SIMULATED_KEYPRESS_TIME_MS
/// How long to hold down a key when simulating a keypress, in milliseconds?
/// Some keys, such as Esc, can be ignored if the simulated press duration is
/// too short (since it may be unrecognisable from a terminal escape sequence).
/// Since the delay halts everything, I recommend using at most 10 ms here
/// and configuring software to use a shorter delay as necessary.
#if KEYBOARD_POLL_INTERVAL_MS <= 10
#define SIMULATED_KEYPRESS_TIME_MS 10
#else
#define SIMULATED_KEYPRESS_TIME_MS KEYBOARD_POLL_INTERVAL_MS
#endif
#endif

#ifndef RESET_LAYERS_ON_SUSPEND
/// Should layer state be preserved when waking up from suspend (e.g., computer
/// sleep suspends USB keyboards)? The advantage is preserving any sticky
/// layers, the risk is that theoretically there might be some momentary layers
/// activated during or before suspend and they will stay active permanently
/// since the key up event will be lost. But that should be rare, e.g., you are
/// probably not putting the computer to sleep while holding an Fn key.
///
/// However, if you are doing custom layer state management in `handle_reset`
/// in `macros.c`, you may wish to set this to `1` to avoid conflicts.
#define RESET_LAYERS_ON_SUSPEND     0
#endif

#ifndef USB_VENDOR_ID
/// USB vendor id. Technically this needs to be assigned by USB-IF, but
/// in practice it should be safe to copy the vendor and product id of an
/// existing driverless keyboard. (e.g., for my IBM Model M, I'm using
/// IBM's vendor id and the product id of one of their USB keyboards).
#define USB_VENDOR_ID               0x16C0U
#endif
#ifndef USB_PRODUCT_ID
/// USB product id. Also see `USB_VENDOR_ID`.
#define USB_PRODUCT_ID              0x047DU
#endif
#ifndef LANGUAGE_ID
/// The language of the strings (manufacturer, product, serial number).
#define LANGUAGE_ID                 LANGUAGE_ID_EN_US
#endif
#ifndef MANUFACTURER_STRING
/// The manufacturer name to report. Many operating systems display the
/// manufacturer name associated with `USB_VENDOR_ID` instead of this.
#define MANUFACTURER_STRING         "USB"
#endif
#ifndef PRODUCT_STRING
/// The product name to report.
#define PRODUCT_STRING              "AAKBD"
#endif
#ifndef SERIAL_NUMBER_STRING
/// The serial number to report. (It doesn't actually have to be a number.)
#define SERIAL_NUMBER_STRING        "arkku.dev"
#endif
#ifndef USB_VERSION
/// Supported USB version in binary-coded decimal (e.g., 1.11 = 0x0110).
#define USB_VERSION                 0x0110U
//#define USB_VERSION                 0x0200U
#endif
#ifndef LED_COUNT
/// The number of LEDs to report the keyboard supports. Officially they are
/// defined up to 5: `LED_NUM_LOCK_BIT`, `LED_CAPS_LOCK_BIT`,
/// `LED_SCROLL_LOCK_BIT`, `LED_COMPOSE_BIT`, and `LED_KANA_BIT`. The default
/// is the classic 3 LEDs, but this can be defined up to 5 to support more.
#define LED_COUNT                   3
#endif
#ifndef ENABLE_VIRTUAL_LEDS
#if LED_COUNT <= 5
/// Virtual LEDs are locally-generated status indicators that can be displayed
/// by custom hooks or used in macros.
#define ENABLE_VIRTUAL_LEDS         1
#else
#define ENABLE_VIRTUAL_LEDS         0
#endif
#endif

#ifndef DEVICE_VERSION
/// Version number in binary-coded decimal (i.e., 1.00 = 0x0100).
#define DEVICE_VERSION 0x0100U
#endif

#ifndef ENABLE_BOOTLOADER_SHORTCUT
/// Enable Left Shift + Esc + Right Shift (in that order) key combo to
/// reset and jump to bootloader for firmware update? Note that a better way
/// to do the same is by mapping a key, this is mostly for development use
/// when the mappings might not work.
#define ENABLE_BOOTLOADER_SHORTCUT 0
#endif

#ifndef DEBOUNCE_DEBUG
/// Collect debounce statistics: histogram of actual debounce times needed.
/// The debug printout will include a line showing the distribution of observed
/// debounce delays (in ms) and the highest debounce value that would have
/// been sufficient. Only relevant when DEBOUNCE is non-zero, and also only
/// really works on the default sym_defer_g debounce type (QMK-based).
#define DEBOUNCE_DEBUG 0
#endif

#ifndef ENABLE_HOST_FINGERPRINT
/// Enable USB host (computer operating system) fingerprinting. Needs to be
/// supported by the USB implementation.
#define ENABLE_HOST_FINGERPRINT 0
#endif

#ifndef ONESHOT_TAP_TOGGLE
// The number of taps on a oneshot layer to lock it in rather than oneshot.
#define ONESHOT_TAP_TOGGLE          3
#endif
#ifndef ONESHOT_TIMEOUT_MS
/// Timeout to auto-clear oneshot when no key pressed (0 = no timeout).
#define ONESHOT_TIMEOUT_MS          2500
#endif
#define ONESHOT_TIMEOUT_MS_MAX      2500

#ifndef ENABLE_SIMULATED_TYPING
/// Enable function to simulate typing. This is used for macros that wish
/// to simulate typing output.
#define ENABLE_SIMULATED_TYPING 1
#endif
#ifndef DVORAK_MAPPINGS
/// Use Dvorak layout mappings for simulated typing (instead of QWERTY)?
#define DVORAK_MAPPINGS 0
#endif
#ifndef ENABLE_MEDIA_KEYS
/// Enable media keys (consumer control usage).
#define ENABLE_MEDIA_KEYS 0
#endif
#ifndef MEDIA_KEYS_ENDPOINT
/// Use a separate HID endpoint for media keys (Windows-compatible array
/// format). When 0, media keys are embedded in the keyboard report as
/// bitfield (macOS/Linux compatible, but fewer keys). When non-zero,
/// Apple Fn remains in the keyboard report but media keys get their own
/// endpoint with full 16-bit consumer usage support.
#define MEDIA_KEYS_ENDPOINT 0
#endif
#ifndef ENABLE_APPLE_FN_KEY
#if USB_VENDOR_ID == USB_VENDOR_ID_APPLE
/// Enable Apple Fn key? This requires you to use Apple's USB_VENDOR_ID.
#define ENABLE_APPLE_FN_KEY 1
#else
#define ENABLE_APPLE_FN_KEY 0
#endif
#endif
#if !ENABLE_APPLE_FN_KEY
#undef APPLE_FN_IS_MODIFIER
#define APPLE_FN_IS_MODIFIER 0
#endif
#ifndef APPLE_FN_IS_MODIFIER
/// If you don't need the right command key, Apple Fn can be placed there,
/// which allows using it like a modifier. This means that you don't have to
/// use the `USB_KEY_VIRTUAL_APPLE_FN` anymore, and can instead use the "real"
/// key `USB_KEY_APPLE_FN` (which has the same keycode as right Cmd/Win).
#define APPLE_FN_IS_MODIFIER 0
#endif
#if ENABLE_APPLE_FN_KEY && !defined(ENABLE_EXTRA_APPLE_KEYS)
#if APPLE_FN_IS_MODIFIER || ENABLE_MEDIA_KEYS
/// If Apple Fn is a modifier, we don't need an extra byte in the report,
/// or the byte is better spent on media keys.
#define ENABLE_EXTRA_APPLE_KEYS 0
#else
/// Instead of wasting 7 bits on padding for the Apple Fn key, it seems to
/// be ok to combine it with other Apple keys. This can be disabled, though,
/// if there are compatibility issues (e.g., with older OS X versions).
#define ENABLE_EXTRA_APPLE_KEYS 1
#endif
#endif

#ifndef ENABLE_DFU_INTERFACE
/// Enable the DFU interface? This has very low use of resources and allows
/// resetting the device into bootloader mode easily, so this is recommended
/// even if you don't have a DFU-compatible bootloader (you can still reset
/// the device with `dfu-util -e`).
#define ENABLE_DFU_INTERFACE        1
#endif

#ifndef ENABLE_KEYBOARD_ENDPOINT
/// For debugging only: allows disabling the keyboard USB endpoint, which
/// kind of defeats the purpose of a USB keyboard. Perhaps in the future this
/// might be useful if the keyboard data is sent over PS/2 and USB exists for
/// debugging and firmware updates only.
#define ENABLE_KEYBOARD_ENDPOINT    1
#endif

// MARK: - Constants

#ifndef HARDWARE_SUPPORTS_HIGH_SPEED
/// Does the USB hardware support high speed (480 Mbps)? In case of the
/// ATMEGA32U4, for which this is originally written, the answer is no.
#define HARDWARE_SUPPORTS_HIGH_SPEED 0
#endif

#ifndef IS_SUSPEND_SUPPORTED
/// Is suspend (sleep) supported? Makes fairly little difference on keyboards
/// without RGB, solenoid, or other power-consuming bells and whistles, but
/// also should be harmless. You can try disabling if the keyboard won't wake
/// up from sleep.
#define IS_SUSPEND_SUPPORTED        1
#endif

#ifndef USB_DEINIT_POWER_DOWN
/// If enabled, USB deinit completely powers down the USB facilities. This
/// saves a tiny amount of power, but probably has other downsides. Untested.
#define USB_DEINIT_POWER_DOWN       0
#endif

#if ENABLE_APPLE_FN_KEY || ENABLE_MEDIA_KEYS
/// Virtual keys are keys that do not directly correspond to USB keycodes.
/// These are handled as special cases, and do not count towards the USB
/// key rollover.
#define ENABLE_VIRTUAL_KEYS         1
#else
#define ENABLE_VIRTUAL_KEYS         0
#endif

#if ENABLE_MEDIA_KEYS
#if defined(MEDIA_KEYS_COUNT) && MEDIA_KEYS_COUNT > 8 && !defined(MEDIA_KEYS_ENDPOINT)
#define MEDIA_KEYS_ENDPOINT 1
#endif

#if MEDIA_KEYS_ENDPOINT
#ifndef MEDIA_KEYS_COUNT
#define MEDIA_KEYS_COUNT 22
#endif
#if MEDIA_KEYS_COUNT != 22 && MEDIA_KEYS_COUNT != 8
#warning "MEDIA_KEYS_COUNT can be either 22 or 8 when using MEDIA_KEYS_ENDPOINT"
#undef MEDIA_KEYS_COUNT
#define MEDIA_KEYS_COUNT 22
#endif
#else // ^ MEDIA_KEYS_ENDPOINT
#ifndef MEDIA_KEYS_COUNT
#if ENABLE_APPLE_FN_KEY && !(APPLE_FN_IS_MODIFIER || ENABLE_EXTRA_APPLE_KEYS)
// Share the reserved byte with Apple Fn
#define MEDIA_KEYS_COUNT            7
#else
#define MEDIA_KEYS_COUNT            8
#endif
#else // ^ !MEDIA_KEYS_COUNT
#if MEDIA_KEYS_COUNT < 7
#warning "MEDIA_KEYS_COUNT less than 7 doesn't currently help anything."
#undef MEDIA_KEYS_COUNT
#define MEDIA_KEYS_COUNT 7
#endif
#endif // ^ MEDIA_KEYS_COUNT
#if MEDIA_KEYS_COUNT > 8
#error "MEDIA_KEYS_COUNT > 8 requires MEDIA_KEYS_ENDPOINT"
#endif
#endif // ^ !MEDIA_KEYS_ENDPOINT
#else // ^ ENABLE_MEDIA_KEYS
#undef MEDIA_KEYS_COUNT
#define MEDIA_KEYS_COUNT            0
#endif // ^ !ENABLE_MEDIA_KEYS

// When using the consumer endpoint with full key set, disable extra Apple keys
// to free virtual key slots for the 23 consumer keys.
#if MEDIA_KEYS_ENDPOINT && MEDIA_KEYS_COUNT > 8
#if defined(ENABLE_EXTRA_APPLE_KEYS) && ENABLE_EXTRA_APPLE_KEYS
#warning "ENABLE_EXTRA_APPLE_KEYS requires MEDIA_KEYS_COUNT <= 8"
#endif
#undef ENABLE_EXTRA_APPLE_KEYS
#define ENABLE_EXTRA_APPLE_KEYS 0
#endif

#if ENABLE_APPLE_FN_KEY
#if ENABLE_EXTRA_APPLE_KEYS
#define APPLE_KEYS_EXTRA_BITS       8
#else
#define APPLE_KEYS_EXTRA_BITS       (APPLE_FN_IS_MODIFIER ? 0 : 1)
#endif
#else
#define APPLE_KEYS_EXTRA_BITS       0
#endif

#if ENABLE_MEDIA_KEYS && !MEDIA_KEYS_ENDPOINT
// Media keys embedded in keyboard report (bitfield)
#if (MEDIA_KEYS_COUNT + APPLE_KEYS_EXTRA_BITS) <= 8
#define MEDIA_KEYS_BIT_OFFSET       APPLE_KEYS_EXTRA_BITS
#define VIRTUAL_KEY_BYTES_IN_REPORT 1
#else
#define VIRTUAL_KEY_BYTES_IN_REPORT 2
#endif
#else // ^ keyboard report media keys
// Media keys on separate endpoint or disabled: only Apple keys in keyboard report
#define VIRTUAL_KEY_BYTES_IN_REPORT (APPLE_KEYS_EXTRA_BITS > 0 ? 1 : 0)
#endif // ^ separate endpoint or no media keys

#if VIRTUAL_KEY_BYTES_IN_REPORT > 1
#warning "There are multiple bytes of virtual keys - not fully boot protocol compatible."
#endif

// MARK: - PS/2 Keyboard Output

#ifndef ENABLE_PS2_DEVICE
/// Should the PS/2device (i.e., PS/2 output) be enabled? This is currently
/// only supported on AVR, and requires hardware wiring.
#define ENABLE_PS2_DEVICE 0
#endif

#if ENABLE_PS2_DEVICE
#ifndef ENABLE_PS2_NKRO
/// Unlike USB, PS/2 sends individual "key pressed" and "key released" events.
/// Thus, there is no upper limit to how many keys it can reporrt as being
/// held down at once, since it is the PS/2 host (computer) that keeps track
/// of it. Therefore (unlike USB where it lowers compatibility and efficiency),
/// PS/2 can support n-key rollover (NKRO) with no extra effort. This controls
/// that.
///
/// The only downside is if you enable this on a keyboard that does not
/// _physically_ support NKRO, then holding down too many keys may cause
/// situations where the release event never occurs and the key is stuck
/// down "forever". But recovery is simple: press and release the stuck key.
#define ENABLE_PS2_NKRO 1
#endif

#ifndef ENABLE_PS2_DEVICE_SET_3
/// PS/2 has three different scancode sets. The default is set 2, and the only
/// required one. The better, more consistent, set 3 is poorly supported among
/// real keyboards, but this firmware DOES support it fully. Enabling set 3
/// increases both code size and RAM (in the keyboard firmware, not the host
/// computer), but it does give consistent key press and release events for
/// every key (no special case like pause/break) _if the host enables it_.
#define ENABLE_PS2_DEVICE_SET_3 1
#endif
#ifndef ENABLE_PS2_DEVICE_SET_1
/// Out of PS/2's three different scancode sets, set 1 is basically formed
/// from set 2 by a translation process. Enabling it probably makes no
/// difference in practice since any host can either use the superior set 3
/// or do the translation from the guaranteed-working set 2. But enabling it
/// also costs nothing except ROM size. Disable if you run out of space.
#define ENABLE_PS2_DEVICE_SET_1 1
#endif

#ifndef ENABLE_FALLBACK_TO_PS2_FROM_USB
/// When PS/2 output is enabled, the firmware always checks for PS/2 first and
/// decides whether to enable PS/2 or USB mode. But some PS/2 hosts are not
/// easily detectable and may incorrectly go to USB mode (e.g., they may
/// actively suppress the PS/2 bus on boot until they are ready). With this
/// enabled, PS/2 detection will be attempted again if the USB fails to
/// connect within a timeout.
///
/// Downside: when connected to PS/2 if the pins are shared with USB, the USB
/// attempt will look like noise on the PS/2 bus, which might confuse some
/// hosts. Also, if USB is really slow to enumerate, might cause incorrect
/// PS/2 mode activation in some corner cases (have not had it happen, but it
/// might). In that case need to restart the keyboard (bind a shortcut key or
/// just plug it in again).
#define ENABLE_FALLBACK_TO_PS2_FROM_USB 1
#endif

#ifndef PS2_DEVICE_ID
/// The PS/2 host can query the keyboard for a device id. Not all do, but some
/// have special cases for specific keyboards (e.g., media key support based
/// on non-standard scancodes). The default value is what is reported by
/// IBM Model M, which should trigger no special behaviour and is also the true
/// OG PS/2 keyboard. (The original PS/2 systems also complain if they do not
/// see an IBM keyboard, even though third-party keyboards work fine.)
#define PS2_DEVICE_ID    0xAB83U
#endif
#endif

#if !VIAL_ENABLE
#define VIAL_LAYER_COUNT 0
#endif

#ifndef ENABLE_ONESHOT_KEYCODES
/// Enable one-shot layer and one-shot modifier keycodes.
#define ENABLE_ONESHOT_KEYCODES     VIAL_ENABLE
#endif

#ifndef ENABLE_SPACE_CADET
/// Enable Space Cadet extended keycodes: modifier on hold, shifted
/// character on tap.
#define ENABLE_SPACE_CADET          VIAL_ENABLE
#endif

#ifndef ENABLE_TRI_LAYER
/// Enable tri-layer momentary layer keys (FN_MO13, FN_MO23): layer
/// on hold, tri-layer adjust layer when both held together.
#define ENABLE_TRI_LAYER            VIAL_ENABLE
#endif

#ifndef ENABLE_AUTOSHIFT
/// Enable support for Vial auto-shift. This does not actually make
/// auto-shift active all the time by default, it must be explicitly
/// enabled by toggling with the auto-shift control keys.
#define ENABLE_AUTOSHIFT           VIAL_ENABLE
#endif

#ifndef GRAVE_ESC_OVERRIDE_MASK
/// Bitmask of modifier bits that force Grave Escape to produce Esc.
/// Default 0: Shift/GUI+Grave → tilde/grave, Alt/Ctrl+Grave → Esc.
/// Example: set to (SHIFT_BIT | ALT_BIT) to make Shift or Alt also produce Esc.
#define GRAVE_ESC_OVERRIDE_MASK     0
#endif

#endif // ^ KK_USBKBD_CONFIG_H
