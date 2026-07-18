/**
 * vial.h: Vial/QMK compatibility layer for AAKBD.
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

#include <stdint.h>
#include <stdbool.h>
#include "dynamic_keymap.h"

#define VIAL_PROTOCOL_VERSION ((uint32_t) 0x00000006)
#define VIAL_RAW_EPSIZE       32

void vial_init(void);
void vial_handle_cmd(uint8_t data[static 32], uint8_t length);

// Look up a keycode for a Vial-managed layer from EEPROM.
// Returns the keycode in AAKBD format (after QMK→AAKBD translation).
// For layers >= VIAL_LAYER_COUNT, falls through to PROGMEM.
uint16_t vial_get_keycode_for_physical_key(uint8_t physical_key, uint8_t layer);

// Vial keycode lookup using matrix coordinates (fast, O(1) EEPROM read).
// Used by process_key when VIAL_ENABLE is set.
uint16_t vial_get_keycode_at(uint8_t row, uint8_t col, uint8_t layer);

/// Read a keycode from a PROGMEM (static) layer for Vial display.
/// Called to resolve and process an unsupported QMK keycode at the matrix
/// position. The keycode must be read from EEPROM.
void vial_process_qmk_keycode(uint8_t row, uint8_t col, uint8_t layer, bool is_release);

// Weak — users can override in macros.c to handle unsupported QMK keycodes.
void process_qmk_keycode(uint16_t qmk_keycode, bool is_release);

// Reset Vial EEPROM keymaps and config (overrides weak default in keys.c).
void eeconfig_init_via(void);
uint16_t vial_get_encoder_keycode(uint8_t encoder_id, bool clockwise);

#if defined(VIAL_INSECURE) && VIAL_INSECURE
#define vial_unlocked 1
#else
extern uint8_t vial_unlocked;
bool vial_is_unlock_in_progress(void);
#endif

enum {
    vial_get_keyboard_id = 0x00,
    vial_get_size = 0x01,
    vial_get_def = 0x02,
    vial_get_encoder = 0x03,
    vial_set_encoder = 0x04,
    vial_get_unlock_status = 0x05,
    vial_unlock_start = 0x06,
    vial_unlock_poll = 0x07,
    vial_lock = 0x08,
    vial_qmk_settings_query = 0x09,
    vial_qmk_settings_get = 0x0A,
    vial_qmk_settings_set = 0x0B,
    vial_qmk_settings_reset = 0x0C,
    vial_dynamic_entry_op = 0x0D,
};

// Subcommands for vial_dynamic_entry_op (data[2])
enum {
    dynamic_vial_get_number_of_entries = 0x00,
    dynamic_vial_tap_dance_get = 0x01,
    dynamic_vial_tap_dance_set = 0x02,
    dynamic_vial_combo_get = 0x03,
    dynamic_vial_combo_set = 0x04,
};
