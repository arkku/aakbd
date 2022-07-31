/**
 * keys.c: Key processing.
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
#include <assert.h>
#include "keys.h"
#include "aakbd.h"

#include "usbkbd.h"
#include "usb_keys.h"
#include "keycodes.h"

#include <layers.c> // Yes, including .c

#if LAYER_COUNT > 0
#include <macros.c> // Yes, including .c
#endif

#ifndef DEFAULT_BASE_LAYER
#define DEFAULT_BASE_LAYER 1
#endif
#if LAYER_COUNT > 1
// To simplify code elsewhere, define arrays of 1 element for every layer,
// it's only a few dozen wasted bytes at worst (and in practice it will get
// optimised away by the compiler)
#if LAYER_COUNT < 3
DEFINE_EMPTY_LAYER(3);
#endif
#if LAYER_COUNT < 4
DEFINE_EMPTY_LAYER(4);
#endif
#if LAYER_COUNT < 5
DEFINE_EMPTY_LAYER(5);
#endif
#if LAYER_COUNT < 6
DEFINE_EMPTY_LAYER(6);
#endif
#if LAYER_COUNT < 7
DEFINE_EMPTY_LAYER(7);
#endif
#if LAYER_COUNT < 8
DEFINE_EMPTY_LAYER(8);
#endif
#if LAYER_COUNT > 8
#if LAYER_COUNT < 10
DEFINE_EMPTY_LAYER(10);
#endif
#if LAYER_COUNT < 11
DEFINE_EMPTY_LAYER(11);
#endif
#if LAYER_COUNT < 12
DEFINE_EMPTY_LAYER(12);
#endif
#if LAYER_COUNT < 13
DEFINE_EMPTY_LAYER(13);
#endif
#if LAYER_COUNT < 14
DEFINE_EMPTY_LAYER(14);
#endif
#if LAYER_COUNT < 15
DEFINE_EMPTY_LAYER(15);
#endif
#if LAYER_COUNT < 16
DEFINE_EMPTY_LAYER(16);
#endif
#if LAYER_COUNT > 16
#if LAYER_COUNT < 18
DEFINE_EMPTY_LAYER(18);
#endif
#if LAYER_COUNT < 19
DEFINE_EMPTY_LAYER(19);
#endif
#if LAYER_COUNT < 20
DEFINE_EMPTY_LAYER(20);
#endif
#if LAYER_COUNT < 21
DEFINE_EMPTY_LAYER(21);
#endif
#if LAYER_COUNT < 22
DEFINE_EMPTY_LAYER(22);
#endif
#if LAYER_COUNT < 23
DEFINE_EMPTY_LAYER(23);
#endif
#if LAYER_COUNT < 24
DEFINE_EMPTY_LAYER(24);
#endif
#if LAYER_COUNT < 25
DEFINE_EMPTY_LAYER(25);
#endif
#if LAYER_COUNT < 26
DEFINE_EMPTY_LAYER(26);
#endif
#if LAYER_COUNT < 27
DEFINE_EMPTY_LAYER(27);
#endif
#if LAYER_COUNT < 28
DEFINE_EMPTY_LAYER(28);
#endif
#if LAYER_COUNT < 29
DEFINE_EMPTY_LAYER(29);
#endif
#if LAYER_COUNT < 30
DEFINE_EMPTY_LAYER(30);
#endif
#if LAYER_COUNT < 31
DEFINE_EMPTY_LAYER(31);
#endif
#endif // LAYER_COUNT > 16
#endif // LAYER_COUNT > 8
#endif // LAYER_COUNT > 1

#if MAX_KEY_ROLLOVER <= 10
#define MAX_REMAPPED_KEY_ROLLOVER MAX_KEY_ROLLOVER
#else
#define MAX_REMAPPED_KEY_ROLLOVER 10
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
#endif

/// Get the key code for `key` in layer number `num`. This has to be a macro
/// because this needs the name of a specific array.
#define get_key_from_layer(key, num) (              \
    (sizeof(keycode_t) == 1) ?                      \
        pgm_read_byte(LAYER_ARRAY(num) + (key)) :   \
        pgm_read_word(LAYER_ARRAY(num) + (key))     \
)
#define layer_bit(num)              (((layer_mask_t) 1) << ((num) - 1))
#define is_key_in_layer(key, num)   ((key) < LAYER_SIZE(num))
#define is_layer_enabled(num)       (((num) == base_layer) || (layer_mask & layer_bit(num)))

#define layer_enabled(num)          layer_state_changed((num), true)
#define layer_disabled(num)         layer_state_changed((num), false)

// Note: This sets local variables inside `process_key`
#define set_keycode_from_layer(key, num) do {                               \
    if (keycode == 0 && (num) >= base_layer && is_key_in_layer((key), num) && is_layer_enabled(num)) { \
        keycode = get_key_from_layer((key), num);                           \
    }                                                                       \
} while (0)

#if LAYER_COUNT > 0
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

static inline void
set_base_layer (const uint8_t num) {
    previous_base_layer = base_layer;
    if (base_layer == num) {
        return;
    }
    base_layer = num;

    for (int_fast8_t i = previous_base_layer; i < num; ++i) {
        // Active layers with a lower number than the new base became disabled
        if (i == previous_base_layer || is_layer_active(i)) {
            layer_disabled(i);
        }
    }
    if (previous_base_layer > num) {
        for (int_fast8_t i = num; i < previous_base_layer; ++i) {
            // Active layers with a higher number than the new base became enabled
            if (i == num || is_layer_active(i)) {
                layer_enabled(i);
            }
        }
        if (!is_layer_active(previous_base_layer)) {
            // The previous base can still have become disabled
            layer_disabled(previous_base_layer);
        }
    }
}

static inline void
restore_previous_base_layer (void) {
    set_base_layer(previous_base_layer);
}

static inline bool
is_layer_active (const uint8_t num) {
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

static inline uint8_t
current_base_layer (void) {
    return base_layer;
}

static inline void
clear_weak_modifiers (void) {
    weak_modifiers = 0U;
}

static inline void
clear_strong_modifiers (void) {
    strong_modifiers = 0U;
}

static inline void
add_weak_modifiers (const uint8_t mods) {
    weak_modifiers |= mods;
}

static inline void
remove_weak_modifiers (const uint8_t mods) {
    weak_modifiers &= ~(mods);
}

static inline uint8_t
weak_modifiers_mask (void) {
    return weak_modifiers;
}

static inline void
add_strong_modifiers (const uint8_t mods) {
    strong_modifiers |= mods;
}

static inline void
remove_strong_modifiers (const uint8_t mods) {
    strong_modifiers &= ~(mods);
}

static inline uint8_t
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

/// If we have a simulated keypress in progress, this is the pending keycode
/// to release either based on time elapsed or the 
static uint8_t pending_release = 0;

/// The tick since which `pending_release` has been pending.
static uint8_t pending_release_since = 0;

static inline void
set_pending_keypress (const bool is_pending) {
    is_pending_keypress = is_pending;
}

static inline bool
pending_keypress (void) {
    return is_pending_keypress;
}

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

static inline void
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
    clear_override_leds();
    set_active_layers_mask(0);
    previous_layer_mask = 0;
    set_base_layer(DEFAULT_BASE_LAYER);
    previous_base_layer = DEFAULT_BASE_LAYER;
    set_pending_keypress(false);
}

static inline void
register_key (const uint8_t key, const bool is_release) {
    if (is_release) {
        if (IS_MODIFIER(key)) {
            remove_strong_modifiers(MODIFIER_BIT(key));
        } else {
            usb_keyboard_release(key);
        }
    } else {
        if (IS_MODIFIER(key)) {
            add_strong_modifiers(MODIFIER_BIT(key));
        } else {
            usb_keyboard_press(key);
        }
    }
    usb_keyboard_set_modifiers(strong_modifiers_mask() | weak_modifiers_mask());
}
#endif // ^ LAYER_COUNT > 0

void
report_keyboard_error (bool is_rollover_error) {
    usb_keyboard_press(is_rollover_error ? USB_KEY_ROLLOVER : USB_KEY_UNDEFINED_ERROR);
}

void
process_key (uint8_t key, bool is_release) {
#if LAYER_COUNT > 0
    keycode_t keycode = PASS;
    const bool was_pending_keypress = pending_keypress();
    const uint8_t physical_key = key;
    uint8_t data_or_index = 0;

    if (pending_release) {
        // If we are pending some timed key release, release it now since
        // otherwise timing between it and this key event will be wrong.
        send_pending_release();
    }

    if (is_release) {
#if ENABLE_KEYLOCK
        if (physical_key == keylock_key) {
            // Do not release the locked key
            return;
        }
#endif
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
    } else {
#if ENABLE_KEYLOCK
        if (physical_key == keylock_key) {
            // The locked key was pressed again, unlock it.
            // (It will be released when this new press is released.)
            keylock_key = 0;
            return;
        }
#endif
#if LAYER_COUNT > 1
        // The reason this is unrolled is that we get more compile-time
        // constants like `sizeof` here, and the compiler is more likely to
        // optimise away unused layers.
#if LAYER_MASK_BITS > 16
        set_keycode_from_layer(key, 31);
        set_keycode_from_layer(key, 30);
        set_keycode_from_layer(key, 29);
        set_keycode_from_layer(key, 28);
        set_keycode_from_layer(key, 27);
        set_keycode_from_layer(key, 26);
        set_keycode_from_layer(key, 25);
        set_keycode_from_layer(key, 24);
        set_keycode_from_layer(key, 23);
        set_keycode_from_layer(key, 22);
        set_keycode_from_layer(key, 21);
        set_keycode_from_layer(key, 20);
        set_keycode_from_layer(key, 19);
        set_keycode_from_layer(key, 18);
        set_keycode_from_layer(key, 17);
#endif
#if LAYER_MASK_BITS > 8
        set_keycode_from_layer(key, 16);
        set_keycode_from_layer(key, 15);
        set_keycode_from_layer(key, 14);
        set_keycode_from_layer(key, 13);
        set_keycode_from_layer(key, 12);
        set_keycode_from_layer(key, 11);
        set_keycode_from_layer(key, 10);
        set_keycode_from_layer(key, 9);
#endif
        set_keycode_from_layer(key, 8);
        set_keycode_from_layer(key, 7);
        set_keycode_from_layer(key, 6);
        set_keycode_from_layer(key, 5);
        set_keycode_from_layer(key, 4);
        set_keycode_from_layer(key, 3);
        set_keycode_from_layer(key, 2);
#endif
        set_keycode_from_layer(key, 1);

        keycode = preprocess_press(keycode, physical_key, &data_or_index);

        if (keycode == PASS) {
            keycode = key;
        }
        if (keycode != key || data_or_index != 0) {
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
            if (i == MAX_REMAPPED_KEY_ROLLOVER || keybuffer[i].key == key) {
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
        if (is_keylock_armed) {
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
                key = NONE;
            } else {
                // Layer command
                uint8_t layer;
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
                key = NONE; // The key part of this keycode is not a key

                if (layer >= 0 && layer <= LAYER_COUNT) {
                    // Layer modifying command

                    int_fast8_t action; // 1 = activate, -1 = deactivate

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
            // Extended keycode that is not a modifier or a command
            if (extended_keycode_is_macro(key)) {
                // On release the previous data is in `data_or_index`
                // since the keypress object is already overwritten, while
                // on release it is the index of the new object.
                execute_macro(
                    MACRO_OF_EXTENDED(key),
                    is_release,
                    physical_key,
                    is_release ? &data_or_index : &(keybuffer[data_or_index].data)
                );
            } else if (extended_keycode_is_exact_modifiers(key)) {
                is_strong_modifier = true;
                if (is_release) {
                    // Unset the added modifiers - we can't restore the old
                    // modifiers since those keys might have been already
                    // released, which would leave their modifiers stuck
                    mods = data_or_index;
                } else {
                    mods = EXACT_MODS_OF_EXTENDED(key);
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
                case EXT_RESET_LAYERS:
                    if (is_release) {
                        reset_layers();
                    }
                    break;
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
#if ENABLE_KEYLOCK
                case EXT_KEYLOCK:
                    if (!is_release) {
                        if (keylock_key) {
                            // Keylock pressed again, release the locked key
                            key = keylock_key;
                            keylock_key = 0;
                            if (key != physical_key) {
                                process_key(key, true);
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

#ifdef EXT_PRINT_DEBUG_INFO
                case EXT_PRINT_DEBUG_INFO:
                    usb_keyboard_type_debug_report();
                    break;
#endif
                default:
                    break;
                }
            }
            key = NONE;
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
    register_key(key, is_release);
postprocess:
    if (is_release) {
        postprocess_release(keycode, physical_key, data_or_index);
    }
#else // ^ LAYER_COUNT > 0
    // No layers, just use the key as is
    if (is_release) {
        usb_keyboard_release(key);
    } else {
        usb_keyboard_press(key);
    }
#endif
    // Update the USB host if there were any changes
    (void) usb_keyboard_send_if_needed();
}

void
reset_keys (void) {
#if LAYER_COUNT > 0
    clear_weak_modifiers();
    clear_strong_modifiers();
    reset_layers();
    pending_release = 0;
#endif
    usb_release_all_keys();
#if ENABLE_KEYLOCK
    keylock_key = 0;
#endif
    for (int_fast8_t i = 0; i < MAX_REMAPPED_KEY_ROLLOVER; ++i) {
        keybuffer[i].key = 0;
        keybuffer[i].data = 0;
        keybuffer[i].keycode = 0;
    }
#if LAYER_COUNT > 0
    handle_reset();
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
    leds &= ~(override_leds >> 4);
    leds |= override_leds & 0x0FU;
    return leds;
}

uint8_t
keys_error (void) {
    return usb_key_error();
}

static inline keycode_t
keycode_from_layer (uint8_t key, uint8_t num) {
    switch (num) {
    case 0: break;
#if LAYER_COUNT >= 1
    _Static_assert(LAYER_SIZE(1) <= 0xFF, "Extended keycode as index in layer 1");
    case 1: return get_key_from_layer(key, 1);
#endif
#if LAYER_COUNT >= 2
    _Static_assert(LAYER_SIZE(2) <= 0xFF, "Extended keycode as index in layer 2");
    case 2: return get_key_from_layer(key, 2);
#endif
#if LAYER_COUNT >= 3
    _Static_assert(LAYER_SIZE(3) <= 0xFF, "Extended keycode as index in layer 3");
    case 3: return get_key_from_layer(key, 3);
#endif
#if LAYER_COUNT >= 4
    _Static_assert(LAYER_SIZE(4) <= 0xFF, "Extended keycode as index in layer 4");
    case 4: return get_key_from_layer(key, 4);
#endif
#if LAYER_COUNT >= 5
    _Static_assert(LAYER_SIZE(5) <= 0xFF, "Extended keycode as index in layer 5");
    case 5: return get_key_from_layer(key, 5);
#endif
#if LAYER_COUNT >= 6
    _Static_assert(LAYER_SIZE(6) <= 0xFF, "Extended keycode as index in layer 6");
    case 6: return get_key_from_layer(key, 6);
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

/// Called approximately once every 10 milliseconds with an 8-bit time value.
void
keys_tick (uint8_t tick_10ms_count) {
#if LAYER_COUNT > 0
    if (pending_release && is_time_to_release_at(tick_10ms_count)) {
        send_pending_release();
    }
    handle_tick(tick_10ms_count);
#endif
}
