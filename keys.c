/**
 * keys.c: Key processing.
 *
 * This does all the work of managing layers, executing macros, handling
 * remapping, keeping track of modifiers, etc.
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
#define KEYS_C_START // guards against incorrectly doing #include "macros.h"
#include <assert.h>
#include "aakbd.h"

#define USB_KEYBOARD_ACCESS_STATE 1
#include "usbkbd.h"
#include "keys.h"
#include "usb_keys.h"
#include "keycodes.h"

#if VIAL_ENABLE
#include "vial.h"
#include "vial_magic.h"
#endif

#undef KEYS_C_START
#ifndef LAYERS_INCLUDE
#define LAYERS_INCLUDE <layers.c>
#endif
#ifndef MACROS_INCLUDE
#define MACROS_INCLUDE <macros.c>
#endif

#include LAYERS_INCLUDE

#ifndef DEFAULT_BASE_LAYER
#define DEFAULT_BASE_LAYER 1
#endif

#if !defined(LAYER_COUNT) && VIAL_ENABLE
#define LAYER_COUNT VIAL_LAYER_COUNT
#endif

#if LAYER_COUNT > 0
/// The bit for layer `num` is the layer mask.
#define layer_bit(num)          (((layer_mask_t) 1) << ((num) - 1))

/// Is the layer `num` active? (base layer, or above base layer and enabled)
#define is_layer_active(num)    ((num) >= base_layer && (((num) == base_layer) || (layer_mask & layer_bit(num))))

#include MACROS_INCLUDE

/// The current base layer number.
static uint8_t base_layer = DEFAULT_BASE_LAYER;
static uint8_t previous_base_layer = DEFAULT_BASE_LAYER;

/// The bitmask for active layers.
static layer_mask_t layer_mask = 0;
static layer_mask_t previous_layer_mask = 0;

/// "Strong" modifiers mask. These are real modifier keys that are active as
/// long as they are held.
static uint8_t strong_modifiers = 0;

/// "Weak" modififers mask. These are modifiers not set by the actual modifier
/// keys, but rather mapped from extended keycodes, such as for keys that
/// simulate the actual key + a modifier. Weak modifiers are cleared whenever
/// another key is pressed, so that they don't influence them.
static uint8_t weak_modifiers = 0;

#endif // ^ LAYER_COUNT > 0

_Static_assert(LAYER_COUNT >= VIAL_LAYER_COUNT);
_Static_assert(EXT_KEYCODE_COUNT <= 64);

#if LAYER_COUNT > 1
// To simplify code elsewhere, define arrays of 1 element for every layer,
// it's only a few dozen wasted bytes at worst (and in practice it will get
// optimised away by the compiler)
#if VIAL_LAYER_COUNT >= 1
DEFINE_EMPTY_LAYER(1);
#endif
#if LAYER_COUNT < 2 || VIAL_LAYER_COUNT >= 2
DEFINE_EMPTY_LAYER(2);
#endif
#if LAYER_COUNT < 3 || VIAL_LAYER_COUNT >= 3
DEFINE_EMPTY_LAYER(3);
#endif
#if LAYER_COUNT < 4 || VIAL_LAYER_COUNT >= 4
DEFINE_EMPTY_LAYER(4);
#endif
#if LAYER_COUNT < 5 || VIAL_LAYER_COUNT >= 5
DEFINE_EMPTY_LAYER(5);
#endif
#if LAYER_COUNT < 6 || VIAL_LAYER_COUNT >= 6
DEFINE_EMPTY_LAYER(6);
#endif
#if LAYER_COUNT < 7 || VIAL_LAYER_COUNT >= 7
DEFINE_EMPTY_LAYER(7);
#endif
#if LAYER_COUNT < 8 || VIAL_LAYER_COUNT >= 8
DEFINE_EMPTY_LAYER(8);
#endif
#if LAYER_COUNT > 8
#if VIAL_LAYER_COUNT >= 9
DEFINE_EMPTY_LAYER(9);
#endif
#if LAYER_COUNT < 10 || VIAL_LAYER_COUNT >= 10
DEFINE_EMPTY_LAYER(10);
#endif
#if LAYER_COUNT < 11 || VIAL_LAYER_COUNT >= 11
DEFINE_EMPTY_LAYER(11);
#endif
#if LAYER_COUNT < 12 || VIAL_LAYER_COUNT >= 12
DEFINE_EMPTY_LAYER(12);
#endif
#if LAYER_COUNT < 13 || VIAL_LAYER_COUNT >= 13
DEFINE_EMPTY_LAYER(13);
#endif
#if LAYER_COUNT < 14 || VIAL_LAYER_COUNT >= 14
DEFINE_EMPTY_LAYER(14);
#endif
#if LAYER_COUNT < 15 || VIAL_LAYER_COUNT >= 15
DEFINE_EMPTY_LAYER(15);
#endif
#if LAYER_COUNT < 16 || VIAL_LAYER_COUNT >= 16
DEFINE_EMPTY_LAYER(16);
#endif
#if LAYER_COUNT > 16
#if VIAL_LAYER_COUNT >= 17
DEFINE_EMPTY_LAYER(17);
#endif
#if LAYER_COUNT < 18 || VIAL_LAYER_COUNT >= 18
DEFINE_EMPTY_LAYER(18);
#endif
#if LAYER_COUNT < 19 || VIAL_LAYER_COUNT >= 19
DEFINE_EMPTY_LAYER(19);
#endif
#if LAYER_COUNT < 20 || VIAL_LAYER_COUNT >= 20
DEFINE_EMPTY_LAYER(20);
#endif
#if LAYER_COUNT < 21 || VIAL_LAYER_COUNT >= 21
DEFINE_EMPTY_LAYER(21);
#endif
#if LAYER_COUNT < 22 || VIAL_LAYER_COUNT >= 22
DEFINE_EMPTY_LAYER(22);
#endif
#if LAYER_COUNT < 23 || VIAL_LAYER_COUNT >= 23
DEFINE_EMPTY_LAYER(23);
#endif
#if LAYER_COUNT < 24 || VIAL_LAYER_COUNT >= 24
DEFINE_EMPTY_LAYER(24);
#endif
#if LAYER_COUNT < 25 || VIAL_LAYER_COUNT >= 25
DEFINE_EMPTY_LAYER(25);
#endif
#if LAYER_COUNT < 26 || VIAL_LAYER_COUNT >= 26
DEFINE_EMPTY_LAYER(26);
#endif
#if LAYER_COUNT < 27 || VIAL_LAYER_COUNT >= 27
DEFINE_EMPTY_LAYER(27);
#endif
#if LAYER_COUNT < 28 || VIAL_LAYER_COUNT >= 28
DEFINE_EMPTY_LAYER(28);
#endif
#if LAYER_COUNT < 29 || VIAL_LAYER_COUNT >= 29
DEFINE_EMPTY_LAYER(29);
#endif
#if LAYER_COUNT < 30 || VIAL_LAYER_COUNT >= 30
DEFINE_EMPTY_LAYER(30);
#endif
#if LAYER_COUNT < 31 || VIAL_LAYER_COUNT >= 31
DEFINE_EMPTY_LAYER(31);
#endif
#endif // LAYER_COUNT > 16
#endif // LAYER_COUNT > 8
#endif // LAYER_COUNT > 1

#ifndef MAX_REMAPPED_KEY_ROLLOVER
/// How many _remapped_ keys can be pressed at once?
#define MAX_REMAPPED_KEY_ROLLOVER 8
#endif

struct key_source {
    uint8_t key;
    uint8_t data;
    uint16_t keycode;
};

/// A list of source layers for keys where the source layer differs from the
/// default keycode. This allows matching key releases to the correct layer,
/// even if layer states have changed.
static struct key_source keybuffer[MAX_REMAPPED_KEY_ROLLOVER + 1] = { { 0, 0, 0 } };

#if ENABLE_KEYLOCK
/// Keylock state, either 0, the key that is locked, or `KEYLOCK_ARMED`.
static uint8_t keylock_key = 0;

#define KEYLOCK_ARMED       (0xFFU)

#define is_keylock_armed    (keylock_key == KEYLOCK_ARMED)
#define arm_keylock()       do { keylock_key = KEYLOCK_ARMED; } while (0)

static inline bool
is_keylock_enabled (void) {
    return keylock_key != 0;
}
#else
static inline bool
is_keylock_enabled (void) {
    return false;
}
#endif // ^ ENABLE_KEYLOCK

#define get_key_from_static_layer(key, num) (       \
    (sizeof(keycode_t) == 1) ?                      \
        pgm_read_byte(LAYER_ARRAY(num) + (key)) :   \
        pgm_read_word(LAYER_ARRAY(num) + (key))     \
)

#define is_key_in_static_layer(key, num)   ((key) < LAYER_SIZE(num))

#if VIAL_ENABLE
#define get_key_from_layer(key, num) \
    ((num) <= VIAL_LAYER_COUNT \
        ? vial_get_keycode_for_physical_key((key), (num)) \
        : (is_key_in_static_layer((key), num) ? get_key_from_static_layer((key), num) : 0) \
    )

#define set_keycode_from_layer(key, num, row, col) do {                     \
    if (keycode == 0 && is_layer_active(num)) { \
        if (num <= VIAL_LAYER_COUNT) { \
            keycode = vial_get_keycode_at((row), (col), (num)); \
        } else if (is_key_in_static_layer((key), num)) { \
            keycode = get_key_from_static_layer((key), num); \
        } \
        if (keycode != 0) { layer = (num); } \
    } \
} while (0)
#else // ^ VIAL_ENABLE
#define get_key_from_layer(key, num) \
    (is_key_in_static_layer((key), num) ? get_key_from_static_layer((key), num) : 0)

#define set_keycode_from_layer(key, num, row, col) do { \
    if (keycode == 0 && is_key_in_static_layer((key), num) && is_layer_active(num)) { \
        keycode = get_key_from_static_layer((key), num); \
        if (keycode != 0) { layer = (num); } \
    } \
} while (0)
#endif

#define layer_enabled(num)  layer_state_changed((num), true)
#define layer_disabled(num) layer_state_changed((num), false)

/// If we have a simulated keypress in progress, this is the pending keycode
/// to release.
static uint8_t pending_release = 0;

/// The tick since which `pending_release` has been pending.
static uint8_t pending_release_since = 0;

static inline void
send_pending_release (void) {
    usb_keyboard_release(pending_release);
    pending_release = 0;
    (void) usb_keyboard_send_if_needed();
}

static inline void
set_pending_release (const uint8_t key) {
    pending_release = key;
    pending_release_since = current_10ms_tick_count();
}

#if LAYER_COUNT > 0

static_or_vial void
set_base_layer (const uint8_t num) {
    previous_base_layer = base_layer;
    if (base_layer == num) {
        return;
    }
    base_layer = num;

    for (int_fast8_t i = previous_base_layer; i < num; ++i) {
        // Active layers with a lower number than the new base became disabled
        if (i == previous_base_layer || is_layer_enabled(i)) {
            layer_disabled(i);
        }
    }

    if (previous_base_layer > num) {
        layer_enabled(num);

        for (int_fast8_t i = num + 1; i < previous_base_layer; ++i) {
            // Active layers with a higher number than the new base became enabled
            if (is_layer_enabled(i)) {
                layer_enabled(i);
            }
        }
        if (!is_layer_enabled(previous_base_layer)) {
            // The previous base can still have become disabled
            layer_disabled(previous_base_layer);
        }
    } else if (!is_layer_enabled(num)) {
        layer_enabled(num);
    }
}

static inline void
restore_previous_base_layer (void) {
    set_base_layer(previous_base_layer);
}

static inline bool
is_layer_enabled (const uint8_t num) {
    return (layer_mask & layer_bit(num)) != 0;
}

static inline uint8_t
highest_active_layer (void) {
    if (layer_mask == 0) {
        return 0;
    }
#if LAYER_MASK_BITS <= 16
    return (LAYER_MASK_BITS - ((sizeof(unsigned int) * 8) - LAYER_MASK_BITS)) - __builtin_clz((unsigned int) layer_mask);
#else
    return (LAYER_MASK_BITS - ((sizeof(unsigned long) * 8) - LAYER_MASK_BITS)) - __builtin_clzl((unsigned long) layer_mask);
#endif
}

static inline layer_mask_t
active_layers_mask (void) {
    return layer_mask;
}

static_or_vial uint8_t
current_base_layer (void) {
    return base_layer;
}

static_or_vial void
register_modifiers (void) {
    usb_keyboard_set_modifiers(strong_modifiers_mask() | weak_modifiers_mask());
}

static_or_vial void
clear_weak_modifiers (void) {
    weak_modifiers = 0U;
}

static inline void
clear_strong_modifiers (void) {
    strong_modifiers = 0U;
}

static_or_vial void
add_weak_modifiers (const uint8_t mods) {
    weak_modifiers |= mods;
}

static_or_vial void
remove_weak_modifiers (const uint8_t mods) {
    weak_modifiers &= ~(mods);
}

static_or_vial uint8_t
weak_modifiers_mask (void) {
    return weak_modifiers;
}

static_or_vial void
add_strong_modifiers (const uint8_t mods) {
    strong_modifiers |= mods;
}

static_or_vial void
remove_strong_modifiers (const uint8_t mods) {
    strong_modifiers &= ~(mods);
}

static_or_vial uint8_t
strong_modifiers_mask (void) {
    return strong_modifiers;
}

static inline void
enable_layer (const uint8_t num) {
    const layer_mask_t bit = layer_bit(num);
    if (!(layer_mask & bit)) {
        layer_mask |= bit;
        if (num > base_layer) {
            layer_enabled(num);
        }
    }
}

static inline void
disable_layer (const uint8_t num) {
    const layer_mask_t bit = layer_bit(num);
    if (layer_mask & bit) {
        layer_mask &= ~bit;
        if (num > base_layer) {
            layer_disabled(num);
        }
    }
}

static inline void
toggle_layer (const uint8_t num) {
    const layer_mask_t bit = layer_bit(num);
    layer_mask ^= layer_bit(num);
    if (num > base_layer) {
        if (layer_mask & bit) {
            layer_enabled(num);
        } else {
            layer_disabled(num);
        }
    }
}

static inline void
set_active_layer (const uint8_t num) {
    const layer_mask_t bit = ((num == 0) ? num : layer_bit(num));
    set_active_layers_mask(bit);
}

static inline void
set_active_layers_mask (const layer_mask_t mask) {
    previous_layer_mask = layer_mask;
    if (layer_mask == mask) {
        return;
    }
    layer_mask = mask;

    const layer_mask_t enabled_layers = mask & ~previous_layer_mask;
    const layer_mask_t disabled_layers = previous_layer_mask & ~mask;

    for (int_fast8_t i = base_layer + 1; i <= LAYER_COUNT; ++i) {
        const layer_mask_t bit = layer_bit(i);
        if (disabled_layers & bit) {
            layer_disabled(i);
        } else if (enabled_layers & bit) {
            layer_enabled(i);
        }
    }
}

static inline void
restore_previous_layer_state (void) {
    set_active_layers_mask(previous_layer_mask);
}

static inline keycode_t keycode_from_layer(uint8_t key, uint8_t num);

/// Is a dual-action key currently being held down such that another keypress
/// during that time will trigger the alternative action? For example, a
/// key that works as Ctrl when held but sends Esc if only tapped without
/// another keypress.
static bool is_pending_keypress = false;

static inline void
set_pending_keypress (const bool is_pending) {
    is_pending_keypress = is_pending;
}

static inline bool
pending_keypress (void) {
    return is_pending_keypress;
}

static void
register_press_and_release (const uint8_t key, const uint8_t mods) {
    if (pending_release) {
        // Clear any previous release if there was one pending.
        // (Doesn't happen in this file, but macros have access to this.)
        send_pending_release();
    }
    usb_keyboard_set_modifiers(mods);
    (void) usb_keyboard_send_if_needed();
    usb_keyboard_press(key);
    (void) usb_keyboard_send_if_needed();
    // The release will be handled later to give some time for it to register.
    set_pending_release(key);
}

static inline void
send_pending_key_down (const uint8_t key) {
    set_pending_keypress(false);
    // Use strong modifiers only since this is only used in contexts where
    // the weak modifiers are not intended for this key
    register_press_and_release(key, strong_modifiers_mask());
}

static void
reset_layers (void) {
    set_active_layers_mask(0);
    previous_layer_mask = 0;
    set_base_layer(DEFAULT_BASE_LAYER);
    previous_base_layer = DEFAULT_BASE_LAYER;
    set_pending_keypress(false);
}

static void
register_key (const uint8_t key, const bool is_release) {
    if (is_release) {
        if (IS_MODIFIER(key)) {
            remove_strong_modifiers(MODIFIER_BIT(key));
        } else if (key) {
            usb_keyboard_release(key);
        }
    } else {
        if (IS_MODIFIER(key)) {
            add_strong_modifiers(MODIFIER_BIT(key));
        } else if (key) {
            register_modifiers();
            usb_keyboard_press(key);
        }
    }
    register_modifiers();
}
#endif // ^ LAYER_COUNT > 0

void
report_keyboard_error (bool is_rollover_error) {
    usb_keyboard_press(is_rollover_error ? USB_KEY_ROLLOVER : USB_KEY_UNDEFINED_ERROR);
}

#if VIAL_ENABLE
uint8_t grave_esc_override_mask = 0;
#else
#define grave_esc_override_mask GRAVE_ESC_OVERRIDE_MASK
#endif

#if ENABLE_ONESHOT_KEYCODES
#if VIAL_ENABLE
/// Runtime one-shot tap toggle (loaded from EEPROM).
uint8_t oneshot_tap_toggle = ONESHOT_TAP_TOGGLE;

/// Runtime one-shot timeout in ms (loaded from EEPROM).
uint16_t oneshot_timeout_ms = ONESHOT_TIMEOUT_MS;
#else

#define oneshot_timeout_ms ONESHOT_TIMEOUT_MS
#define oneshot_tap_toggle ONESHOT_TAP_TOGGLE

#endif

_Static_assert(ONESHOT_TIMEOUT_MS <= ONESHOT_TIMEOUT_MS_MAX, "ONESHOT_TIMEOUT_MS too large");

/// One-shot modifier bitmask.
static uint8_t oneshot_mods = 0;

/// One-shot layer (1-31) that is temporarily activated. 0 = inactive.
static uint8_t oneshot_layer = 0;

/// Command used to activate the one-shot (1-5), for undo on consumption.
static uint8_t oneshot_command = 0;

/// Tap count for the current one-shot key (used for tap toggle).
uint8_t oneshot_tap_count = 0;

/// Timestamp of last one-shot layer press (for timeout).
uint8_t oneshot_layer_time = 0;

// Apply a one-shot layer command: execute command c on layer n.
static void
oneshot_apply (uint8_t layer, uint8_t command) {
    switch (command) {
    case CMD_LAYER_TOGGLE:
        toggle_layer(layer);
        return;
    case CMD_LAYER_DISABLE:
        disable_layer(layer);
        return;
    case CMD_LAYER_ENABLE:
        enable_layer(layer);
        return;
    case CMD_LAYER_SET_MASK:
        set_active_layer(layer);
        return;
    case CMD_LAYER_SET_BASE:
        set_base_layer(layer);
        return;
    }
}

// Undo the previous one-shot layer command.
static void
restore_oneshot_layer (void) {
    uint8_t undo = oneshot_command;
    switch (undo) {
    case CMD_LAYER_ENABLE:
        undo = CMD_LAYER_DISABLE;
        break;
    case CMD_LAYER_DISABLE:
        undo = CMD_LAYER_ENABLE;
        break;
    case CMD_LAYER_SET_MASK:
        restore_previous_layer_state();
        return;
    case CMD_LAYER_SET_BASE:
        restore_previous_base_layer();
        return;
    default:
        break;
    }
    oneshot_apply(oneshot_layer, undo);
}
#endif

#if ENABLE_TRI_LAYER
static void
update_tri_layer (void) {
    if (is_layer_enabled(2) && is_layer_enabled(3)) {
        enable_layer(4);
    } else {
        disable_layer(4);
    }
}
#endif

void
process_keycode (const uint8_t physical_key, keycode_t keycode, int8_t action, uint8_t row, uint8_t col) {
#if LAYER_COUNT > 0
    uint8_t layer = 0;
    uint8_t key = physical_key;
    uint8_t data_or_index = 0;
    const bool was_pending_keypress = pending_keypress();
    bool is_release = (action > PRESS);

    if (pending_release) {
        // If we are pending some timed key release, release it now since
        // otherwise the timing between it and this key event will be wrong.
        send_pending_release();
    }

    if (is_release) {
#if ENABLE_KEYLOCK
        if (keylock_key && physical_key == keylock_key) {
            // Do not release the locked key
            return;
        }
#endif

#if VIAL_ENABLE && VIAL_COMBO_COUNT > 0
        if (physical_key && combo_handle_release(physical_key, &keycode, &data_or_index)) {
            // The keycode has been consumed by the combo, do not release the key
            goto postprocess;
        }
#endif

        if (!keycode && physical_key) {
            // Find the keycode corresponding to the original press of this key,
            // which might differ with the current layer activation state.
            int_fast8_t ri = 0, wi = 0;
            do {
                if (keybuffer[ri].key != key) {
                    keybuffer[wi].key = keybuffer[ri].key;
                    keybuffer[wi].data = keybuffer[ri].data;
                    keybuffer[wi].keycode = keybuffer[ri].keycode;
                    ++wi;
                } else {
                    keycode = keybuffer[ri].keycode;
                    data_or_index = keybuffer[ri].data;
                }
                ++ri;
            } while (keybuffer[wi].key);
        }
    } else {
#if ENABLE_KEYLOCK
        if (keylock_key && physical_key == keylock_key) {
            // The locked key was pressed again, unlock it.
            // (It will be released when this new press is released.)
            keylock_key = 0;
            return;
        }
#endif
#if LAYER_COUNT > 1
        // The reason this is unrolled is that we get more compile-time
        // constants like `sizeof` here, and the compiler is more likely to
        // optimise away unused layers (which is usually most of them).
#if LAYER_MASK_BITS > 16
        set_keycode_from_layer(key, 31, row, col);
        set_keycode_from_layer(key, 30, row, col);
        set_keycode_from_layer(key, 29, row, col);
        set_keycode_from_layer(key, 28, row, col);
        set_keycode_from_layer(key, 27, row, col);
        set_keycode_from_layer(key, 26, row, col);
        set_keycode_from_layer(key, 25, row, col);
        set_keycode_from_layer(key, 24, row, col);
        set_keycode_from_layer(key, 23, row, col);
        set_keycode_from_layer(key, 22, row, col);
        set_keycode_from_layer(key, 21, row, col);
        set_keycode_from_layer(key, 20, row, col);
        set_keycode_from_layer(key, 19, row, col);
        set_keycode_from_layer(key, 18, row, col);
        set_keycode_from_layer(key, 17, row, col);
#endif
#if LAYER_MASK_BITS > 8
        set_keycode_from_layer(key, 16, row, col);
        set_keycode_from_layer(key, 15, row, col);
        set_keycode_from_layer(key, 14, row, col);
        set_keycode_from_layer(key, 13, row, col);
        set_keycode_from_layer(key, 12, row, col);
        set_keycode_from_layer(key, 11, row, col);
        set_keycode_from_layer(key, 10, row, col);
        set_keycode_from_layer(key, 9, row, col);
#endif
        set_keycode_from_layer(key, 8, row, col);
        set_keycode_from_layer(key, 7, row, col);
        set_keycode_from_layer(key, 6, row, col);
        set_keycode_from_layer(key, 5, row, col);
        set_keycode_from_layer(key, 4, row, col);
        set_keycode_from_layer(key, 3, row, col);
        set_keycode_from_layer(key, 2, row, col);
#endif // ^ LAYER_COUNT > 1
        set_keycode_from_layer(key, 1, row, col);

#if VIAL_ENABLE
        if (keycode > 0 && keycode <= MODIFIERS_END) {
            keycode = vial_magic_remap_key((uint8_t) keycode);
        }
#endif

#if VIAL_ENABLE && VIAL_COMBO_COUNT > 0
        if (action == PRESS)
#endif
        keycode = preprocess_press(keycode, physical_key, layer, &data_or_index);

        if (keycode == PASS) {
            keycode = key;
        }

#if VIAL_ENABLE && VIAL_TAP_DANCE_COUNT > 0
        if (physical_key) {
            vial_tap_dance_interrupt();
        }
#endif
#if VIAL_ENABLE && VIAL_COMBO_COUNT > 0
        if (physical_key && action == PRESS) {
            if (combo_handle_press(keycode, physical_key, row, col, data_or_index)) {
                goto postprocess;
            }
        }
#endif

        if ((keycode != key || data_or_index != 0) && physical_key) {
            // The key differs from the physical key, so we need to record
            // the keycode so that it will be correctly released even if the
            // layer configuration changes before then. Since we only do this
            // for keys that differ from the physical key, the list we have
            // to maintain should normally be much shorter than the full set
            // of pressed keys.
            int_fast8_t i = 0;
            while (keybuffer[i].key && keybuffer[i].key != key) {
                ++i;
            }
            if (i == MAX_REMAPPED_KEY_ROLLOVER || (key && keybuffer[i].key == key)) {
                usb_keyboard_press(KEY_ROLLOVER_ERROR_CODE);
                is_release = true;
                goto postprocess;
            }
            keybuffer[i].key = key;
            keybuffer[i].data = data_or_index;
            keybuffer[i].keycode = keycode;
            data_or_index = i;
        }
    }

    if (keycode == PASS) {
        // No mapping, pass to default
        keycode = key;
    } else {
        key = PLAIN_KEY_OF(keycode);
    }

    if (!is_release) {
        // Weak modifiers should not affect presses other than the actual
        // key itself.
        clear_weak_modifiers();
        set_pending_keypress(false);

#if ENABLE_KEYLOCK
        if (is_keylock_armed && physical_key) {
            // Keylock was armed, lock this key down
            keylock_key = physical_key;
        }
#endif
    }

    if (keycode == NONE) {
        // Special keycode to do nothing
        goto postprocess;
    } else if (is_extended_keycode(keycode)) {
        uint8_t mods = MODIFIERS_OF_EXTENDED(keycode);
        uint8_t command = COMMAND_OF(keycode);

        // Double modifiers like `CTRL(ALT)` needs both modifiers to
        // be strong or the second one would be cancelled by the next
        // keypress, which would make the combo useless.
        bool is_strong_modifier = (mods && IS_MODIFIER(key));

        if (command) {
            if (command == CMD_MODIFIER_OR_KEY) {
#if ENABLE_ONESHOT_KEYCODES
                if (!MODIFIERS_OF_EXTENDED(keycode)) {
                    // Zero modifiers = one-shot modifier
                    if (!is_release) {
                        oneshot_mods |= keycode & 0xFF;
                        key = PASS;
                    }
                } else
#endif
                {
                    // The modifier must be strong to have effect
                    is_strong_modifier = true;
                    if (is_release) {
                        mods = data_or_index; // Undo only the actual mods we added
                        if (was_pending_keypress) {
                            // No other key was pressed, act as a keypress
                            remove_strong_modifiers(mods);
                            send_pending_key_down(key);
                        }
                    } else {
                        // Don't affect the same modifiers if they are already
                        // present so that this can be used for the key and that
                        // modifier from another key.
                        mods &= ~strong_modifiers_mask();
                        keybuffer[data_or_index].data = mods;
                        set_pending_keypress(true);
                    }
                    key = PASS;
                }
            } else {
                // Layer command
                uint8_t modifier;

                if (command == CMD_LAYER_OR_KEY) {
                    // A special case where the layer number isn't in the
                    // keycode section since there is an actual keycode
                    if (is_release && was_pending_keypress) {
                        send_pending_key_down(key);
                    } else {
                        set_pending_keypress(!is_release);
                    }
                    layer = LAYER_OF_LAYER_OR_KEY(keycode);
                    mods = 0; // The layer is in the modifier bits
                    modifier = ACT_ON_HOLD;
                    command = CMD_LAYER_TOGGLE;
                } else {
                    layer = LAYER_OF_COMMAND(keycode);
                    is_strong_modifier = true; // Need strength to have effect
                    modifier = LAYER_CMD_MODIFIER_OF(keycode);
                }
                key = PASS; // The key part of this keycode is not a key

                if (layer >= 0 && layer <= LAYER_COUNT) {
                    // Layer modifying command

                    switch (modifier) {
                    case ACT_ON_HOLD:
                        action = is_release ? -1 : 1;
                        break;
                    case ACT_ON_RELEASE:
                        action = is_release ? 1 : 0;
                        break;
                    case ACT_ON_PRESS:
                        action = is_release ? 0 : 1;
                        break;
                    case ACT_IF_NO_KEYPRESS:
                        if (is_release) {
                            action = was_pending_keypress ? 1 : 0;
                        } else {
                            action = 0;
                            set_pending_keypress(true);
                        }
                        break;
                    case ACT_ON_HOLD_KEEP_IF_NO_KEYPRESS:
                        if (is_release) {
                            action = was_pending_keypress ? 0 : -1;
                        } else {
                            action = 1;
                            set_pending_keypress(true);
                        }
                        break;
#if ENABLE_ONESHOT_KEYCODES
                    case ACT_ONESHOT:
                        if (!is_release) {
                            if (oneshot_tap_toggle > 1 &&
                                oneshot_layer == layer &&
                                oneshot_tap_count + 1 >= oneshot_tap_toggle) {
                                // Tap count threshold reached — lock layer
                                oneshot_layer = 0;
                                oneshot_tap_count = 0;
                                goto postprocess;
                            }
                            if (oneshot_layer && oneshot_layer != layer) {
                                restore_oneshot_layer();
                            }
                            oneshot_layer = layer;
                            oneshot_command = command;
                            if (oneshot_tap_toggle > 1) {
                                ++oneshot_tap_count;
                            }
                            if (oneshot_timeout_ms) {
                                oneshot_layer_time = current_10ms_tick_count();
                            }
                            action = 1;
                        } else {
                            if (oneshot_timeout_ms) {
                                oneshot_layer_time = current_10ms_tick_count();
                            }
                            action = 0;
                        }
                        break;
#endif
                    default:
                        action = 0;
                        break;
                    }

                    if (action && (layer || command == CMD_LAYER_SET_BASE || command == CMD_LAYER_SET_MASK)) {
                        switch (command) {
                        case CMD_LAYER_DISABLE:
                            action = -action;
                            // fallthrough
                        case CMD_LAYER_ENABLE:
                            if (action == 1) {
                                enable_layer(layer);
                            } else {
                                disable_layer(layer);
                            }
                            break;
                        case CMD_LAYER_TOGGLE:
                            toggle_layer(layer);
                            break;
                        case CMD_LAYER_SET_MASK:
                            if (action == 1) {
                                set_active_layer(layer);
                            } else {
                                restore_previous_layer_state();
                            }
                            break;
                        case CMD_LAYER_SET_BASE:
                            if (action == 1) {
                                set_base_layer(layer);
                            } else {
                                restore_previous_base_layer();
                            }
                            break;
                        default:
                            break;
                        }
                    }
                }
            }
        } else if (!mods) {
            // Make sure to update modifier state before any macros/etc.
            register_modifiers();

            // Extended keycode that is not a modifier or a command
            if (extended_keycode_is_macro(key)) {
#if VIAL_ENABLE
                uint8_t macro_num = MACRO_OF_EXTENDED(key);
                if (macro_num >= VIAL_MACRO_START) {
#if VIAL_MACRO_COUNT > 0
                    if (physical_key && !is_release) {
                        // Don't allow recursive Vial macros
                        dynamic_keymap_macro_send(macro_num - VIAL_MACRO_START);
                    }
#endif
                } else {
                    execute_macro(
                        macro_num,
                        is_release,
                        physical_key,
                        layer,
                        is_release ? &data_or_index : &(keybuffer[data_or_index].data)
                    );
                }
#else
                execute_macro(
                    MACRO_OF_EXTENDED(key),
                    is_release,
                    physical_key,
                    layer,
                    is_release ? &data_or_index : &(keybuffer[data_or_index].data)
                );
#endif
                goto postprocess;
            } else if (extended_keycode_is_exact_modifiers(key)) {
                is_strong_modifier = true;
                if (is_release) {
                    // Unset the added modifiers - we can't restore the old
                    // modifiers since those keys might have been already
                    // released, which would leave their modifiers stuck
                    mods = data_or_index;
                } else {
                    mods = EXACT_MODIFIERS_OF_EXTENDED(key);
                    // Store the modifiers we added with this key
                    keybuffer[data_or_index].data = mods & ~strong_modifiers_mask();
                    // Set exactly these modifiers, nothing else
                    clear_strong_modifiers();
                }
            } else {
                const enum extended_keycode extended_key = key;

                // Built-in extended keycode
                switch (extended_key) {
                case EXT_RESET_KEYBOARD:
                    if (is_release) {
                        keyboard_reset();
                    }
                    break;
                case EXT_ENTER_BOOTLOADER:
                    jump_to_bootloader();
                    break;
                case EXT_EEPROM_RESET:
                    if (is_release) {
                        keyboard_clear_settings();
                    }
                    break;
                case EXT_GRAVE_ESCAPE:
                    if (is_release) {
                        key = data_or_index;
                    } else {
                        mods = strong_modifiers_mask() | weak_modifiers_mask();
                        key = ((mods & (BOTH_SHIFT_BITS | CMD_BIT | RIGHT_CMD_BIT)) && !(mods & grave_esc_override_mask)) ? KEY(BACKTICK) : KEY(ESC);
                        keybuffer[data_or_index].data = key;
                        mods = 0;
                    }
                    goto extended_key_fallthrough;
#if ENABLE_EXT_HYPER_MEH
                case EXT_HYPER_MODIFIERS:
                    mods |= CMD_BIT;
                    // fallthrough
                case EXT_MEH_MODIFIERS:
                    if (is_release) {
                        mods = data_or_index;
                    } else {
                        mods |= SHIFT_BIT | CTRL_BIT | ALT_BIT;
                        mods = mods & ~strong_modifiers_mask();
                        keybuffer[data_or_index].data = mods;
                    }
                    is_strong_modifier = true;
                    break;
#endif
#if ENABLE_KEYLOCK
                case EXT_KEYLOCK:
                    if (!is_release) {
                        if (keylock_key) {
                            // Keylock pressed again, release the locked key
                            key = keylock_key;
                            keylock_key = 0;
                            if (key != physical_key) {
                                process_key(key, true, row, col);
                            }
                        } else {
                            arm_keylock();
                        }
                    }
                    break;
#endif
                case EXT_TOGGLE_BOOT_PROTOCOL:
                    if (is_release) {
                        usb_keyboard_toggle_boot_protocol();
                    }
                    break;
#if ENABLE_SIMULATED_TYPING
                case EXT_PRINT_DEBUG_INFO:
                    if (!is_release) {
                        usb_keyboard_type_debug_report();
                    }
                    break;
#endif
#if VIAL_ENABLE
                case EXT_QMK_KEYCODE:
                    if (!is_release) {
                        keybuffer[data_or_index].data = layer;
                    }
                    vial_process_qmk_keycode(row, col, is_release ? data_or_index : layer, is_release);
                    break;
                case EXT_VIAL_APPLE_FN:
#if ENABLE_APPLE_FN_KEY
                    key = USB_KEY_VIRTUAL_APPLE_FN;
                    goto extended_key_fallthrough;
#else
#ifndef APPLE_FN_LAYER
#ifdef FN_LAYER
#define APPLE_FN_LAYER FN_LAYER
#else
#define APPLE_FN_LAYER VIAL_LAYER_COUNT
#endif
#endif
                    if (!is_release) {
                        enable_layer(APPLE_FN_LAYER);
                    } else {
                        disable_layer(APPLE_FN_LAYER);
                    }
#endif
                    break;
#endif
#if ENABLE_TRI_LAYER
                case EXT_LAYER_2_4:
                    if (!is_release) {
                        enable_layer(2);
                    } else {
                        disable_layer(2);
                    }
                    update_tri_layer();
                    break;
                case EXT_LAYER_3_4:
                    if (!is_release) {
                        enable_layer(3);
                    } else {
                        disable_layer(3);
                    }
                    update_tri_layer();
                    break;
#endif
#if ENABLE_SPACE_CADET
                case EXT_SC_LEFT_CTRL_PARENTHESIS_OPEN:
                    mods = SC_LCPO_HOLD;
                    goto space_cadet_process;
                case EXT_SC_RIGHT_CTRL_PARENTHESIS_CLOSE:
                    mods = SC_RCPC_HOLD;
                    goto space_cadet_process;
                case EXT_SC_LEFT_SHIFT_PARENTHESIS_OPEN:
                    mods = SC_LSPO_HOLD;
                    goto space_cadet_process;
                case EXT_SC_RIGHT_SHIFT_PARENTHESIS_CLOSE:
                    mods = SC_RSPC_HOLD;
                    goto space_cadet_process;
                case EXT_SC_LEFT_ALT_PARENTHESIS_OPEN:
                    mods = SC_LAPO_HOLD;
                    goto space_cadet_process;
                case EXT_SC_RIGHT_ALT_PARENTHESIS_CLOSE:
                    mods = SC_RAPC_HOLD;
                    goto space_cadet_process;
                case EXT_SC_RIGHT_SHIFT_ENTER:
                    mods = SC_SENT_HOLD;

                space_cadet_process:
                    is_strong_modifier = true;

                    if (!is_release) {
                        set_pending_keypress(true);
                    } else if (was_pending_keypress) {
                        // No other key pressed — send the tap sequence
                        remove_strong_modifiers(mods);
                        switch (extended_key) {
                        case EXT_SC_LEFT_CTRL_PARENTHESIS_OPEN:
                            mods = SC_LCPO_TAP_MOD;
                            key = SC_LCPO_TAP_KEY;
                            break;
                        case EXT_SC_RIGHT_CTRL_PARENTHESIS_CLOSE:
                            mods = SC_RCPC_TAP_MOD;
                            key = SC_RCPC_TAP_KEY;
                            break;
                        case EXT_SC_LEFT_SHIFT_PARENTHESIS_OPEN:
                            mods = SC_LSPO_TAP_MOD;
                            key = SC_LSPO_TAP_KEY;
                            break;
                        case EXT_SC_RIGHT_SHIFT_PARENTHESIS_CLOSE:
                            mods = SC_RSPC_TAP_MOD;
                            key = SC_RSPC_TAP_KEY;
                            break;
                        case EXT_SC_LEFT_ALT_PARENTHESIS_OPEN:
                            mods = SC_LAPO_TAP_MOD;
                            key = SC_LAPO_TAP_KEY;
                            break;
                        case EXT_SC_RIGHT_ALT_PARENTHESIS_CLOSE:
                            mods = SC_RAPC_TAP_MOD;
                            key = SC_RAPC_TAP_KEY;
                            break;
                        default:
                            mods = SC_SENT_TAP_MOD;
                            key = SC_SENT_TAP_KEY;
                            break;
                        }
                        usb_keyboard_simulate_keypress(key, mods);
                        mods = 0;
                    }
                    break;
#endif
                case EXT_KEYCODE_COUNT:
                    break;
                }
            }
            key = PASS;
        extended_key_fallthrough:
            // This will fall through to process `key` as normal
        }

        if (mods) {
            if (is_strong_modifier) {
                if (is_release) {
                    remove_strong_modifiers(mods);
                } else {
                    add_strong_modifiers(mods);
                }
            } else {
                if (is_release) {
                    remove_weak_modifiers(mods);
                } else {
                    add_weak_modifiers(mods);
                }
            }
        }
    }
#if ENABLE_AUTOSHIFT
    else if (physical_key && vial_autoshift_process(keycode, key, is_release)) {
        goto postprocess;
    }
#endif

#if ENABLE_ONESHOT_KEYCODES
    // Apply OSM mods as weak modifiers for this keypress
    if (oneshot_mods) {
        add_weak_modifiers(oneshot_mods);
    }

    // Consume one-shot layer on non-modifier keypress.
    // Skip layer commands (command 1-5) since those are the OSL key itself.
    if (!is_release && oneshot_layer) {
        uint8_t cmd = COMMAND_OF(keycode);
        if (cmd < 1 || cmd > 5) {
            bool is_modifier_only = IS_MODIFIER(PLAIN_KEY_OF(keycode));
            if (!is_modifier_only && keycode != NONE && keycode != PASS) {
                bool is_consuming = (keycode <= (uint8_t)(NONE - 1)) || is_extended_keycode(keycode);
                if (is_consuming) {
                    restore_oneshot_layer();
                    oneshot_layer = 0;
                }
            }
        }
    }
#endif

    register_key(key, is_release);

#if ENABLE_ONESHOT_KEYCODES
    // Consume OSM if a non-modifier key is in the buffer
    if (oneshot_mods && usb_keys_buffer[0] != 0) {
        oneshot_mods = 0;
    }
#endif

postprocess:
    if (is_release) {
        postprocess_release(keycode, physical_key, data_or_index);
    }
#else // ^ LAYER_COUNT > 0
    // No layers, just use the key as is
    if (action > PRESS) {
        usb_keyboard_release(physical_key);
    } else {
        usb_keyboard_press(physical_key);
    }
#endif
    // Update the USB host if there were any changes
    (void) usb_keyboard_send_if_needed();
}

void
reset_keys (bool is_wake_up) {
#if LAYER_COUNT > 0
    clear_weak_modifiers();
    clear_strong_modifiers();
    pending_release = 0;
#endif
    usb_keyboard_release_all_keys();
#if ENABLE_KEYLOCK
    keylock_key = 0;
#endif
    for (int_fast8_t i = 0; i < MAX_REMAPPED_KEY_ROLLOVER; ++i) {
        keybuffer[i].key = 0;
        keybuffer[i].data = 0;
        keybuffer[i].keycode = 0;
    }
#if ENABLE_AUTOSHIFT
    vial_autoshift_reset();
#endif
#if LAYER_COUNT > 0
    if (!is_wake_up || RESET_LAYERS_ON_SUSPEND) {
        reset_layers();
    }

    handle_reset();

#if !RESET_LAYERS_ON_SUSPEND
    if (is_wake_up) {
        // Re-trigger layer hooks after suspend.
        for (uint8_t layer = 1; layer <= LAYER_COUNT; ++layer) {
            if (is_layer_active(layer)) {
                layer_state_changed(layer, true);
            } else if (layer == DEFAULT_BASE_LAYER) {
                layer_state_changed(layer, false);
            }
        }
    } else
#endif
    {
        layer_state_changed(base_layer, true);
    }
#endif
}

#if SIMULATED_KEYPRESS_TIME_MS >= 15
#define is_time_to_release_at(now)  (((pending_release_since + ((SIMULATED_KEYPRESS_TIME_MS - 5)/ 10)) - (now)) & 0x80U)
#elif SIMULATED_KEYPRESS_TIME_MS <= 5
#define is_time_to_release_at(now)  (1) 
#else
#define is_time_to_release_at(now)  ((now) != pending_release_since)
#endif

/// Mask of overridden LEDs, where lower 4 bits are a mask to add to the
/// mask requested by host, and the upper 4 bits are a mask to subtract
/// from the LEDs requested by host. The subtraction is done first.
static uint8_t override_leds = 0;

/// The previous USB LED state - so we can react to updates, such as
/// programmatically turning on Num Lock.
static uint8_t previous_usb_led_state = 0;

static inline void
remove_override_leds_on (const uint8_t mask) {
    override_leds &= ~(mask & 0x0FU);
}

static inline void
add_override_leds_on (const uint8_t mask) {
    override_leds |= mask & 0x0FU;
}

static inline void
remove_override_leds_off (const uint8_t mask) {
    override_leds &= ~(mask << 4);
}

static inline void
add_override_leds_off (const uint8_t mask) {
    override_leds |= (mask << 4);
}

static inline void
clear_override_leds (void) {
    override_leds = 0;
}

uint8_t
keys_led_state (void) {
    uint8_t leds = usb_keyboard_led_state();
    if (leds != previous_usb_led_state) {
        previous_usb_led_state = leds;
#if LAYER_COUNT > 0
        keyboard_host_leds_changed(leds);
#endif
    }
    leds &= ~(override_leds >> 4);
    leds |= override_leds & 0x0FU;
    return leds;
}

uint8_t
keys_error (void) {
    return usb_key_error();
}

#if LAYER_COUNT > 0
static keycode_t
keycode_from_layer (uint8_t key, uint8_t num) {
#if VIAL_ENABLE
    if (num <= VIAL_LAYER_COUNT) {
        return vial_get_keycode_for_physical_key(key, num);
    }
#endif
    switch (num) {
    case 0: break;
#if LAYER_COUNT >= 1
#if VIAL_LAYER_COUNT < 1
    _Static_assert(LAYER_SIZE(1) <= 0xFF, "Extended keycode as index in layer 1");
    case 1: return get_key_from_layer(key, 1);
#endif
#endif
#if LAYER_COUNT >= 2
#if VIAL_LAYER_COUNT < 2
    _Static_assert(LAYER_SIZE(2) <= 0xFF, "Extended keycode as index in layer 2");
    case 2: return get_key_from_layer(key, 2);
#endif
#endif
#if LAYER_COUNT >= 3
#if VIAL_LAYER_COUNT < 3
    _Static_assert(LAYER_SIZE(3) <= 0xFF, "Extended keycode as index in layer 3");
    case 3: return get_key_from_layer(key, 3);
#endif
#endif
#if LAYER_COUNT >= 4
#if VIAL_LAYER_COUNT < 4
    _Static_assert(LAYER_SIZE(4) <= 0xFF, "Extended keycode as index in layer 4");
    case 4: return get_key_from_layer(key, 4);
#endif
#endif
#if LAYER_COUNT >= 5
#if VIAL_LAYER_COUNT < 5
    _Static_assert(LAYER_SIZE(5) <= 0xFF, "Extended keycode as index in layer 5");
    case 5: return get_key_from_layer(key, 5);
#endif
#endif
#if LAYER_COUNT >= 6
#if VIAL_LAYER_COUNT < 6
    _Static_assert(LAYER_SIZE(6) <= 0xFF, "Extended keycode as index in layer 6");
    case 6: return get_key_from_layer(key, 6);
#endif
#endif
#if LAYER_COUNT >= 7
    _Static_assert(LAYER_SIZE(7) <= 0xFF, "Extended keycode as index in layer 7");
    case 7: return get_key_from_layer(key, 7);
#endif
#if LAYER_COUNT >= 8
    _Static_assert(LAYER_SIZE(8) <= 0xFF, "Extended keycode as index in layer 8");
    case 8: return get_key_from_layer(key, 8);
#endif
#if LAYER_COUNT >= 9
    _Static_assert(LAYER_SIZE(9) <= 0xFF, "Extended keycode as index in layer 9");
    case 9: return get_key_from_layer(key, 9);
#endif
#if LAYER_COUNT >= 10
    _Static_assert(LAYER_SIZE(10) <= 0xFF, "Extended keycode as index in layer 10");
    case 10: return get_key_from_layer(key, 10);
#endif
#if LAYER_COUNT >= 11
    _Static_assert(LAYER_SIZE(11) <= 0xFF, "Extended keycode as index in layer 11");
    case 11: return get_key_from_layer(key, 11);
#endif
#if LAYER_COUNT >= 12
    _Static_assert(LAYER_SIZE(12) <= 0xFF, "Extended keycode as index in layer 12");
    case 12: return get_key_from_layer(key, 12);
#endif
#if LAYER_COUNT >= 13
    _Static_assert(LAYER_SIZE(13) <= 0xFF, "Extended keycode as index in layer 13");
    case 13: return get_key_from_layer(key, 13);
#endif
#if LAYER_COUNT >= 14
    _Static_assert(LAYER_SIZE(14) <= 0xFF, "Extended keycode as index in layer 14");
    case 14: return get_key_from_layer(key, 14);
#endif
#if LAYER_COUNT >= 15
    _Static_assert(LAYER_SIZE(15) <= 0xFF, "Extended keycode as index in layer 15");
    case 15: return get_key_from_layer(key, 15);
#endif
#if LAYER_COUNT >= 16
    _Static_assert(LAYER_SIZE(16) <= 0xFF, "Extended keycode as index in layer 16");
    case 16: return get_key_from_layer(key, 16);
#endif
#if LAYER_COUNT >= 17
    _Static_assert(LAYER_SIZE(17) <= 0xFF, "Extended keycode as index in layer 17");
    case 17: return get_key_from_layer(key, 17);
#endif
#if LAYER_COUNT >= 18
    _Static_assert(LAYER_SIZE(18) <= 0xFF, "Extended keycode as index in layer 18");
    case 18: return get_key_from_layer(key, 18);
#endif
#if LAYER_COUNT >= 19
    _Static_assert(LAYER_SIZE(19) <= 0xFF, "Extended keycode as index in layer 19");
    case 19: return get_key_from_layer(key, 19);
#endif
#if LAYER_COUNT >= 20
    _Static_assert(LAYER_SIZE(20) <= 0xFF, "Extended keycode as index in layer 20");
    case 20: return get_key_from_layer(key, 20);
#endif
#if LAYER_COUNT >= 21
    _Static_assert(LAYER_SIZE(21) <= 0xFF, "Extended keycode as index in layer 21");
    case 21: return get_key_from_layer(key, 21);
#endif
#if LAYER_COUNT >= 22
    _Static_assert(LAYER_SIZE(22) <= 0xFF, "Extended keycode as index in layer 22");
    case 22: return get_key_from_layer(key, 22);
#endif
#if LAYER_COUNT >= 23
    _Static_assert(LAYER_SIZE(23) <= 0xFF, "Extended keycode as index in layer 23");
    case 23: return get_key_from_layer(key, 23);
#endif
#if LAYER_COUNT >= 24
    _Static_assert(LAYER_SIZE(24) <= 0xFF, "Extended keycode as index in layer 24");
    case 24: return get_key_from_layer(key, 24);
#endif
#if LAYER_COUNT >= 25
    _Static_assert(LAYER_SIZE(25) <= 0xFF, "Extended keycode as index in layer 25");
    case 25: return get_key_from_layer(key, 25);
#endif
#if LAYER_COUNT >= 26
    _Static_assert(LAYER_SIZE(26) <= 0xFF, "Extended keycode as index in layer 26");
    case 26: return get_key_from_layer(key, 26);
#endif
#if LAYER_COUNT >= 27
    _Static_assert(LAYER_SIZE(27) <= 0xFF, "Extended keycode as index in layer 27");
    case 27: return get_key_from_layer(key, 27);
#endif
#if LAYER_COUNT >= 28
    _Static_assert(LAYER_SIZE(28) <= 0xFF, "Extended keycode as index in layer 28");
    case 28: return get_key_from_layer(key, 28);
#endif
#if LAYER_COUNT >= 29
    _Static_assert(LAYER_SIZE(29) <= 0xFF, "Extended keycode as index in layer 29");
    case 29: return get_key_from_layer(key, 29);
#endif
#if LAYER_COUNT >= 30
    _Static_assert(LAYER_SIZE(30) <= 0xFF, "Extended keycode as index in layer 30");
    case 30: return get_key_from_layer(key, 30);
#endif
#if LAYER_COUNT >= 31
    _Static_assert(LAYER_SIZE(30) <= 0xFF, "Extended keycode as index in layer 31");
    case 31: return get_key_from_layer(key, 31);
#endif
    }
    return key;
}
#endif // ^ LAYER_COUNT > 0

/// Called approximately once every 10 milliseconds with an 8-bit time value.
void
keys_tick (uint8_t tick_10ms_count) {
    if (pending_release && is_time_to_release_at(tick_10ms_count)) {
        send_pending_release();
    }

#if LAYER_COUNT > 0
#if ENABLE_ONESHOT_KEYCODES
    // One-shot timeout: clear if no key pressed within timeout
    if (oneshot_timeout_ms && oneshot_layer && (uint8_t) (tick_10ms_count - oneshot_layer_time) >= (uint8_t) (oneshot_timeout_ms / 10)) {
        restore_oneshot_layer();
        oneshot_layer = 0;
        if (oneshot_tap_toggle > 1) {
            oneshot_tap_count = 0;
        }
    }
#endif

    handle_tick(tick_10ms_count);
#endif

#if VIAL_ENABLE && VIAL_COMBO_COUNT > 0
    combo_task(tick_10ms_count);
#endif
}

#if VIAL_ENABLE
void
keys_vial_task (void) {
#if VIAL_TAP_DANCE_COUNT > 0
    vial_tap_dance_task();
#endif
#if ENABLE_AUTOSHIFT
    vial_autoshift_task();
#endif
}

uint16_t vial_read_progmem_keycode(uint8_t layer, uint8_t physical_key) {
    if (layer > LAYER_COUNT || layer <= VIAL_LAYER_COUNT) {
        return 0;
    }
    return keycode_from_layer(physical_key, layer);
}

uint8_t vial_total_layer_count(void) {
    return LAYER_COUNT;
}
#endif
