/**
 * vial_magic.h: Vial/QMK "magic" key swap support layer for AAKBD.
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

#include "eeconfig.h"

#define VIAL_MAGIC_EEPROM_ADDR ((void *) EECONFIG_KEYMAP)

enum vial_magic_bit {
    VIAL_MAGIC_BIT_SWAP_CTRL_CAPS = 0,
    VIAL_MAGIC_BIT_CAPS_TO_CTRL = 1,
    VIAL_MAGIC_BIT_SWAP_LALT_LGUI = 2,
    VIAL_MAGIC_BIT_SWAP_RALT_RGUI = 3,
    VIAL_MAGIC_BIT_SWAP_ALT_GUI = 4,
    VIAL_MAGIC_BIT_SWAP_GRAVE_ESC = 5,
    VIAL_MAGIC_BIT_SWAP_BSLASH_BSPACE = 6,
    VIAL_MAGIC_BIT_NO_GUI = 7,
    VIAL_MAGIC_BIT_SWAP_LCTL_LGUI = 8,
    VIAL_MAGIC_BIT_SWAP_RCTL_RGUI = 9,
    VIAL_MAGIC_BIT_SWAP_CTL_GUI = 10,
    VIAL_MAGIC_BIT_SWAP_ESC_CAPS = 11,
};

#define VIAL_MAGIC_SWAP_CONTROL_CAPSLOCK      0x5C02
#define VIAL_MAGIC_UNSWAP_CONTROL_CAPSLOCK    0x5C0B
#define VIAL_MAGIC_CAPSLOCK_TO_CONTROL        0x5C03
#define VIAL_MAGIC_UNCAPSLOCK_TO_CONTROL      0x5C0C
#define VIAL_MAGIC_SWAP_LALT_LGUI             0x5C04
#define VIAL_MAGIC_UNSWAP_LALT_LGUI           0x5C0D
#define VIAL_MAGIC_SWAP_RALT_RGUI             0x5C05
#define VIAL_MAGIC_UNSWAP_RALT_RGUI           0x5C0E
#define VIAL_MAGIC_SWAP_ALT_GUI               0x5C0A
#define VIAL_MAGIC_TOGGLE_ALT_GUI             0x5C15
#define VIAL_MAGIC_SWAP_GRAVE_ESC             0x5C07
#define VIAL_MAGIC_UNSWAP_GRAVE_ESC           0x5C10
#define VIAL_MAGIC_SWAP_BACKSLASH_BACKSPACE   0x5C08
#define VIAL_MAGIC_UNSWAP_BACKSLASH_BACKSPACE 0x5C11
#define VIAL_MAGIC_NO_GUI                     0x5C06
#define VIAL_MAGIC_UNNO_GUI                   0x5C0F
#define VIAL_MAGIC_SWAP_LCTL_LGUI             0x5CFB
#define VIAL_MAGIC_SWAP_RCTL_RGUI             0x5CFC
#define VIAL_MAGIC_SWAP_CTL_GUI               0x5CFF
#define VIAL_MAGIC_TOGGLE_CTL_GUI             0x5D01

/// Load the current MAGIC config bitmask from EEPROM.
uint16_t vial_magic_load(void);

/// Save the MAGIC config bitmask to EEPROM.
void vial_magic_save(uint16_t mask);

/// Called to process a QMK MAGIC keycode.
/// - Returns: `true` if the keycode was handled.
bool vial_magic_process_keycode(uint16_t qmk_keycode);

/// Remap `physical_key` according to the current MAGIC state.
/// - Returns: The remapped key.
uint8_t vial_magic_remap_key(uint8_t physical_key);
