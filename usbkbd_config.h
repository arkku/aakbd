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
 * Copyright (c) 2021-2022 Kimmo Kulovesi, https://arkku.dev/
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

#ifndef USB_MAX_KEY_ROLLOVER
/// The maximum number of keys to include in the report. The boot protocol will
/// be supported anyway, which has 6KRO. We can gain 7KRO for free by using the
/// reserved byte in the boot protocol report when not in boot protocol mode.
/// Every extra key past 7 costs 1 byte of report size. I don't think there are
/// (m)any realistic scenarios where even 6 is needed since any of the
/// modifiers (Shift, Ctrl, Alt, Cmd/Win) don't count towards this limit.
#define USB_MAX_KEY_ROLLOVER        7
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
#define SIMULATED_KEYPRESS_TIME_MS 10
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
/// The number of LEDs to report the keyboard supports (typically 3-5).
#define LED_COUNT                   3
#endif
#ifndef DEVICE_VERSION
/// Version number in binary-coded decimal (i.e., 1.00 = 0x0100).
#define DEVICE_VERSION 0x0100U
#endif
#ifndef SCROLL_LOCK_LED_ON_OVERFLOW
/// Blink the scroll lock LED on key overflow?
#define SCROLL_LOCK_LED_ON_OVERFLOW 1
#endif
#ifndef SCROLL_LOCK_LED_ON_SUSPEND
/// Occasionally blink scroll lock LED on suspend (sleep)?
/// The rationale is that since we can't actually power off the keyboard while
/// sleeping (otherwise we couldn't get a wake press from it), it might be
/// useful to know that it's actually powered.
#define SCROLL_LOCK_LED_ON_SUSPEND 1
#endif
#ifndef ENABLE_BOOTLOADER_SHORTCUT
/// Enable Left Shift + Scroll Lock + Right Shift (in that order) key combo to
/// reset and jump to bootloader for firmware update? Note that this can be
/// disabled here and instead mapped to a custom key in `layers.c`.
#define ENABLE_BOOTLOADER_SHORTCUT  1
#endif
#ifndef ENABLE_RESET_SHORTCUT
/// Enable Left Shift + Esc + Right Shift (in that order) key combo to
/// reset the keyboard and release all keys. This can be used if the PS/2
/// keyboard has somehow got into an invalid state. It is recommended to
/// instead map this to a key in `layers.c`, that way it can be customised.
#define ENABLE_RESET_SHORTCUT  0
#endif
#ifndef ENABLE_DEBUG_SHORTCUT
/// Enable Left Shift + F1 + Right Shift (in that order) key combo to "type"
/// debug info with the keyboard. This has little value unless you are
/// messing with the USB implementation.
#define ENABLE_DEBUG_SHORTCUT 0
#endif
#ifndef ENABLE_SIMULATED_TYPING
/// Enable function to simulate typing. This is required for the
/// `ENABLE_DEBUG_SHORTCUT` option and for macros that wish to utilise it.
#define ENABLE_SIMULATED_TYPING 1
#endif
#ifndef DVORAK_MAPPINGS
/// Use Dvorak layout mappings for simulated typing (instead of QWERTY)?
#define DVORAK_MAPPINGS 0
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
#if APPLE_FN_IS_MODIFIER
/// If Apple Fn is a modifier, we don't need an extra byte in the report.
#define ENABLE_EXTRA_APPLE_KEYS 0
#else
/// Instead of wasting 7 bits on padding for the Apple Fn key, it seems to
/// be ok to combine it with other Apple keys. This can be disabled, though,
/// if there are compatibility issues (e.g., with older OS X versions).
#define ENABLE_EXTRA_APPLE_KEYS 1
#endif
#endif

#if ENABLE_APPLE_FN_KEY && APPLE_FN_IS_MODIFIER
/// Repurpose the last modifier (right command) to be Apple Fn.
#define USB_KEY_APPLE_FN    MODIFIERS_END
#define APPLE_FN_BIT        MODIFIER_BIT(USB_KEY_APPLE_FN)
#endif

#if !ENABLE_SIMULATED_TYPING && ENABLE_DEBUG_SHORTCUT
#error "ENABLE_DEBUG_SHORTCUT requires ENABLE_SIMULATED_TYPING"
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

#ifndef USE_MULTIPLE_REPORTS
/// Possible future placeholder, not actually supported.
#define USE_MULTIPLE_REPORTS        0
#endif

// MARK: - Constants

#ifndef HARDWARE_SUPPORTS_HIGH_SPEED
/// Does the USB hardware support high speed (480 Mbps)? In case of the
/// ATMEGA32U4, for which this is originally written, the answer is no.
#define HARDWARE_SUPPORTS_HIGH_SPEED 0
#endif

#ifndef IS_SUSPEND_SUPPORTED
#define IS_SUSPEND_SUPPORTED        1
#endif

#endif
