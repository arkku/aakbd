/**
 * dynamic_keymap.h: EEPROM-backed keymaps for AAKBD Vial compatibility.
 *
 * Copyright (c) 2026 Kimmo Kulovesi, https://arkku.dev/
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "usbkbd_config.h"
#include "dynamic_storage.h"
#include "eeconfig.h"
#include "config.h"

/// Maximum dynamic Vial layers (cap to reserve space for static layers).
#ifndef VIAL_LAYER_COUNT_MAX
#define VIAL_LAYER_COUNT_MAX 12
#endif

/// The number of inputs (trigger keys) per combo.
#define VIAL_COMBO_INPUTS 4

#ifndef VIAL_COMBO_COUNT
/// The number of combos to support.
#define VIAL_COMBO_COUNT 4
#endif

#ifndef VIAL_MACRO_COUNT
/// The number of macros to support.
#define VIAL_MACRO_COUNT 16
#endif

#ifndef VIAL_ENABLE_MATRIX_TESTER
/// Whether to support the Vial matrix tester (makes the firmware larger and
/// in theory can be used for key logging if not in secure mode).
#define VIAL_ENABLE_MATRIX_TESTER 1
#endif

#ifndef NUM_ENCODERS
/// The number of rotary encoders.
#define NUM_ENCODERS 0
#endif

#ifndef VIAL_TAP_DANCE_COUNT
/// The number of tap dance keys to support.
#define VIAL_TAP_DANCE_COUNT 4
#endif

#ifndef VIAL_MACRO_RESERVE_BYTES
/// Minimum macro buffer size (it gets only what is left over after
/// everything else).
#define VIAL_MACRO_RESERVE_BYTES 16
#endif

struct __attribute__((packed)) vial_qmk_settings {
    uint16_t settings_canary;
    uint8_t grave_esc_override;
    uint8_t tap_toggle;
    uint8_t timeout_div_20;
    uint8_t autoshift_flags;
    uint16_t autoshift_timeout;
    uint16_t tapping_term;
    uint16_t combo_term;
};

typedef struct {
    uint16_t input[VIAL_COMBO_INPUTS];
    uint16_t output;
} vial_combo_entry_t;

typedef struct {
    uint16_t on_tap;
    uint16_t on_hold;
    uint16_t on_double_tap;
    uint16_t on_tap_hold;
    uint16_t custom_tapping_term;
} vial_tap_dance_entry_t;

// MARK: - EEPROM Layout

// EEPROM layout (low → high address):
// - QMK EEPROM config (0 .. EECONFIG_SIZE-1)
// - VIA magic (3 bytes at EECONFIG_SIZE)
// - Layout options (2 bytes at EECONFIG_SIZE + 3)
// - Dynamic keymaps (VIAL_LAYER_COUNT layers at VIA_KEYMAP_BASE)
// - Combo entries (VIAL_COMBO_COUNT * entry_size)
// - Tap dance entries (VIAL_TAP_DANCE_COUNT * entry_size)
// - Encoder map (NUM_ENCODERS * VIAL_LAYER_COUNT * 4)
// - QMK settings (sizeof vial_qmk_settings)
// - Macro buffer (variable, grows to EEPROM_MAX)
// - EEPROM_MAX

#define VIA_EEPROM_MAGIC_SIZE          3
#define VIA_EEPROM_LAYOUT_OPTIONS_SIZE 2

struct via_eeprom_header {
    uint8_t magic[VIA_EEPROM_MAGIC_SIZE];
    uint8_t layout_options[VIA_EEPROM_LAYOUT_OPTIONS_SIZE];
};

#define VIA_EEPROM_HEADER_SIZE (VIA_EEPROM_MAGIC_SIZE + VIA_EEPROM_LAYOUT_OPTIONS_SIZE)
_Static_assert(
    sizeof(struct via_eeprom_header) == VIA_EEPROM_HEADER_SIZE, "VIA EEPROM header size mismatch");

#define VIA_EEPROM_MAGIC_ADDR ((void *) (EECONFIG_SIZE))
#define VIA_EEPROM_LAYOUT_OPTIONS_ADDR \
    ((void *) (EECONFIG_SIZE + offsetof(struct via_eeprom_header, layout_options)))
#define VIA_KEYMAP_BASE ((void *) (EECONFIG_SIZE + sizeof(struct via_eeprom_header)))

#define VIAL_QMK_SETTINGS_SIZE 12
_Static_assert(
    sizeof(struct vial_qmk_settings) == VIAL_QMK_SETTINGS_SIZE, "QMK settings struct size mismatch");

#define VIAL_COMBO_ENTRY_SIZE_NUM (VIAL_COMBO_INPUTS * 2 + 2)
#define VIAL_COMBO_ENTRY_SIZE     sizeof(vial_combo_entry_t)
_Static_assert(VIAL_COMBO_ENTRY_SIZE == VIAL_COMBO_ENTRY_SIZE_NUM,
    "vial_combo_entry_t size mismatch with numeric calculation");
#define VIAL_COMBO_TOTAL_SIZE (VIAL_COMBO_COUNT * VIAL_COMBO_ENTRY_SIZE_NUM)

#define VIAL_ENCODER_ENTRY_SIZE (2 * 2)

#define VIAL_TAP_DANCE_ENTRY_SIZE_NUM (5 * 2)
#define VIAL_TAP_DANCE_ENTRY_SIZE     sizeof(vial_tap_dance_entry_t)
_Static_assert(VIAL_TAP_DANCE_ENTRY_SIZE == VIAL_TAP_DANCE_ENTRY_SIZE_NUM,
    "vial_tap_dance_entry_t size mismatch with numeric calculation");
#define VIAL_TAP_DANCE_OVERHEAD (VIAL_TAP_DANCE_COUNT * VIAL_TAP_DANCE_ENTRY_SIZE_NUM)

#define BYTES_PER_LAYER (MATRIX_ROWS * MATRIX_COLS * 2)
#if VIAL_MACRO_COUNT > 0
#define VIAL_LAYER_COUNT \
    ((EEPROM_MAX - (EECONFIG_SIZE + VIA_EEPROM_HEADER_SIZE) - VIAL_QMK_SETTINGS_SIZE \
         - VIAL_COMBO_TOTAL_SIZE - VIAL_TAP_DANCE_OVERHEAD - VIAL_MACRO_RESERVE_BYTES) \
        / BYTES_PER_LAYER)
#else
#define VIAL_LAYER_COUNT \
    ((EEPROM_MAX - (EECONFIG_SIZE + VIA_EEPROM_HEADER_SIZE) - VIAL_QMK_SETTINGS_SIZE \
         - VIAL_COMBO_TOTAL_SIZE - VIAL_TAP_DANCE_OVERHEAD) \
        / BYTES_PER_LAYER)
#endif

_Static_assert(VIAL_LAYER_COUNT >= 1, "Not enough EEPROM for even one Vial layer");

_Static_assert(EECONFIG_SIZE + VIA_EEPROM_HEADER_SIZE + VIAL_LAYER_COUNT * BYTES_PER_LAYER
            + VIAL_COMBO_TOTAL_SIZE + VIAL_TAP_DANCE_OVERHEAD + VIAL_QMK_SETTINGS_SIZE
            + NUM_ENCODERS * VIAL_LAYER_COUNT * VIAL_ENCODER_ENTRY_SIZE
        <= EEPROM_MAX,
    "Vial layout exceeds EEPROM");

#if VIAL_LAYER_COUNT > VIAL_LAYER_COUNT_MAX
#undef VIAL_LAYER_COUNT
#define VIAL_LAYER_COUNT VIAL_LAYER_COUNT_MAX
#endif

#define VIAL_KEYMAP_SIZE       (VIAL_LAYER_COUNT * BYTES_PER_LAYER)
#define VIAL_COMBO_OVERHEAD    (VIAL_COMBO_COUNT * VIAL_COMBO_ENTRY_SIZE)
#define VIAL_COMBO_EEPROM_ADDR ((vial_combo_entry_t *) ((uintptr_t) VIA_KEYMAP_BASE + VIAL_KEYMAP_SIZE))
#define VIAL_TAP_DANCE_EEPROM_ADDR \
    ((vial_tap_dance_entry_t *) ((uintptr_t) VIAL_COMBO_EEPROM_ADDR + VIAL_COMBO_OVERHEAD))
#define VIAL_TAP_DANCE_EEPROM_SIZE (VIAL_TAP_DANCE_COUNT * VIAL_TAP_DANCE_ENTRY_SIZE)

#define VIAL_ENCODERS_SIZE (NUM_ENCODERS * VIAL_LAYER_COUNT * VIAL_ENCODER_ENTRY_SIZE)
#define VIAL_ENCODERS_EEPROM_ADDR \
    ((void *) ((uintptr_t) VIAL_TAP_DANCE_EEPROM_ADDR + VIAL_TAP_DANCE_OVERHEAD))

// AAKBD user macros 0–63, Vial EEPROM macros start here.
#define VIAL_MACRO_START 64

#define VIAL_QMK_SETTINGS_ADDR ((void *) ((uintptr_t) VIAL_ENCODERS_EEPROM_ADDR + VIAL_ENCODERS_SIZE))
#define VIAL_MACRO_EEPROM_ADDR \
    ((uint8_t *) ((uintptr_t) VIAL_QMK_SETTINGS_ADDR + VIAL_QMK_SETTINGS_SIZE))
#define VIAL_DATA_END_ADDR     ((uint8_t *) (EEPROM_MAX - 1))
#define VIAL_MACRO_EEPROM_SIZE ((uint16_t) (EEPROM_MAX - (uintptr_t) VIAL_MACRO_EEPROM_ADDR))

#if VIAL_MACRO_COUNT > 0
_Static_assert(
    VIAL_MACRO_EEPROM_SIZE >= VIAL_MACRO_RESERVE_BYTES, "Not enough EEPROM for macro buffer");
#endif

// MARK: - Keymaps

/// The number of Vial-managed layers.
uint8_t dynamic_keymap_get_layer_count(void);

/// Read a QMK keycode from the EEPROM keymap.
/// - Returns: The QMK keycode.
uint16_t dynamic_keymap_get_qmk_keycode(uint8_t layer, uint8_t row, uint8_t col);

/// Write a QMK keycode to the EEPROM keymap.
void dynamic_keymap_set_qmk_keycode(uint8_t layer, uint8_t row, uint8_t col, uint16_t keycode);

/// Find the matrix position for a physical key.
/// - Returns: 0 on success, -1 if not found.
int8_t dynamic_keymap_find_matrix_pos(
    uint8_t physical_key, uint8_t row[static 1], uint8_t col[static 1]);

/// Reset Vial / dynamic keymaps to defaults.
void dynamic_keymap_reset(void);

/// Bulk-read the EEPROM keymap buffer.
void dynamic_keymap_get_buffer(uint16_t offset, uint8_t size, uint8_t data[static size]);

/// Bulk-write the EEPROM keymap buffer.
void dynamic_keymap_set_buffer(uint16_t offset, uint8_t size, const uint8_t data[static size]);

// MARK: - Layout Options

/// Called when layout options change (weak, for device-specific handling).
void dynamic_keymap_layout_updated(uint16_t old_layout_options, uint16_t new_layout_options);

/// Read layout options from EEPROM.
/// - Returns: The layout option bitmask.
uint16_t dynamic_keymap_get_layout_options(void);

/// Write layout options to EEPROM.
void dynamic_keymap_set_layout_options(uint16_t opts);

// MARK: - Encoders

/// A fake matrix row value used to signal rotary encoders.
#define ENCODERS_ROW 255

/// Read an encoder keycode from EEPROM.
/// - Returns: The QMK keycode.
uint16_t dynamic_keymap_get_encoder(uint8_t layer, uint8_t encoder_id, bool clockwise);

/// Write an encoder keycode to EEPROM.
void dynamic_keymap_set_encoder(uint8_t layer, uint8_t encoder_id, bool clockwise, uint16_t keycode);

// MARK: - Combos

#if VIAL_COMBO_COUNT > 0
_Static_assert(sizeof(vial_combo_entry_t) == VIAL_COMBO_TOTAL_SIZE / VIAL_COMBO_COUNT,
    "combo entry size mismatch");

/// Read a combo entry from EEPROM.
/// - Returns: 0 on success, -1 if not found.
int dynamic_keymap_get_combo(uint8_t index, vial_combo_entry_t entry[static 1]);

/// Write a combo entry to EEPROM.
/// - Returns: 0 on success, -1 if out of bounds.
int dynamic_keymap_set_combo(uint8_t index, const vial_combo_entry_t entry[static 1]);
#endif

// MARK: - Tap Dance

#if VIAL_TAP_DANCE_COUNT > 0
/// Read a tap dance entry from EEPROM.
/// - Returns: 0 on success, -1 if not found.
int dynamic_keymap_get_tap_dance(uint8_t index, vial_tap_dance_entry_t entry[static 1]);

/// Write a tap dance entry to EEPROM.
/// - Returns: 0 on success, -1 if out of bounds.
int dynamic_keymap_set_tap_dance(uint8_t index, const vial_tap_dance_entry_t entry[static 1]);
#endif

// MARK: - Macros

#define SS_QMK_PREFIX       0x01
#define SS_TAP_CODE         0x01
#define SS_DOWN_CODE        0x02
#define SS_UP_CODE          0x03
#define SS_DELAY_CODE       0x04
#define VIAL_MACRO_EXT_TAP  0x05
#define VIAL_MACRO_EXT_DOWN 0x06
#define VIAL_MACRO_EXT_UP   0x07

#if VIAL_MACRO_COUNT > 0
/// The number of EEPROM macros.
uint8_t dynamic_keymap_macro_get_count(void);

/// The size of the macro buffer.
uint16_t dynamic_keymap_macro_get_buffer_size(void);

/// Bulk-read macro buffer.
void dynamic_keymap_macro_get_buffer(uint16_t offset, uint8_t size, uint8_t data[static size]);

/// Bulk-write macro buffer.
void dynamic_keymap_macro_set_buffer(uint16_t offset, uint8_t size, const uint8_t data[static size]);

/// Reset all macros.
void dynamic_keymap_macro_reset(void);
#endif

/// Play back a macro by ID.
void dynamic_keymap_macro_send(uint8_t macro_id);

#if VIAL_MACRO_COUNT > 0 && !defined(ENABLE_SIMULATED_TYPING)
#define ENABLE_SIMULATED_TYPING 1
#endif

// MARK: - Spaghetti

// Any static layers (in addition to Vial dynamic layers) must be defined
// using `STATIC_LAYER_n` macros defined below:

#if VIAL_LAYER_COUNT == 1
#define LAST_VIAL_LAYER 1
#define STATIC_LAYER_1  2
#define STATIC_LAYER_2  3
#define STATIC_LAYER_3  4
#define STATIC_LAYER_4  5
#define STATIC_LAYER_5  6
#define STATIC_LAYER_6  7
#define STATIC_LAYER_7  8
#define STATIC_LAYER_8  9
#define STATIC_LAYER_9  10
#define STATIC_LAYER_10 11
#define STATIC_LAYER_11 12
#define STATIC_LAYER_12 13
#define STATIC_LAYER_13 14
#define STATIC_LAYER_14 15
#elif VIAL_LAYER_COUNT == 2
#define LAST_VIAL_LAYER 2
#define STATIC_LAYER_1  3
#define STATIC_LAYER_2  4
#define STATIC_LAYER_3  5
#define STATIC_LAYER_4  6
#define STATIC_LAYER_5  7
#define STATIC_LAYER_6  8
#define STATIC_LAYER_7  9
#define STATIC_LAYER_8  10
#define STATIC_LAYER_9  11
#define STATIC_LAYER_10 12
#define STATIC_LAYER_11 13
#define STATIC_LAYER_12 14
#define STATIC_LAYER_13 15
#elif VIAL_LAYER_COUNT == 3
#define LAST_VIAL_LAYER 3
#define STATIC_LAYER_1  4
#define STATIC_LAYER_2  5
#define STATIC_LAYER_3  6
#define STATIC_LAYER_4  7
#define STATIC_LAYER_5  8
#define STATIC_LAYER_6  9
#define STATIC_LAYER_7  10
#define STATIC_LAYER_8  11
#define STATIC_LAYER_9  12
#define STATIC_LAYER_10 13
#define STATIC_LAYER_11 14
#define STATIC_LAYER_12 15
#elif VIAL_LAYER_COUNT == 4
#define LAST_VIAL_LAYER 4
#define STATIC_LAYER_1  5
#define STATIC_LAYER_2  6
#define STATIC_LAYER_3  7
#define STATIC_LAYER_4  8
#define STATIC_LAYER_5  9
#define STATIC_LAYER_6  10
#define STATIC_LAYER_7  11
#define STATIC_LAYER_8  12
#define STATIC_LAYER_9  13
#define STATIC_LAYER_10 14
#define STATIC_LAYER_11 15
#elif VIAL_LAYER_COUNT == 5
#define LAST_VIAL_LAYER 5
#define STATIC_LAYER_1  6
#define STATIC_LAYER_2  7
#define STATIC_LAYER_3  8
#define STATIC_LAYER_4  9
#define STATIC_LAYER_5  10
#define STATIC_LAYER_6  11
#define STATIC_LAYER_7  12
#define STATIC_LAYER_8  13
#define STATIC_LAYER_9  14
#define STATIC_LAYER_10 15
#elif VIAL_LAYER_COUNT == 6
#define LAST_VIAL_LAYER 6
#define STATIC_LAYER_1  7
#define STATIC_LAYER_2  8
#define STATIC_LAYER_3  9
#define STATIC_LAYER_4  10
#define STATIC_LAYER_5  11
#define STATIC_LAYER_6  12
#define STATIC_LAYER_7  13
#define STATIC_LAYER_8  14
#define STATIC_LAYER_9  15
#elif VIAL_LAYER_COUNT == 7
#define LAST_VIAL_LAYER 7
#define STATIC_LAYER_1  8
#define STATIC_LAYER_2  9
#define STATIC_LAYER_3  10
#define STATIC_LAYER_4  11
#define STATIC_LAYER_5  12
#define STATIC_LAYER_6  13
#define STATIC_LAYER_7  14
#define STATIC_LAYER_8  15
#elif VIAL_LAYER_COUNT == 8
#define LAST_VIAL_LAYER 8
#define STATIC_LAYER_1  9
#define STATIC_LAYER_2  10
#define STATIC_LAYER_3  11
#define STATIC_LAYER_4  12
#define STATIC_LAYER_5  13
#define STATIC_LAYER_6  14
#define STATIC_LAYER_7  15
#elif VIAL_LAYER_COUNT == 9
#define LAST_VIAL_LAYER 9
#define STATIC_LAYER_1  10
#define STATIC_LAYER_2  11
#define STATIC_LAYER_3  12
#define STATIC_LAYER_4  13
#define STATIC_LAYER_5  14
#define STATIC_LAYER_6  15
#elif VIAL_LAYER_COUNT == 10
#define LAST_VIAL_LAYER 10
#define STATIC_LAYER_1  11
#define STATIC_LAYER_2  12
#define STATIC_LAYER_3  13
#define STATIC_LAYER_4  14
#define STATIC_LAYER_5  15
#elif VIAL_LAYER_COUNT == 11
#define LAST_VIAL_LAYER 11
#define STATIC_LAYER_1  12
#define STATIC_LAYER_2  13
#define STATIC_LAYER_3  14
#define STATIC_LAYER_4  15
#elif VIAL_LAYER_COUNT == 12
#define LAST_VIAL_LAYER 12
#define STATIC_LAYER_1  13
#define STATIC_LAYER_2  14
#define STATIC_LAYER_3  15
#elif VIAL_LAYER_COUNT == 13
#define LAST_VIAL_LAYER 13
#define STATIC_LAYER_1  14
#define STATIC_LAYER_2  15
#elif VIAL_LAYER_COUNT == 14
#define LAST_VIAL_LAYER 14
#define STATIC_LAYER_1  15
#elif VIAL_LAYER_COUNT == 15
#define LAST_VIAL_LAYER 15
#else
#error "STATIC_LAYER_1: unexpected VIAL_LAYER_COUNT"
#endif
