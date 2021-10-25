/**
 * usbkbd.h USB HID keyboard implementation for ATMEGA32U4.
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
 * Copyright (c) 2021 Kimmo Kulovesi, https://arkku.dev/
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

#ifndef KK_USBKBD_H
#define KK_USBKBD_H

#include <stdbool.h>
#include <stdint.h>

// MARK: - Configuration

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
#define PRODUCT_STRING              "PS/2 Keyboard"
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
#if ENABLE_APPLE_FN_KEY && !defined(ENABLE_EXTRA_APPLE_KEYS)
/// Instead of wasting 7 bits on padding for the Apple Fn key, it seems to
/// be ok to combine it with other Apple keys. This can be disabled, though,
/// if there are compatibility issues (e.g., with older OS X versions).
#define ENABLE_EXTRA_APPLE_KEYS 1
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

// MARK: - Constants

#ifndef HARDWARE_SUPPORTS_HIGH_SPEED
/// Does the USB hardware support high speed (480 Mbps)? In case of the
/// ATMEGA32U4, for which this is originally written, the answer is no.
#define HARDWARE_SUPPORTS_HIGH_SPEED 0
#endif

// MARK: - USB

#ifdef INCLUDE_USB_HARDWARE_ACCESS

/// Start USB.
void usb_init(void);

/// Is USB configured and was the last operation a success?
bool usb_is_ok(void);

/// Is USB configured?
bool usb_is_configured(void);

/// Zero if the last USB operation was a success, a non-zero code otherwise.
uint8_t usb_last_error(void);

/// Is USB suspended?
bool usb_is_suspended(void);

/// Deinit and disable USB.
void usb_deinit(void);

/// Is USB host requesting detach (e.g., for firmware update)?
/// If yes, this is the nearest positive 8-bit value of the detach timeout
/// requested (in milliseconds).
uint8_t usb_detach_requested(void);

/// This should be called periodically to allow the USB keyboard to handle any
/// internal timers, etc.
void usb_tick(void);

/// Is USB using boot protocol mode?
bool usb_is_in_boot_protocol(void);

/// The USB address of the device.
uint8_t usb_address(void);

#endif

// MARK: - Keyboard

#ifdef INCLUDE_USB_KEYBOARD_ACCESS

#define KEY_ROLLOVER_ERROR_CODE         0x01
#define KEY_UNDEFINED_ERROR_CODE        0x03
#define KEY_MAX_ERROR_CODE              KEY_UNDEFINED_ERROR_CODE

#define KEY_ERROR_OVERFLOW              KEY_ROLLOVER_ERROR_CODE
#define KEY_ERROR_OVERFLOW_REPORTED     (KEY_ERROR_OVERFLOW + 1)
#define KEY_ERROR_GENERAL               KEY_UNDEFINED_ERROR_CODE
#define KEY_ERROR_GENERAL_REPORTED      (KEY_ERROR_GENERAL + 1)

/// Press `key` down. It can be any USB key, including modifier or error state.
void usb_keyboard_press(const uint8_t key);

/// Release `key` up. It can be any USB key, including modifier or error state.
void usb_keyboard_release(const uint8_t key);

/// Release all keys and modifiers.
void usb_release_all_keys(void);

/// Send the current keyboard state to the USB host.
bool usb_keyboard_send_report(void);

/// Send the current keyboard state to the USB host if there have been changes.
/// This must be called periodically (preferably after each set of keypresses)
/// for the keyboard to react propertly to the press/release calls.
/// This is not needed, however, for simulated typing / keypresses.
bool usb_keyboard_send_if_needed(void);

/// The desired keyboard LED state as set by the USB host. This should be
/// polled periodically to sync the state of the physical LEDs.
uint8_t usb_keyboard_led_state(void);

/// Error state of the keyboard, such as `KEY_ERROR_OVERFLOW`. Overflow errors
/// are automatically cleared when all keys are released, and a reset will be
/// requested if there is an error state with unseen keys being released.
uint8_t usb_key_error(void);

/// The bit mask of active modifiers.
uint8_t usb_keyboard_modifiers(void);

/// Sets the active modifier mask.
void usb_keyboard_set_modifiers(const uint8_t modifier_flags);

/// Add the given mask to the active modifiers.
void usb_keyboard_add_modifiers(const uint8_t modifier_flags);

/// Remove the given mask from the active modifiers.
void usb_keyboard_remove_modifiers(const uint8_t modifier_flags);

/// Simulate the press and release of `key` with `modifier_flags`. Any existing
/// modifiers are released before and restored after the simulated press. Any
/// existing non-modifier keys are not released for the simulated press, which
/// also means the simulated press may overflow the report (and not register)
/// if there are already `USB_MAX_KEY_ROLLOVER` keys held down (or 6 in boot
/// protocol mode).
bool usb_keyboard_simulate_keypress(const uint8_t key, const uint8_t modifier_flags);

#if ENABLE_APPLE_FN_KEY
/// Set the Apple Fn key down. This key does _not_ count as a modifier, even
/// though it technically is.
void press_apple_virtual(const uint8_t key);

/// Set the Apple Fn key up.
void release_apple_virtual(const uint8_t key);

/// Is the Apple Fn key being held down?
bool is_apple_virtual_pressed(const uint8_t key);
#endif

#if ENABLE_SIMULATED_TYPING
#include <stdio.h>

/// Simulate typing the character `chr` on a US QWERTY layout. Supports all the
/// normal printable characters, but not control characters.
bool usb_keyboard_type_char(const char chr);

/// Type the eight bits of the given bitmask as 0 or 1, most significant first.
void usb_keyboard_type_bitmask(const uint8_t bitmask);

/// A virtual file that can be used to `fputs` or `printf` simulated typing.
/// Ultimately calls `usb_keyboard_type_char` to print each character.
extern FILE *usb_kbd_type;

/// Simulate typing a debug info report.
void usb_keyboard_type_debug_report(void);
#endif

/// Toggles the keyboard protocol between boot protocol and report protocol.
/// This can be used to work around a BIOS that does not request the boot
/// protocol correctly, even though according to spec it should.
void usb_keyboard_toggle_boot_protocol(void);

#endif // INCLUDE_USB_KEYBOARD_ACCESS

#endif
