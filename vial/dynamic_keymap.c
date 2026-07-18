/**
 * dynamic_keymap.c: EEPROM-backed keymaps for AAKBD Vial compatibility.
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

#include "dynamic_keymap.h"
#include "qmk_keycodes.h"
#include "qmk_translate.h"
#include "progmem.h"
#include "usbkbd.h"
#include "aakbd.h"
#include "keys.h"
#include "vial.h"
#include "vial_keys.h"

#if ENABLE_PS2_DEVICE
#include "ps2_output.h"
#endif

#include "dynamic_storage.h"
#include <string.h>

/// The physical keymap from keymap.c
extern const uint8_t PROGMEM keymaps[1][MATRIX_ROWS][MATRIX_COLS];

uint8_t
dynamic_keymap_get_layer_count (void) {
    // Include the progmem layers so they show up as read-only
    return vial_total_layer_count();
}

int8_t
dynamic_keymap_find_matrix_pos (uint8_t physical_key, uint8_t row[static 1], uint8_t col[static 1]) {
    for (uint8_t r = 0; r < MATRIX_ROWS; ++r) {
        for (uint8_t c = 0; c < MATRIX_COLS; ++c) {
            if (pgm_read_byte(&keymaps[0][r][c]) == physical_key) {
                *row = r;
                *col = c;
                return 0;
            }
        }
    }
    return -1;
}

static void *
keycode_to_eeprom_address (uint8_t layer, uint8_t row, uint8_t col) {
    return VIA_KEYMAP_BASE + (layer * MATRIX_ROWS * MATRIX_COLS * 2) + (row * MATRIX_COLS * 2)
        + (col * 2);
}

uint16_t
dynamic_keymap_get_qmk_keycode (uint8_t layer, uint8_t row, uint8_t col) {
    // Vial uses 0-indexed layers. AAKBD PROGMEM layers start at VIAL_LAYER_COUNT+1.
    if (layer < VIAL_LAYER_COUNT && row < MATRIX_ROWS && col < MATRIX_COLS) {
        void *const addr = keycode_to_eeprom_address(layer, row, col);
        uint16_t keycode = (uint16_t) eeprom_read_byte(addr) << 8;
        keycode |= eeprom_read_byte(addr + 1);
        return keycode;
    }

    // This is used to show the static layers in Vial configurator. They can't
    // be edited, but they can still show there!
    if (layer < vial_total_layer_count() && row < MATRIX_ROWS && col < MATRIX_COLS) {
        const uint8_t physical_key = pgm_read_byte(&keymaps[0][row][col]);
        const uint16_t aakbd_key = vial_read_progmem_keycode(layer + 1, physical_key);
        return aakbd_to_qmk(aakbd_key);
    }
    return 0;
}

void
dynamic_keymap_set_qmk_keycode (uint8_t layer, uint8_t row, uint8_t col, uint16_t keycode) {
    if (layer >= VIAL_LAYER_COUNT || row >= MATRIX_ROWS || col >= MATRIX_COLS) {
        return;
    }

    void *addr = keycode_to_eeprom_address(layer, row, col);
    eeprom_update_byte(addr, (uint8_t) (keycode >> 8));
    eeprom_update_byte(addr + 1, (uint8_t) (keycode & 0xFF));
}

void
dynamic_keymap_reset (void) {
    // Layer 0: copy the default layout from the physical keymap
    for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
        for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
            uint16_t qmk_key = aakbd_to_qmk(pgm_read_byte(&keymaps[0][row][col]));
            dynamic_keymap_set_qmk_keycode(0, row, col, qmk_key);
        }
    }

    // All other layers initialized to blank / passthrough
    for (uint8_t layer = 1; layer < VIAL_LAYER_COUNT; ++layer) {
        for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
            for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
                dynamic_keymap_set_qmk_keycode(layer, row, col, KC_TRNS);
            }
        }
    }

#if VIAL_COMBO_COUNT > 0
    vial_combo_entry_t empty_combo = { 0 };
    for (uint8_t i = 0; i < VIAL_COMBO_COUNT; ++i) {
        dynamic_keymap_set_combo(i, &empty_combo);
    }
#endif

#if VIAL_TAP_DANCE_COUNT > 0
    vial_tap_dance_entry_t empty_td = { 0 };
    for (uint8_t i = 0; i < VIAL_TAP_DANCE_COUNT; ++i) {
        dynamic_keymap_set_tap_dance(i, &empty_td);
    }
#endif

#if NUM_ENCODERS > 0
    // Reset encoder map: volume up (CW) and volume down (CCW) on layer 0
    dynamic_keymap_set_encoder(0, 0, true, KC_AUDIO_VOL_UP);
    dynamic_keymap_set_encoder(0, 0, false, KC_AUDIO_VOL_DOWN);
    for (uint8_t layer = 1; layer < VIAL_LAYER_COUNT; ++layer) {
        for (uint8_t enc = 0; enc < NUM_ENCODERS; ++enc) {
            dynamic_keymap_set_encoder(layer, enc, true, KC_TRNS);
            dynamic_keymap_set_encoder(layer, enc, false, KC_TRNS);
        }
    }
#endif

    dynamic_keymap_layout_updated(0, dynamic_keymap_get_layout_options());
}

#if NUM_ENCODERS > 0
static void *
encoder_map_addr (uint8_t layer, uint8_t encoder_id) {
    return (uint8_t *) VIAL_ENCODERS_EEPROM_ADDR
        + (layer * NUM_ENCODERS + encoder_id) * VIAL_ENCODER_ENTRY_SIZE;
}

uint16_t
dynamic_keymap_get_encoder (uint8_t layer, uint8_t encoder_id, bool clockwise) {
    if (layer >= VIAL_LAYER_COUNT || encoder_id >= NUM_ENCODERS) {
        if (layer < vial_total_layer_count()) {
            // Static layer can't have encoders, show as transparent
            return KC_TRNS;
        }
        return 0;
    }
    void *addr = encoder_map_addr(layer, encoder_id);
    uint16_t keycode = (uint16_t) eeprom_read_byte((uint8_t *) addr + (clockwise ? 0 : 2)) << 8;
    keycode |= eeprom_read_byte((uint8_t *) addr + (clockwise ? 0 : 2) + 1);
    return keycode;
}

void
dynamic_keymap_set_encoder (uint8_t layer, uint8_t encoder_id, bool clockwise, uint16_t keycode) {
    if (layer >= VIAL_LAYER_COUNT || encoder_id >= NUM_ENCODERS) {
        return;
    }
    void *addr = encoder_map_addr(layer, encoder_id);
    eeprom_update_byte((uint8_t *) addr + (clockwise ? 0 : 2), (uint8_t) (keycode >> 8));
    eeprom_update_byte((uint8_t *) addr + (clockwise ? 0 : 2) + 1, (uint8_t) (keycode & 0xFF));
}
#endif // NUM_ENCODERS > 0

__attribute__((weak)) void
dynamic_keymap_layout_updated (uint16_t old_layout_options, uint16_t new_layout_options) {
}

uint16_t
dynamic_keymap_get_layout_options (void) {
    return eeprom_read_word(VIA_EEPROM_LAYOUT_OPTIONS_ADDR);
}

void
dynamic_keymap_set_layout_options (uint16_t opts) {
    eeprom_update_word(VIA_EEPROM_LAYOUT_OPTIONS_ADDR, opts);
}

void
dynamic_keymap_get_buffer (uint16_t offset, uint8_t size, uint8_t data[static size]) {
    const uint16_t one_layer_size = MATRIX_ROWS * MATRIX_COLS * 2U;
    const uint16_t eeprom_keymap_size = VIAL_LAYER_COUNT * one_layer_size;
    const uint16_t total_keymap_size = vial_total_layer_count() * one_layer_size;

    for (uint8_t i = 0; i < size; ++i) {
        if (offset < eeprom_keymap_size) {
            // Dynamic layers
            data[i] = eeprom_read_byte((uint8_t *) VIA_KEYMAP_BASE + offset);
        } else if (offset < total_keymap_size) {
            // Static layers
            const uint16_t progmem_offset = offset - eeprom_keymap_size;
            const uint8_t layer = VIAL_LAYER_COUNT + progmem_offset / (one_layer_size);
            const uint16_t pos_in_layer = progmem_offset % (one_layer_size);
            const uint8_t row = pos_in_layer / (MATRIX_COLS * 2);
            const uint8_t col = (pos_in_layer % (MATRIX_COLS * 2)) / 2;
            const uint16_t kc = dynamic_keymap_get_qmk_keycode(layer, row, col);
            data[i] = (progmem_offset % 2 == 0) ? (kc >> 8) : (kc & 0xFFU);
        } else {
            data[i] = 0;
        }
        ++offset;
    }
}

void
dynamic_keymap_set_buffer (uint16_t offset, uint8_t size, const uint8_t data[static size]) {
    uint16_t eeprom_keymap_size = VIAL_LAYER_COUNT * MATRIX_ROWS * MATRIX_COLS * 2;
    while (size--) {
        if (offset < eeprom_keymap_size) {
            // Only dynamic layers can be written to
            eeprom_update_byte((uint8_t *) VIA_KEYMAP_BASE + offset, *data++);
        }
        ++offset;
    }
}

#if VIAL_COMBO_COUNT > 0
static int
combo_eeprom_read (uint8_t index, vial_combo_entry_t entry[static 1]) {
    if (index >= VIAL_COMBO_COUNT) {
        return -1;
    }
    eeprom_read_block(entry, &VIAL_COMBO_EEPROM_ADDR[index], sizeof(vial_combo_entry_t));
    return 0;
}

static int
combo_eeprom_write (uint8_t index, const vial_combo_entry_t entry[static 1]) {
    if (index >= VIAL_COMBO_COUNT) {
        return -1;
    }
    eeprom_write_block(entry, &VIAL_COMBO_EEPROM_ADDR[index], sizeof(vial_combo_entry_t));
    return 0;
}

int
dynamic_keymap_get_combo (uint8_t index, vial_combo_entry_t entry[static 1]) {
    return combo_eeprom_read(index, entry);
}

int
dynamic_keymap_set_combo (uint8_t index, const vial_combo_entry_t entry[static 1]) {
    return combo_eeprom_write(index, entry);
}
#endif // ^ VIAL_COMBO_COUNT > 0

#if VIAL_TAP_DANCE_COUNT > 0
int
dynamic_keymap_get_tap_dance (uint8_t index, vial_tap_dance_entry_t entry[static 1]) {
    if (index >= VIAL_TAP_DANCE_COUNT) {
        return -1;
    }
    eeprom_read_block(entry, &VIAL_TAP_DANCE_EEPROM_ADDR[index], VIAL_TAP_DANCE_ENTRY_SIZE);
    return 0;
}

int
dynamic_keymap_set_tap_dance (uint8_t index, const vial_tap_dance_entry_t entry[static 1]) {
    if (index >= VIAL_TAP_DANCE_COUNT) {
        return -1;
    }
    eeprom_write_block(entry, &VIAL_TAP_DANCE_EEPROM_ADDR[index], VIAL_TAP_DANCE_ENTRY_SIZE);
    return 0;
}
#endif // ^ VIAL_TAP_DANCE_COUNT > 0

#if VIAL_MACRO_COUNT > 0
#if !ENABLE_SIMULATED_TYPING
#error "VIAL_MACRO_COUNT > 0 requires ENABLE_SIMULATED_TYPING"
#endif

uint8_t
dynamic_keymap_macro_get_count (void) {
    return VIAL_MACRO_COUNT;
}

uint16_t
dynamic_keymap_macro_get_buffer_size (void) {
    return VIAL_MACRO_EEPROM_SIZE;
}

void
dynamic_keymap_macro_get_buffer (uint16_t offset, uint8_t size, uint8_t data[static size]) {
    while (size-- && offset < VIAL_MACRO_EEPROM_SIZE) {
        *data++ = eeprom_read_byte(VIAL_MACRO_EEPROM_ADDR + offset);
        ++offset;
    }
}

void
dynamic_keymap_macro_set_buffer (uint16_t offset, uint8_t size, const uint8_t data[static size]) {
    while (size-- && offset < VIAL_MACRO_EEPROM_SIZE) {
        eeprom_update_byte(VIAL_MACRO_EEPROM_ADDR + offset, *data++);
        ++offset;
    }
}

void
dynamic_keymap_macro_reset (void) {
    void *p = VIAL_MACRO_EEPROM_ADDR;
    void *end = VIAL_DATA_END_ADDR;
    while (p <= end) {
        eeprom_update_byte(p++, 0);
    }
}

// SS_QMK_PREFIX and friends for macro special sequences
// (defined in dynamic_keymap.h)

static void
macro_send_qmk_keycode (uint16_t qmk_key, bool is_release) {
    if (!qmk_key || qmk_key == KC_NO) {
        return;
    }
    const keycode_t aakbd_key = qmk_to_aakbd(qmk_key);
    if (!aakbd_key) {
        return;
    }
    process_keycode(0, aakbd_key, is_release, 0, 0);
    (void) usb_keyboard_send_report();
    usb_keyboard_keypress_delay();
}

void
dynamic_keymap_macro_send (uint8_t id) {
    // Find the Nth NUL-terminated string in the macro buffer
    void *p = VIAL_MACRO_EEPROM_ADDR;
    void *end = VIAL_DATA_END_ADDR;

    // Skip NULs to find the Nth macro
    while (id > 0) {
        if (p > end) {
            return;
        }
        if (eeprom_read_byte(p) == 0) {
            --id;
        }
        ++p;
    }

    // Play back the macro
    while (p <= end) {
        uint8_t c = eeprom_read_byte(p++);
        if (c == 0) {
            break; // End of this macro
        }
        if (c == SS_QMK_PREFIX) {
            uint16_t kc;
            uint8_t next = eeprom_read_byte(p++);
            if (next == 0) {
                break;
            }
            kc = eeprom_read_byte(p++);

            switch (next) {
                case SS_TAP_CODE:
                    macro_send_qmk_keycode(kc, false);
                    macro_send_qmk_keycode(kc, true);
                    break;
                case SS_DOWN_CODE:
                    macro_send_qmk_keycode(kc, false);
                    break;
                case SS_UP_CODE:
                    macro_send_qmk_keycode(kc, true);
                    break;
                case SS_DELAY_CODE: {
                    const uint8_t hi = eeprom_read_byte(p++);
                    if (kc == 0 || hi == 0) {
                        p = end + 1;
                        break;
                    }
                    kc = (kc - 1) + ((uint16_t) (hi - 1)) * 255U;
                    while (kc--) {
                        delay_milliseconds(1);
#if ENABLE_PS2_DEVICE
                        ps2_output_task();
#endif
                    }
                    break;
                }
                case VIAL_MACRO_EXT_TAP:
                case VIAL_MACRO_EXT_DOWN:
                case VIAL_MACRO_EXT_UP: {
                    const uint8_t hi = eeprom_read_byte(p++);
                    kc |= (uint16_t) hi << 8;
                    if (kc > 0xFF00U) {
                        kc = (uint16_t) (kc & 0xFF) << 8;
                    }
                    bool is_down = (next != VIAL_MACRO_EXT_UP);
                    macro_send_qmk_keycode(kc, !is_down);
                    if (next == VIAL_MACRO_EXT_TAP) {
                        macro_send_qmk_keycode(kc, true);
                    }
                    break;
                }
            }
        } else {
            // Regular character — type it using simulated typing
            usb_keyboard_type_char(c);
        }
    }
}

#else // ^ VIAL_MACRO_COUNT > 0
void
dynamic_keymap_macro_send (uint8_t id) {
}
#endif
