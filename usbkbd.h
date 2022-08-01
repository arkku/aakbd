/**
 * usbkbd.h: USB HID keyboard implementation.
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

#include "usb.h"
#include "usbkbd_config.h"

// MARK: - Keyboard

#define KEY_ROLLOVER_ERROR_CODE         0x01
#define KEY_UNDEFINED_ERROR_CODE        0x03
#define KEY_MAX_ERROR_CODE              KEY_UNDEFINED_ERROR_CODE

#define KEY_ERROR_OVERFLOW              KEY_ROLLOVER_ERROR_CODE
#define KEY_ERROR_OVERFLOW_REPORTED     (KEY_ERROR_OVERFLOW + 1)
#define KEY_ERROR_GENERAL               KEY_UNDEFINED_ERROR_CODE
#define KEY_ERROR_GENERAL_REPORTED      (KEY_ERROR_GENERAL + 1)

/// Reset the USB keyboard state to initial, unconfigured values.
/// This should be called before using the other functions.
void usb_keyboard_reset(void);

/// Press `key` down. It can be any USB key, including modifier or error state.
void usb_keyboard_press(const uint8_t key);

/// Release `key` up. It can be any USB key, including modifier or error state.
void usb_keyboard_release(const uint8_t key);

/// Release all keys and modifiers.
void usb_keyboard_release_all_keys(void);

/// Send the current keyboard state to the USB host.
bool usb_keyboard_send_report(void);

/// Send the current keyboard state to the USB host if there have been changes.
/// This must be called periodically (preferably after each set of keypresses)
/// for the keyboard to react propertly to the press/release calls.
/// This is not needed, however, for simulated typing / keypresses.
bool usb_keyboard_send_if_needed(void);

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

/// Toggles the keyboard protocol between boot protocol and report protocol.
/// This can be used to work around a BIOS that does not request the boot
/// protocol correctly, even though according to spec it should.
void usb_keyboard_toggle_boot_protocol(void);

#if ENABLE_APPLE_FN_KEY
/// Set the Apple Fn key down. This key does _not_ count as a modifier, even
/// though it technically is.
void press_apple_virtual(const uint8_t key);

/// Set the Apple Fn key up.
void release_apple_virtual(const uint8_t key);

/// Is the Apple Fn key being held down?
bool is_apple_virtual_pressed(const uint8_t key);
#endif

// MARK: - Simulated typing

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

/// Sends the keyboard state report.
bool usb_keyboard_send_report(void);

// MARK: - Keyboard State

/// Get the keyboard LEDs state, as requested by the USB host.
/// Note: For low-level access, use `usb_keyboard_leds` directly.
uint8_t usb_keyboard_led_state(void);

#ifdef USB_KEYBOARD_ACCESS_STATE

/// Desired state of the keyboard LEDs as set over USB.
extern volatile uint8_t usb_keyboard_leds;

/// The buffer for keys currently pressed. Terminated by a zero, hence one
/// element more than required.
extern uint8_t usb_keys_buffer[MAX_KEY_ROLLOVER + 1];

/// Flags indicating which modifier keys are currently pressed. Note that if
/// multiple keys are mapped to the same modifier key, releasing either of
/// them causes the modifier to be released.
extern uint8_t usb_keys_modifier_flags;

/// Flags of extended keys (e.g., Apple Fn).
extern uint8_t usb_keys_extended_flags;

/// The selected keyboard protocol.
extern uint8_t usb_keyboard_protocol;

/// Are there changes to the pressed keys that we haven't sent?
extern volatile bool usb_keyboard_updated;

/// Are there currently no keys pressed?
#define usb_keyboard_is_idle                (usb_keys_buffer[0] == 0 && usb_keys_modifier_flags == 0 && usb_keys_extended_flags == 0)

#define usb_keyboard_is_in_boot_protocol    (usb_keyboard_protocol == HID_PROTOCOL_BOOT)

/// Effective rollover.
#define usb_keyboard_rollover               (usb_keyboard_is_in_boot_protocol ? USB_BOOT_PROTOCOL_ROLLOVER : USB_MAX_KEY_ROLLOVER)

// MARK: - Keyboard Errors

#define KEY_ERROR_NEEDS_REPORTING_FLAG  (1)

/// Error status of the keyboard (e.g., overflow). The least significant bit
/// is 1 if the error has not yet been sent to the host in a report.
extern volatile uint8_t key_error;

#endif

// MARK: - Virtual keys

#if ENABLE_APPLE_FN_KEY
#include "usb_keys.h"

#define APPLE_VIRTUAL_START     USB_KEY_VIRTUAL_APPLE_FN
#define APPLE_VIRTUAL_END       USB_KEY_VIRTUAL_APPLE_EXPOSE_DESKTOP
#define APPLE_VIRTUAL_MASK      (0xFFU)

#define IS_APPLE_VIRTUAL(key)   ((key) >= APPLE_VIRTUAL_START && (key) <= APPLE_VIRTUAL_END)

#define APPLE_VIRTUAL_BIT(key)  (1 << ((key) - APPLE_VIRTUAL_START))
#endif

#endif
