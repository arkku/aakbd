/**
 * keys.h: Key processing.
 *
 * This does all the work of managing layers, executing macros, handling
 * remapping, keeping track of modifiers, etc.
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

#ifndef KK_KEYS_H
#define KK_KEYS_H

#include <stdint.h>
#include <stdbool.h>

#ifndef ENABLE_KEYLOCK
/// Keylock enables mapping a key to `EXT(KEYLOCK)`, which causes the next key
/// that is pressed to be locked down until either the lock key or the locked
/// key itself is pressed again. This adds a very minimal amount of processing
/// (basically two `if` equality comparisons per keypress and 1 byte of RAM),
/// and is thus enabled by default.
#define ENABLE_KEYLOCK 1
#endif

// Standard USB LED bits (from the host computer).
#define LED_NUM_LOCK_BIT                (1 << 0)
#define LED_CAPS_LOCK_BIT               (1 << 1)
#define LED_SCROLL_LOCK_BIT             (1 << 2)
#define LED_COMPOSE_BIT                 (1 << 3)
#define LED_KANA_BIT                    (1 << 4)

// Virtual (locally set) LED bits. These just enable keyboards to show status
// LED indicators through the `keyboard_host_leds_changed()` callback.
#define LED_VIRTUAL_BOOT_PROTOCOL_BIT   (1 << 5)
#define LED_VIRTUAL_USB_ACTIVE_BIT      (1 << 6)
#define LED_VIRTUAL_USB_ERROR_BIT       (1 << 7)
#define LED_VIRTUAL_BIT_MASK            (LED_VIRTUAL_BOOT_PROTOCOL_BIT | LED_VIRTUAL_USB_ACTIVE_BIT | LED_VIRTUAL_USB_ERROR_BIT)

/// Processes the key. The argument `usbkey` must be a constant keycode to
/// _uniquely_ identify a specific physical key. Mapping keys should be done
/// via `layers.c` (and by extension `keys.c`).
///
/// Note that if multiple physical keys were to produce the same keycode passed
/// as argument to this function, various things would break. As such, all
/// key remapping _must_ be done using the facilities of this function and
/// things called by it (e.g., `layers.c` and `macros.c`).
void process_key(uint8_t usbkey, bool is_release);

/// Reset all key state.
void reset_keys(bool is_wake_up);

/// Keyboard error state, typically (almost exclusively) overflow.
uint8_t keys_error(void);

/// Reports an error state with the physical keyboard. The most common such
/// case is excess rollover, i.e., so many keys pressed that it's not possible
/// to correctly identify which ones. The current error state should be read
/// back from `usb_key_error()` since the error can also occur (and be cleared)
/// internally (e.g., `USB_MAX_KEY_ROLLOVER` exceeded).
void report_keyboard_error(bool is_rollover_error);

/// The LED state (USB host + overrides).
uint8_t keys_led_state(void);

/// Called periodically, approx. once every 10 ms, to process time-based
/// events. An 8-bit 10 ms tick count is passed as argument.
void keys_tick(uint8_t tick10ms_count);

#endif
