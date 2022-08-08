/**
 * macros.h: Facilities available to macros in `macros.c`.
 *
 * There are various cases that would be extremely complicated or impossible
 * impossible to represent in a 16-bit extended keycode. Those cases should
 * instead be implemented as macros, which are arbitrary pieces of code that
 * get run on key press and release. Each macro gets one byte of storage that
 * will persist from press to release, allowing it to store information about
 * its state. (Of course `macros.c` can also add `static` variables for more
 * storage if needed.)
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

#ifndef KK_MACROS_H
#define KK_MACROS_H

#include <stdint.h>
#include <stdbool.h>

#include "usbkbd.h"
#include "progmem.h"

#include "usb_hardware.h"
#include "usb_keys.h"
#include "keycodes.h"
#include "keys.h"
#include "aakbd.h"

#if LAYER_COUNT > 31
#error "The maximum LAYER_COUNT is 31."
#elif LAYER_COUNT > 16
typedef uint32_t layer_mask_t;
#define LAYER_MASK_BITS 32
#elif LAYER_COUNT > 8
typedef uint16_t layer_mask_t;
#define LAYER_MASK_BITS 16
#else
typedef uint8_t layer_mask_t;
#define LAYER_MASK_BITS 8
#endif

#define LED_NUM_LOCK        LED_NUM_LOCK_BIT
#define LED_CAPS_LOCK       LED_CAPS_LOCK_BIT
#define LED_SCROLL_LOCK     LED_SCROLL_LOCK_BIT

/// Register a key press or release. Note that you _must_ release each
/// key pressed this way, or it will be stuck!
static inline void register_key(const uint8_t key, const bool is_release);

/// Register a key press now, and automatically release it later, with the
/// given modifiers mask for the press.
static inline void register_press_and_release(const uint8_t key, const uint8_t mods);

/// Add "strong" modifiers that will persist until cleared. Strong modifiers
/// _must_ be cleared eventually or they will be stuck!
static inline void add_strong_modifiers(const uint8_t mods);

/// Remove the given set of strong modifiers.
static inline void remove_strong_modifiers(const uint8_t mods);

/// Clear all strong modifiers.
static inline void clear_strong_modifiers(void);

/// The currently active strong modifiers.
static inline uint8_t strong_modifiers_mask(void);

/// Add "strong" modifiers that will affect exactly the next keypress, after
/// which they will be cleared automatically.
static inline void add_weak_modifiers(const uint8_t mods);

/// Remove the given set of weak modifiers.
static inline void remove_weak_modifiers(const uint8_t mods);

/// Clear all weak modifiers. (Seldom necessary, since they are cleared
/// automatically on the next key down event.)
static inline void clear_weak_modifiers(void);

/// The currently active weak modifiers. Usually zero, since they are
/// cleared automatically on keypress.
static inline uint8_t weak_modifiers_mask(void);

/// Reset all layers to the default state.
static void reset_layers(void);

/// Enable the given layer number.
static inline void enable_layer(const uint8_t num);

/// Disable the given layer number.
static inline void disable_layer(const uint8_t num);

/// Toggle the given layer number on/off.
static inline void toggle_layer(const uint8_t num);

/// Is the given layer number active (excluding base layers)?
static inline bool is_layer_active(const uint8_t num);

/// The number of the highest active layer (ignoring base layer status)?
static inline uint8_t highest_active_layer(void);

/// The mask of currently active layers (ignoring base layer status).
static inline layer_mask_t active_layers_mask(void);

/// Set the mask of active layers.
static inline void set_active_layers_mask(const layer_mask_t mask);

/// Set only the given layer active, disabling all others.
static inline void set_active_layer(const uint8_t num);

/// Restore the previous layer state (after `set_active_layer` or
/// `set_active_layers_mask`).
static inline void restore_previous_layer_state(void);

/// Set the currently active base layer.
static inline void set_base_layer(const uint8_t num);

/// Restore the previous base layer from `set_base_layer`.
static inline void restore_previous_base_layer(void);

/// The current base layer.
static inline uint8_t current_base_layer(void);

/// Set the "pending keypress" flag. This is cleared automatically when a
/// key is pressed. This can be used to implement behaviour that differs
/// depending on whether other keys were pressed while a key was held down.
static inline void set_pending_keypress(const bool is_pending);

/// Is the "pending keypress" flag set?
static inline bool pending_keypress(void);

/// Force the given LEDs to be on regardless of what the USB host says.
static inline void add_override_leds_on(const uint8_t mask);

/// Remove "force on" overrides from the given LEDs.
static inline void remove_override_leds_on(const uint8_t mask);

/// Force the given LEDs to be off regardless of what the USB host says.
static inline void add_override_leds_off(const uint8_t mask);

/// Remove "force off" overrides from the given LEDs.
static inline void remove_override_leds_off(const uint8_t mask);

/// Clear all LED overrides.
static inline void clear_override_leds(void);

/// Is keylock enabled?
static inline bool is_keylock_enabled(void);

/// Called after resolving keycode from the currently active layers.
/// - Parameters:
///     - keycode: The resolved keycode of the key being pressed.
///     - physical_key: The keycode of the physical key pressed.
///     - data: Pointer to a single byte of storage that can be written to.
///       This byte is later passed on to `postprocess_release` and
///       `execute_macro` (in case the keycode is a macro).
/// - Returns: The new keycode to use for this keypress. Returning `NONE` will
///   prevent the key from having other effects except eventually calling
///   `postprocess_release` when released.
static inline keycode_t preprocess_press(keycode_t keycode, uint8_t physical_key, uint8_t * restrict data);

/// Called to handle custom macros.
/// - Parameters:
///     - macro_number: The macro number / enum case.
///     - is_release: `true` iff this is a key release.
///     - physical_key: The keycode of the physical key pressed.
///     - data: Pointer to a single byte of storage that the macro handler may
///       set to any value, which will persist unchanged from the key press to
///       release its release.
static void execute_macro(uint8_t macro_number, bool is_release, uint8_t physical_key, uint8_t * restrict data);

/// Called after all key release handlers have been called.
/// - Parameters:
///     - keycode: The keycode of the released key.
///     - physical_key: The keycode of the physical key pressed.
///     - data: The single byte of data associated with this keypress,
///       the same as can be written by `preprocess_press` and `execute_macro`.
static inline void postprocess_release(keycode_t keycode, uint8_t physical_key, uint8_t data);

/// Called after enabling or disabling a layer. This includes both base layers
/// and the active layers mask.
static inline void layer_state_changed(uint8_t layer, bool is_enabled);

/// Called when USB host LED state changes. This enables reacting to the
/// computer programmatically changing things like Num Lock (e.g., for
/// selecting numpad mappings accordingly).
/// - Parameter leds: The new USB host LED state. Does not include local
///   overrides or necessarily match the current state of the physical LEDs.
static inline void keyboard_host_leds_changed(uint8_t leds);

/// Called after reset. This can be used to customise the initial state of
/// layers, etc. For example, configuration could be loaded from EEPROM, etc.
static inline void handle_reset(void);

/// Called approximately once every 10 milliseconds with an 8-bit time value.
/// Long macros and simulated typing can cause this to be called less
/// frequently, since this is not an interrupt.
static inline void handle_tick(uint8_t tick_10ms_count);

#endif
