/**
 * usbkbd.h USB HID keyboard implementation for ATMEGA32U4.
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

#ifndef KK_USBKBD_H
#define KK_USBKBD_H

#include <stdbool.h>
#include <stdint.h>

#include "usbkbd_config.h"

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
