/**
 * ps2usb.h: A PS/2 to USB keyboard converter for ATMEGA32U4.
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

#ifndef KK_PS2USB_H
#define KK_PS2USB_H

#ifndef F_CPU
/// Crystal frequency.
#define F_CPU   16000000UL
#endif

#ifndef PS2USB_PREFERRED_KEYCODE_SET
/// The preferred keycode set (2 or 3) to use. 3 is better, but many cheaper
/// or newer PS/2 keyboards do not implement it. Set 2 has weird corner cases,
/// but is the common default.
#define PS2USB_PREFERRED_KEYCODE_SET 3
#endif

#ifndef SCROLL_LOCK_LED_ON_OVERFLOW
/// Blink the scroll lock LED on key overflow?
#define SCROLL_LOCK_LED_ON_OVERFLOW 1
#endif
#ifndef SCROLL_LOCK_LED_ON_SUSPEND
/// Occasionally blink scroll lock LED on suspend (sleep)?
#define SCROLL_LOCK_LED_ON_SUSPEND 1
#endif

#if PS2USB_PREFERRED_KEYCODE_SET == 3
#define PS2USB_FALLBACK_KEYCODE_SET 2
#elif PS2USB_PREFERRED_KEYCODE_SET == 2
#define PS2USB_FALLBACK_KEYCODE_SET 3
#else
#error "PS2USB_PREFERRED_KEYCODE_SET must be 2 or 3"
#endif

#ifndef PS2USB_DEBUG_SCANCODES
/// Set this to one so the keyboard uses simulated typing (which must be
/// enabled) to print the hex value of every keypress read, rather than
/// actually activate the key. This can be used to debug what code a keyboard
/// produces for each key.
#define PS2USB_DEBUG_SCANCODES 0
#endif

#ifndef PS2USB_DEBUG_COMMANDS
/// Further set this (after `PS2USB_DEBUG_SCANCODES`) to make certain keys
/// execute PS/2 commands. This can be used to check how the keyboard responds
/// to commands. Requires keycode set 3 currently (because most commands are
/// for set 3 anyway).
#define PS2USB_DEBUG_COMMANDS 0
#endif

#if !PS2USB_DEBUG_SCANCODES && PS2USB_DEBUG_COMMANDS
#undef PS2USB_DEBUG_SCANCODES
#define PS2USB_DEBUG_SCANCODES 1
#warning PS2USB_DEBUG_COMMANDS requires PS2USB_DEBUG_SCANCODES
#endif

#endif
