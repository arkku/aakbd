/**
 * vial_keys.c: Vial/QMK key processing for AAKBD.
 *
 * Reimplementation of Vial/QMK features on top of AAKBD's key processing
 * system.
 *
 * Copyright (c) 2026 Kimmo Kulovesi, https://arkku.dev/
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

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "vial_config.h"

#define USB_KEYBOARD_ACCESS_STATE 1
#include "usbkbd.h"
#include "aakbd.h"
#include "keys.h"

#include "dynamic_storage.h"
#include "vial.h"
#include "vial_keys.h"
#include "dynamic_keymap.h"
#include "qmk_keycodes.h"
#include "qmk_translate.h"
#include "vial_magic.h"
#include "progmem.h"

#if ENABLE_PS2_DEVICE
#include "ps2_output.h"
#endif

#ifndef TESTING
#if HAPTIC_ENABLE
#include "haptic.h"
#include "solenoid.h"
#endif
#include "matrix.h"
#include "timer.h"
#endif

#define QS_ID_AUTO_SHIFT_FLAGS   3
#define QS_ID_AUTO_SHIFT_TIMEOUT 4
#define ASF_ENABLE               ((uint8_t) (1u << 0))
#define ASF_MODIFIERS            ((uint8_t) (1u << 1))
#define ASF_NO_SPECIAL           ((uint8_t) (1u << 2))
#define ASF_NO_NUMERIC           ((uint8_t) (1u << 3))
#define ASF_NO_ALPHA             ((uint8_t) (1u << 4))
#define ASF_AUTOREPEAT           ((uint8_t) (1u << 5))
#define ASF_NO_AUTO_REPEAT       ((uint8_t) (1u << 6))
#define autoshift_enabled        (autoshift_flags & ASF_ENABLE)

#ifndef AUTOSHIFT_TIMEOUT_MS
#define AUTOSHIFT_TIMEOUT_MS 150
#endif
#ifndef AUTOSHIFT_ADJUST_INCREMENT_MS
#define AUTOSHIFT_ADJUST_INCREMENT_MS 5
#endif
#ifndef AUTOSHIFT_MAX_TIMEOUT_MS
#define AUTOSHIFT_MAX_TIMEOUT_MS 5000
#endif

#if VIAL_COMBO_COUNT > 0
static bool vial_combo_disabled = 0;
#endif

#if VIAL_TAP_DANCE_COUNT > 0
static bool vial_tap_dance_process(uint16_t qmk_key, bool is_release);
#endif

#if NUM_ENCODERS > 0
void
handle_encoder_rotation (bool is_clockwise) {
    const uint16_t qmk_key = vial_get_encoder_keycode(0, is_clockwise);
    if (qmk_key) {
        const keycode_t aakbd_key = qmk_to_aakbd(qmk_key);
        process_keycode(0, aakbd_key, PRESS, ENCODERS_ROW, 0);
        usb_keyboard_send_if_needed();
        process_keycode(0, aakbd_key, RELEASE, ENCODERS_ROW, 0);
        usb_keyboard_send_if_needed();
    }
}
#endif

uint16_t
vial_get_keycode_for_physical_key (uint8_t physical_key, uint8_t layer) {
    // Do the QMK magic swaps based on the physical key first
    const uint8_t effective_key = vial_magic_remap_key(physical_key);

    uint8_t row, col;
    if (dynamic_keymap_find_matrix_pos(effective_key, &row, &col) < 0) {
        return effective_key;
    }

    return qmk_to_aakbd(dynamic_keymap_get_qmk_keycode(layer - 1, row, col));
}

uint16_t
vial_get_keycode_at (uint8_t row, uint8_t col, uint8_t layer) {
    return qmk_to_aakbd(dynamic_keymap_get_qmk_keycode(layer - 1, row, col));
}

#if HAPTIC_ENABLE
static bool
vial_haptic_process_keycode (uint16_t qmk_key, bool is_release) {
    switch (qmk_key) {
        case QK_HAPTIC_ON:
            if (!is_release) {
                haptic_enable();
                solenoid_fire(0);
            }
            return true;
        case QK_HAPTIC_OFF:
            if (!is_release) {
                haptic_disable();
            }
            return true;
        case QK_HAPTIC_TOGGLE:
            if (!is_release) {
                haptic_toggle();
            }
            return true;
        case QK_HAPTIC_RESET:
            if (!is_release) {
                haptic_reset();
            }
            return true;
        case QK_HAPTIC_FEEDBACK_TOGGLE:
            if (!is_release) {
                haptic_feedback_toggle();
            }
            return true;
        case QK_HAPTIC_BUZZ_TOGGLE:
            if (!is_release) {
                haptic_buzz_toggle();
            }
            return true;
        case QK_HAPTIC_MODE_NEXT:
            if (!is_release) {
                haptic_mode_increase();
            }
            return true;
        case QK_HAPTIC_MODE_PREVIOUS:
            if (!is_release) {
                haptic_mode_decrease();
            }
            return true;
        case QK_HAPTIC_CONTINUOUS_TOGGLE:
            if (!is_release) {
                haptic_toggle_continuous();
            }
            return true;
        case QK_HAPTIC_CONTINUOUS_UP:
            if (!is_release) {
                haptic_cont_increase();
            }
            return true;
        case QK_HAPTIC_CONTINUOUS_DOWN:
            if (!is_release) {
                haptic_cont_decrease();
            }
            return true;
        case QK_HAPTIC_DWELL_UP:
            if (!is_release) {
                haptic_dwell_increase();
            }
            return true;
        case QK_HAPTIC_DWELL_DOWN:
            if (!is_release) {
                haptic_dwell_decrease();
            }
            return true;
        default:
            return false;
    }
}
#endif

void
vial_process_qmk_keycode (uint8_t row, uint8_t col, uint8_t layer, bool is_release) {
    // This function reads from the matrix keymap. Non-matrix events
    // (encoders with ENCODERS_ROW, etc.) have no matrix position to
    // read from — their keycodes are resolved by the caller.
    if (row >= MATRIX_ROWS) {
        return;
    }
    const uint16_t qmk_key = dynamic_keymap_get_qmk_keycode(layer - 1, row, col);
    if (qmk_key == KC_NO || qmk_key == KC_TRNS) {
        return;
    }

    // MAGIC keycodes (swap Ctrl/Caps, Alt/GUI, etc.) — process on press only,
    // since toggle (XOR) would cancel if also processed on release.
    if (!is_release && vial_magic_process_keycode(qmk_key)) {
        return;
    }

    if (qmk_key == QK_CLEAR_EEPROM) {
        if (is_release) {
            keyboard_clear_settings();
        }
        return;
    }

    // PDF (persistent default layer)
    if (IS_QK_PDF(qmk_key)) {
        if (is_release) {
            set_base_layer((qmk_key & 0x1F) + 1);
            eeconfig_update_default_layer(current_base_layer());
        }
        return;
    }

#if HAPTIC_ENABLE
    if (vial_haptic_process_keycode(qmk_key, is_release)) {
        return;
    }
#endif

#if VIAL_COMBO_COUNT > 0
    if (!is_release) {
        switch (qmk_key) {
            case QK_COMBO_ON:
                vial_combo_disabled = false;
                return;
            case QK_COMBO_OFF:
                vial_combo_disabled = true;
                return;
            case QK_COMBO_TOGGLE:
                vial_combo_disabled = !vial_combo_disabled;
                return;
            default:
                break;
        }
    }
#endif

#if VIAL_TAP_DANCE_COUNT > 0
    if (qmk_key >= QK_TAP_DANCE && qmk_key <= QK_TAP_DANCE_MAX) {
        if (vial_tap_dance_process(qmk_key, is_release)) {
            return;
        }
    }
#endif

#if ENABLE_AUTOSHIFT
    if (!is_release) {
        switch (qmk_key) {
            case QK_AUTO_SHIFT_ON:
                if (!autoshift_enabled) {
                    autoshift_flags |= ASF_ENABLE;
                    eeprom_update_byte(
                        VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, autoshift_flags),
                        autoshift_flags);
                }
                return;
            case QK_AUTO_SHIFT_OFF:
                if (autoshift_enabled) {
                    autoshift_flags &= ~ASF_ENABLE;
                    eeprom_update_byte(
                        VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, autoshift_flags),
                        autoshift_flags);
                }
                return;
            case QK_AUTO_SHIFT_TOGGLE:
                autoshift_flags ^= ASF_ENABLE;
                eeprom_update_byte(
                    VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, autoshift_flags),
                    autoshift_flags);
                return;
            case QK_AUTO_SHIFT_UP:
                autoshift_timeout_ms += AUTOSHIFT_ADJUST_INCREMENT_MS;
                if (autoshift_timeout_ms > AUTOSHIFT_MAX_TIMEOUT_MS) {
                    autoshift_timeout_ms = AUTOSHIFT_MAX_TIMEOUT_MS;
                }
                return;
            case QK_AUTO_SHIFT_DOWN:
                if (autoshift_timeout_ms > AUTOSHIFT_ADJUST_INCREMENT_MS) {
                    autoshift_timeout_ms -= AUTOSHIFT_ADJUST_INCREMENT_MS;
                }
                return;
            case QK_AUTO_SHIFT_REPORT:
                fprintf_P(usb_kbd_type, PSTR("%u"), (unsigned) autoshift_timeout_ms);
                return;
        }
    }
#endif

    process_qmk_keycode(qmk_key, is_release);
}

#ifndef TESTING
// Weak default — users can override in macros.c to handle unsupported
// QMK keycodes that are stored in EEPROM as EXT_QMK_KEYCODE.
__attribute__((weak)) void
process_qmk_keycode (uint16_t qmk_key, bool is_release) {
}
#endif

#if NUM_ENCODERS > 0
uint16_t
vial_get_encoder_keycode (uint8_t encoder_id, bool clockwise) {
    return dynamic_keymap_get_encoder(0, encoder_id, clockwise);
}
#endif

#if VIAL_TAP_DANCE_COUNT > 0
// Tap dance state per entry
enum { TD_NONE, TD_SINGLE_TAP, TD_SINGLE_HOLD, TD_DOUBLE_TAP, TD_DOUBLE_HOLD, TD_INTERRUPTED };

static struct {
    uint8_t state;
    uint8_t count;
    uint16_t timer;
    bool pressed;
    bool finished;
} td_state[VIAL_TAP_DANCE_COUNT];

static void
vial_tap_dance_press (uint16_t qmk_key, const bool is_release) {
    if (qmk_key == KC_NO || qmk_key == 0) {
        return;
    }
    const keycode_t aakbd_key = qmk_to_aakbd(qmk_key);
    if (aakbd_key) {
        process_keycode(0, aakbd_key, is_release, 0, 0);
        (void) usb_keyboard_send_if_needed();
    }
}

static void
vial_tap_dance_reset (uint8_t index) {
    const uint8_t state = td_state[index].state;

    if (state == TD_NONE) {
        return;
    }

    if (td_state[index].pressed) {
        // Key still pressed
        return;
    }

    vial_tap_dance_entry_t entry;
    if (dynamic_keymap_get_tap_dance(index, &entry)) {
        return;
    }

    // Release whatever key was pressed by the dance
    switch (state) {
        case TD_SINGLE_TAP:
            vial_tap_dance_press(entry.on_tap, true);
            break;
        case TD_SINGLE_HOLD:
            vial_tap_dance_press(entry.on_hold ? entry.on_hold : entry.on_tap, true);
            break;
        case TD_DOUBLE_TAP:
            vial_tap_dance_press(entry.on_double_tap ? entry.on_double_tap : entry.on_tap, true);
            break;
        case TD_DOUBLE_HOLD:
            vial_tap_dance_press(entry.on_tap_hold ? entry.on_tap_hold :
                    entry.on_double_tap            ? entry.on_double_tap :
                                                     entry.on_tap,
                true);
            break;
        case TD_INTERRUPTED:
            vial_tap_dance_press(entry.on_tap, true);
            break;
    }

    td_state[index].count = 0;
    td_state[index].timer = 0;
    td_state[index].pressed = false;
    td_state[index].finished = false;
    td_state[index].state = TD_NONE;
}

static void
vial_tap_dance_tap (uint8_t index, uint16_t qmk_key) {
    if (qmk_key == KC_NO || qmk_key == 0) {
        return;
    }
    vial_tap_dance_press(qmk_key, false);
    usb_keyboard_keypress_delay();
    vial_tap_dance_press(qmk_key, true);
}

void
vial_tap_dance_interrupt (void) {
    for (uint8_t i = 0; i < VIAL_TAP_DANCE_COUNT; ++i) {
        if (td_state[i].count > 0 && !td_state[i].finished) {
            // Only interrupt if the TD key is still pressed.
            // If released, this is a new tap (e.g., second tap for double-tap),
            // not a real interrupt — let the timer handle it.
            if (!td_state[i].pressed) {
                continue;
            }
            td_state[i].state = TD_INTERRUPTED;
            td_state[i].finished = true;
            vial_tap_dance_entry_t entry;
            if (!dynamic_keymap_get_tap_dance(i, &entry)) {
                // Still held — press on_tap (release will happen on physical release)
                vial_tap_dance_press(entry.on_tap, false);
                vial_tap_dance_reset(i);
            }
        }
    }
}

static bool
vial_tap_dance_process (uint16_t qmk_key, bool is_release) {
    uint8_t index = qmk_key & 0xFF;
    if (index >= VIAL_TAP_DANCE_COUNT) {
        return false;
    }

    // Make sure to release any previous key so it won't get stuck
    if (td_state[index].finished) {
        td_state[index].pressed = false;
        vial_tap_dance_reset(index);
        if (is_release) {
            return true;
        }
    }

    if (is_release) {
        td_state[index].pressed = false;
        if (td_state[index].count > 0 && td_state[index].finished) {
            vial_tap_dance_reset(index);
        }
        return true;
    }

    // Press
    td_state[index].pressed = true;
    ++td_state[index].count;
    td_state[index].timer = timer_read();

    // Three taps resets the counter
    if (td_state[index].count >= 3) {
        vial_tap_dance_entry_t entry;
        if (!dynamic_keymap_get_tap_dance(index, &entry)) {
            td_state[index].state = TD_SINGLE_TAP;
            td_state[index].finished = true;
            vial_tap_dance_tap(index, entry.on_tap);
            td_state[index].count = 0;
            td_state[index].timer = 0;
            td_state[index].finished = false;
            td_state[index].state = TD_NONE;
        }
    }

    return true;
}

void
vial_tap_dance_task (void) {
    for (uint8_t i = 0; i < VIAL_TAP_DANCE_COUNT; ++i) {
        if (td_state[i].count == 0 || td_state[i].finished) {
            continue;
        }

        const uint16_t elapsed = timer_elapsed(td_state[i].timer);
        vial_tap_dance_entry_t entry;
        if (dynamic_keymap_get_tap_dance(i, &entry)) {
            continue;
        }
        const uint16_t term =
            entry.custom_tapping_term ? entry.custom_tapping_term : vial_tap_hold_timeout_ms;
        if (elapsed < term) {
            continue;
        }

        td_state[i].finished = true;

        if (!td_state[i].pressed) {
            switch (td_state[i].count) {
                case 1:
                    td_state[i].state = TD_SINGLE_TAP;
                    vial_tap_dance_tap(i, entry.on_tap);
                    break;
                case 2:
                    td_state[i].state = TD_DOUBLE_TAP;
                    vial_tap_dance_tap(i, entry.on_double_tap ? entry.on_double_tap : entry.on_tap);
                    break;
                default:
                    break;
            }
            td_state[i].count = 0;
            td_state[i].timer = 0;
            td_state[i].pressed = false;
            td_state[i].finished = false;
            td_state[i].state = TD_NONE;
        } else {
            switch (td_state[i].count) {
                case 1:
                    td_state[i].state = TD_SINGLE_HOLD;
                    vial_tap_dance_press(entry.on_hold ? entry.on_hold : entry.on_tap, false);
                    break;
                case 2:
                    td_state[i].state = TD_DOUBLE_HOLD;
                    vial_tap_dance_press(entry.on_tap_hold ? entry.on_tap_hold :
                            entry.on_double_tap            ? entry.on_double_tap :
                                                             entry.on_tap,
                        false);
                    break;
                default:
                    break;
            }

            vial_tap_dance_reset(i);
        }
    }
}
#endif // VIAL_TAP_DANCE_COUNT > 0

#if VIAL_COMBO_COUNT > 0

uint16_t vial_combo_timeout_ms = VIAL_COMBO_TIMEOUT_MS;

// Buffer for held combo trigger keys. Entries persist until physical release
// or combo break.
#define COMBO_BUFFER_MAX (VIAL_COMBO_COUNT * VIAL_COMBO_INPUTS)

typedef struct {
    uint8_t phys;
    uint16_t keycode;
    uint8_t data;
    uint8_t row, col;
    uint8_t press_order;
    bool removed;
    bool was_fired; // consumed by a fired combo, never flush to USB
} combo_entry_t;

static combo_entry_t combo_buffer[COMBO_BUFFER_MAX];
static uint8_t combo_count;
static bool combo_fired[VIAL_COMBO_COUNT]; // per-combo fired state
static uint8_t combo_first_tick;

static void
combo_clear (void) {
    combo_count = 0;
    combo_first_tick = 0;
    for (uint8_t i = 0; i < VIAL_COMBO_COUNT; ++i) {
        combo_fired[i] = false;
    }
}

void
vial_reset_combo (void) {
    combo_clear();
    vial_combo_disabled = false;
}

// True if any combo is fired.
static bool
combo_any_fired (void) {
    for (uint8_t i = 0; i < VIAL_COMBO_COUNT; ++i) {
        if (combo_fired[i]) {
            return true;
        }
    }
    return false;
}

// Convert a buffer entry's keycode to QMK for matching.
static uint16_t
combo_buf_to_qmk (uint8_t idx) {
    uint16_t qmk = aakbd_to_qmk(combo_buffer[idx].keycode);
    if (qmk == KC_NO && is_extended_keycode(combo_buffer[idx].keycode)) {
        for (uint8_t l = 0; l < VIAL_LAYER_COUNT; ++l) {
            uint16_t raw =
                dynamic_keymap_get_qmk_keycode(l, combo_buffer[idx].row, combo_buffer[idx].col);
            if (raw != KC_NO && raw != KC_TRNS) {
                qmk = raw;
                break;
            }
        }
    }
    return qmk;
}

// Check if a specific combo's ALL inputs are present (non-removed entries only).
// Only considers unfired combos.
static bool
combo_all_inputs_present (uint8_t combo_idx) {
    vial_combo_entry_t entry;
    if (dynamic_keymap_get_combo(combo_idx, &entry)) {
        return false;
    }
    for (uint8_t j = 0; j < VIAL_COMBO_INPUTS; ++j) {
        uint16_t input = entry.input[j];
        if (input == KC_NO) {
            break;
        }
        bool found = false;
        for (uint8_t k = 0; k < combo_count; ++k) {
            if (combo_buffer[k].removed) {
                continue;
            }
            if (combo_buf_to_qmk(k) == input) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    return true;
}

// Convert QMK keycode to AAKBD output (handles EXTENDED(QMK_KEYCODE) placeholder).
static uint16_t
combo_qmk_to_output (uint16_t qmk_output) {
    uint16_t aakbd = qmk_to_aakbd(qmk_output);
    if (aakbd == EXTENDED(QMK_KEYCODE)) {
        return qmk_output | 0x8000u;
    }
    return aakbd;
}

// Process combo output as virtual key (physical_key=0).
static void
combo_output_process (uint16_t output, bool is_release) {
    if (output == NONE || output == KC_NO) {
        return;
    }
    if (output & 0x8000u) {
        uint16_t qmk_key = output & 0x7FFFu;
        vial_magic_process_keycode(qmk_key);
        process_qmk_keycode(qmk_key, is_release);
    } else {
        process_keycode(0, output, is_release, 0, 0);
    }
}

// Flush deferred press of all non-removed entries in press_order.
// Passes DEFERRED_PRESS to skip preprocess_press, but preserves real row/col
// so EEPROM-based keycode lookups (EXTENDED(QMK_KEYCODE)) work correctly.
static void
combo_flush_pending (void) {
    for (uint8_t order = 0; order < combo_count; ++order) {
        for (uint8_t i = 0; i < combo_count; ++i) {
            if (combo_buffer[i].removed) {
                continue;
            }
            if (combo_buffer[i].press_order != order) {
                continue;
            }
            process_keycode(combo_buffer[i].phys, combo_buffer[i].keycode, DEFERRED_PRESS,
                combo_buffer[i].row, combo_buffer[i].col);
            (void) usb_keyboard_send_if_needed();
            break;
        }
    }
}

// Fire all completable unfired combos. Called after each key press.
// Returns true if at least one combo fired.
static bool
combo_try_fire_all (void) {
    bool any_fired = false;
    for (uint8_t i = 0; i < VIAL_COMBO_COUNT; ++i) {
        if (combo_fired[i]) {
            continue;
        }
        vial_combo_entry_t entry;
        if (dynamic_keymap_get_combo(i, &entry)) {
            continue;
        }
        uint8_t count = 0;
        for (uint8_t j = 0; j < VIAL_COMBO_INPUTS; ++j) {
            if (entry.input[j] == KC_NO) {
                break;
            }
            ++count;
        }
        bool can_fire;
        if (count < 2) {
            // Single-key: check if its single input is in buffer
            can_fire = false;
            for (uint8_t k = 0; k < combo_count; ++k) {
                if (combo_buffer[k].removed) {
                    continue;
                }
                uint16_t qmk = combo_buf_to_qmk(k);
                if (qmk == entry.input[0]) {
                    can_fire = true;
                    break;
                }
            }
        } else {
            can_fire = combo_all_inputs_present(i);
        }
        if (can_fire) {
            combo_fired[i] = true;
            any_fired = true;
            // Mark all non-removed entries as consumed by a fired combo
            for (uint8_t k = 0; k < combo_count; ++k) {
                if (!combo_buffer[k].removed) {
                    combo_buffer[k].was_fired = true;
                }
            }
            uint16_t output = combo_qmk_to_output(entry.output);
            combo_output_process(output, false);
            (void) usb_keyboard_send_if_needed();
        }
    }
    return any_fired;
}

// Check if a QMK keycode matches any input of a specific combo.
static bool
combo_key_matches_combo (uint16_t qmk_key, uint8_t combo_idx) {
    vial_combo_entry_t entry;
    if (dynamic_keymap_get_combo(combo_idx, &entry)) {
        return false;
    }
    for (uint8_t j = 0; j < VIAL_COMBO_INPUTS; ++j) {
        if (entry.input[j] == KC_NO) {
            break;
        }
        if (entry.input[j] == qmk_key) {
            return true;
        }
    }
    return false;
}

// Release all fired combos that include the given key.
// Returns true if any combo was released.
static bool
combo_release_fired_for_key (uint16_t qmk_key) {
    bool any = false;
    for (uint8_t i = 0; i < VIAL_COMBO_COUNT; ++i) {
        if (!combo_fired[i]) {
            continue;
        }
        if (!combo_key_matches_combo(qmk_key, i)) {
            continue;
        }
        combo_fired[i] = false;
        any = true;
        vial_combo_entry_t entry;
        if (dynamic_keymap_get_combo(i, &entry)) {
            continue;
        }
        uint16_t output = combo_qmk_to_output(entry.output);
        combo_output_process(output, true);
        (void) usb_keyboard_send_if_needed();
    }
    return any;
}

bool
combo_handle_press (uint16_t keycode, uint8_t physical_key, uint8_t row, uint8_t col, uint8_t data) {
    if (vial_combo_disabled) {
        return false;
    }

    // Convert to QMK keycode for matching
    uint16_t qmk_key = aakbd_to_qmk(keycode);
    if (qmk_key == KC_NO && is_extended_keycode(keycode)) {
        for (uint8_t layer = 0; layer < VIAL_LAYER_COUNT; ++layer) {
            uint16_t raw = dynamic_keymap_get_qmk_keycode(layer, row, col);
            if (raw != KC_NO && raw != KC_TRNS) {
                qmk_key = raw;
                break;
            }
        }
    }

    // Check if key is a trigger for ANY combo (fired or unfired)
    bool is_trigger = false;
    for (uint8_t i = 0; i < VIAL_COMBO_COUNT; ++i) {
        if (combo_key_matches_combo(qmk_key, i)) {
            is_trigger = true;
            break;
        }
    }

    if (!is_trigger) {
        // Not a combo trigger key
        if (combo_count > 0 && !combo_any_fired()) {
            if (IS_MODIFIER(keycode)) {
                // Modifier keys don't break pending combos
                return false;
            }
            // Stray key while combos pending — break all pending
            combo_flush_pending();
            combo_clear();
        }
        return false;
    }

    // ---- Key IS a combo trigger ----
    // If already in buffer, don't re-add
    for (uint8_t i = 0; i < combo_count; ++i) {
        if (combo_buffer[i].phys == physical_key && !combo_buffer[i].removed) {
            return false;
        }
    }

    // Add to buffer
    if (combo_count >= COMBO_BUFFER_MAX) {
        return false;
    }
    combo_buffer[combo_count].phys = physical_key;
    combo_buffer[combo_count].keycode = keycode;
    combo_buffer[combo_count].data = data;
    combo_buffer[combo_count].row = row;
    combo_buffer[combo_count].col = col;
    combo_buffer[combo_count].press_order = combo_count;
    combo_buffer[combo_count].removed = false;
    combo_buffer[combo_count].was_fired = false;
    ++combo_count;

    if (combo_count == 1) {
        combo_first_tick = current_10ms_tick_count();
    }

    // Try to fire any completable combos
    combo_try_fire_all();

    return true; // consumed
}

// Check if a buffer entry still counts towards any active combo.
// An entry is useful if there exists a combo (fired or still-possible)
// where this entry's key is an input AND all other inputs are present.
static bool
combo_entry_still_useful (uint8_t idx) {
    uint16_t entry_qmk = combo_buf_to_qmk(idx);
    for (uint8_t i = 0; i < VIAL_COMBO_COUNT; ++i) {
        if (!combo_key_matches_combo(entry_qmk, i)) {
            continue;
        }
        if (combo_fired[i]) {
            // Fired combo — all its inputs should still be in the buffer.
            // Check that all inputs are present (i.e., none were released).
            if (combo_all_inputs_present(i)) {
                return true;
            }
        } else {
            // Unfired combo — check if still possible
            if (combo_all_inputs_present(i)) {
                return true;
            }
        }
    }
    return false;
}

// Flush press of entries that no longer count towards any active combo.
// Compacts the buffer. Uses row=255,col=255 for flush presses.
static uint8_t
combo_flush_defunct (void) {
    combo_entry_t keep[COMBO_BUFFER_MAX];
    uint8_t keep_count = 0;
    for (uint8_t i = 0; i < combo_count; ++i) {
        if (combo_buffer[i].removed) {
            continue;
        }
        // Entries consumed by a fired combo are fully suppressed — no USB
        // output, no layer changes, no modifiers. Keep in buffer until
        // physical release fires postprocess_release.
        if (combo_buffer[i].was_fired || combo_entry_still_useful(i)) {
            keep[keep_count++] = combo_buffer[i];
        } else {
            process_keycode(combo_buffer[i].phys, combo_buffer[i].keycode, DEFERRED_PRESS,
                combo_buffer[i].row, combo_buffer[i].col);
            (void) usb_keyboard_send_if_needed();
        }
    }
    for (uint8_t i = 0; i < keep_count; ++i) {
        combo_buffer[i] = keep[i];
    }
    combo_count = keep_count;
    return keep_count;
}

// Returns true if key was part of a fired combo (caller goto postprocess).
bool
combo_handle_release (uint8_t physical_key, uint16_t *keycode, uint8_t *data) {
    if (vial_combo_disabled) {
        return false;
    }

    // Find key in buffer
    uint8_t idx = combo_count;
    for (uint8_t i = 0; i < combo_count; ++i) {
        if (combo_buffer[i].phys == physical_key && !combo_buffer[i].removed) {
            idx = i;
            break;
        }
    }
    if (idx >= combo_count) {
        return false;
    }

    uint16_t qmk_key = combo_buf_to_qmk(idx);
    combo_entry_t *entry = &combo_buffer[idx];

    bool had_fired = combo_any_fired();
    combo_release_fired_for_key(qmk_key);

    entry->removed = true;
    combo_flush_defunct();

    if (had_fired || entry->was_fired) {
        *keycode = entry->keycode;
        *data = entry->data;
        if (combo_count == 0) {
            combo_clear();
        }
        return true;
    }

    // Non-fired: flush deferred press so keybuffer has entry for normal release
    process_keycode(physical_key, entry->keycode, DEFERRED_PRESS, entry->row, entry->col);
    (void) usb_keyboard_send_if_needed();

    if (combo_count == 0) {
        combo_clear();
    }
    return false;
}

void
combo_task (uint8_t tick_10ms_count) {
    if (combo_any_fired() || combo_count == 0 || vial_combo_timeout_ms == 0) {
        return;
    }

    uint8_t elapsed = (uint8_t) (tick_10ms_count - combo_first_tick);
    if (elapsed <= (uint8_t) (vial_combo_timeout_ms / 10)) {
        return;
    }

    combo_flush_pending();
    combo_clear();
}
#endif

#if ENABLE_AUTOSHIFT
// Auto-shift: hold a key past the timeout to get the shifted version.

#define AS_MAX_KEYS 6

static struct {
    uint8_t key; // USB keycode (0 = free)
    uint16_t timer;
    bool shifted;
} as_keys[AS_MAX_KEYS];

// State tracking for repeat-path and interrupt-resolution.
static bool as_in_progress;
static uint16_t as_lastkey;
static uint16_t as_last_time;
static bool as_last_shifted;

void
vial_autoshift_reset (void) {
    as_in_progress = false;
    as_lastkey = 0;
    as_last_time = 0;
    as_last_shifted = false;
    for (int i = 0; i < AS_MAX_KEYS; ++i) {
        as_keys[i].key = 0;
    }
}

static bool
is_autoshiftable (uint8_t key) {
    if (key >= USB_KEY_A && key <= USB_KEY_Z) {
        return !(autoshift_flags & ASF_NO_ALPHA);
    }
    if (key >= USB_KEY_1 && key <= USB_KEY_0) {
        return !(autoshift_flags & ASF_NO_NUMERIC);
    }
    if (key >= USB_KEY_DASH && key <= USB_KEY_SLASH) {
        return !(autoshift_flags & ASF_NO_SPECIAL);
    }
    return false;
}

static int
as_find (uint8_t key) {
    for (int i = 0; i < AS_MAX_KEYS; ++i) {
        if (as_keys[i].key == key) {
            return i;
        }
    }
    return -1;
}

static int
as_alloc (void) {
    for (int i = 0; i < AS_MAX_KEYS; ++i) {
        if (!as_keys[i].key) {
            return i;
        }
    }
    return -1;
}

/// Resolve the currently pending key (if any) by sending its tap now.
/// The tap is shifted if the timeout has elapsed, unshifted otherwise.
static void
as_resolve_pending (void) {
    for (int i = 0; i < AS_MAX_KEYS; ++i) {
        if (!as_keys[i].key || as_keys[i].shifted) {
            continue;
        }
        bool shifted = timer_elapsed(as_keys[i].timer) >= autoshift_timeout_ms;
        if (shifted) {
            usb_keyboard_simulate_keypress(as_keys[i].key, SHIFT_BIT);
            as_keys[i].key = 0;
        } else {
            usb_keyboard_simulate_keypress(as_keys[i].key, 0);
            as_keys[i].key = 0;
        }
        as_last_shifted = shifted;
        as_in_progress = false;
        break;
    }
}

bool
vial_autoshift_process (keycode_t keycode, uint8_t key, bool is_release) {
    // Any non-modifier key press must resolve a pending autoshift key
    // first (e.g., Space breaks pending A).  Modifier keys pass through
    // without resolving the pending autoshift.
    if (!is_release && as_in_progress && !IS_MODIFIER(key)) {
        as_resolve_pending();
        (void) usb_keyboard_send_if_needed();
    }

    if (!(autoshift_enabled && is_autoshiftable(key))) {
        return false;
    }

    if (autoshift_flags & ASF_MODIFIERS) {
        if (strong_modifiers_mask() & BOTH_SHIFT_BITS) {
            // If physical shift is down, there is no point to autoshift
            return false;
        }
    } else if (strong_modifiers_mask()) {
        // Not enabled with modifiers
        return false;
    }

    if (is_release) {
        int idx = as_find(key);
        if (idx >= 0) {
            if (as_keys[idx].shifted) {
                // Key was held (ASF_AUTOREPEAT) — release it.
                // Direct modifier/key — bypass process_keycode (MODS_SHIFT|key
                // would match EXT_HYPER_MODIFIERS in the extended switch).
                usb_keyboard_release(as_keys[idx].key);
                // Keep shift if another key is pending (in_progress).
                // QMK: flush_shift called only when released key == lastkey.
                // Only clear shift if the released key is the last one.
                // QMK: flush_shift only when keycode == autoshift_lastkey.
                if (as_lastkey == 0 || key == (uint8_t) as_lastkey) {
                    remove_weak_modifiers(SHIFT_BIT);
                    register_modifiers();
                }
            } else {
                // Timeout hasn't fired — send unshifted tap
                usb_keyboard_simulate_keypress(as_keys[idx].key, 0);
            }
            as_keys[idx].key = 0;
            as_in_progress = false;
            return true;
        }
        // Not in as_keys → repeat-path key.  Let release go through
        // normal pipeline (register_key will clean up).
        return false;
    }

    // Repeat path: same key pressed within tapping term.
    uint16_t now = timer_read();
    if (as_lastkey && key == (uint8_t) as_lastkey && timer_elapsed(as_last_time) < autoshift_timeout_ms
        && (autoshift_flags & ASF_AUTOREPEAT)
        && (!as_last_shifted || (autoshift_flags & ASF_NO_AUTO_REPEAT))) {
        // Immediately register and hold the key with previous shift state.
        if (as_last_shifted) {
            add_weak_modifiers(SHIFT_BIT);
            register_modifiers();
        }
        usb_keyboard_press(key);
        return true; // consumed, held for OS repeat
    }

    // Normal auto-shift: buffer the key for timeout evaluation.
    int idx = as_alloc();
    if (idx < 0) {
        return false;
    }
    as_keys[idx] = (typeof(*as_keys)){ .key = key, .timer = now, .shifted = false };
    as_lastkey = key;
    as_last_time = now;
    as_in_progress = true;
    return true;
}

void
vial_autoshift_task (void) {
    if (!autoshift_enabled) {
        return;
    }

    for (int i = 0; i < AS_MAX_KEYS; ++i) {
        if (!as_keys[i].key || as_keys[i].shifted) {
            continue;
        }
        if (timer_elapsed(as_keys[i].timer) < autoshift_timeout_ms) {
            continue;
        }

        as_keys[i].shifted = true;
        if ((autoshift_flags & ASF_AUTOREPEAT) && !(autoshift_flags & ASF_NO_AUTO_REPEAT)) {
            // Hold shifted key down for OS key repeat.
            // Direct modifier/key — bypass process_keycode (MODS_SHIFT|key
            // would match EXT_HYPER_MODIFIERS in the extended switch).
            add_weak_modifiers(SHIFT_BIT);
            register_modifiers();
            usb_keyboard_press(as_keys[i].key);
        } else {
            // Send shifted tap (press + release) — key is not held down.
            // Keep the entry active so subsequent keys are not auto-shifted.
            usb_keyboard_simulate_keypress(as_keys[i].key, SHIFT_BIT);
        }
    }
}
#endif
