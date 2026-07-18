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
#include "keycodes.h"

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

/// See `process_key` for the preferred API to use most of the time!
///
/// This is like `process_key`, except it takes an already-resolved `keycode`
/// as an argument. This can be used to programmatically process keypresses
/// that are not tied to physical keys but it is desirable to have them go
/// through all the other key processing.
///
/// - Note: When processing generated keypresses, `physical_key` should be
/// zero and `keycode` non-zero. This prevents recording the virtual keypress
/// state, so any `*data` set in `preprocess_press()` will _not_ be preserved
/// to the `postprocess_release()` call!
void process_keycode(uint8_t physical_key, keycode_t keycode, int8_t action, uint8_t row, uint8_t col);

// Values for `process_keycode` `action`:

/// Key down (make).
#define PRESS           0

/// Key up (break).
#define RELEASE         1

/// Register the press, but do not call `preprocess_press`.
#define DEFERRED_PRESS  (-1)

/// Processes the key. The argument `usbkey` must be a constant keycode to
/// _uniquely_ identify a specific physical key. Mapping keys should be done
/// via `layers.c` (and by extension `keys.c`).
///
/// Note that if multiple physical keys were to produce the same keycode passed
/// as argument to this function, various things would break. As such, all
/// key remapping _must_ be done using the facilities of this function and
/// things called by it (e.g., `layers.c` and `macros.c`).
///
/// `row` and `col` are matrix coordinates used by optional features, such
/// as Vial. On non-matrix keyboards just pass `row=0` and `col=usbkey`.
/// On non-matrix keyboards, pass row=0, col=usbkey.
static inline void
process_key (uint8_t key, bool is_release, uint8_t row, uint8_t col) {
    process_keycode(key, PASS, is_release ? RELEASE : PRESS, row, col);
}

/// Reset all key state.
void reset_keys(bool is_wake_up);

/// Clear any persistent settings (e.g., EEPROM) saved by the keyboard. This
/// needs to be implemented by the specific device, and it may do nothing.
void keyboard_clear_settings(void);

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
#if VIAL_ENABLE
void keys_vial_task(void);
#endif

#if ENABLE_ONESHOT_KEYCODES
/// Tap count for the current one-shot key (used for tap toggle).
extern uint8_t oneshot_tap_count;

/// Timestamp of last one-shot layer press (for timeout).
extern uint8_t oneshot_layer_time;
#endif

#if VIAL_ENABLE
#include "vial_keys.h"

#define static_or_vial
static_or_vial uint8_t strong_modifiers_mask(void);
static_or_vial void add_weak_modifiers(const uint8_t mods);
static_or_vial void remove_weak_modifiers(const uint8_t mods);
static_or_vial void clear_weak_modifiers(void);
static_or_vial uint8_t weak_modifiers_mask(void);
static_or_vial void register_modifiers(void);
static_or_vial uint8_t current_base_layer(void);
static_or_vial void set_base_layer(const uint8_t num);
#else
#define static_or_vial static inline
#endif

#endif
