/**
 * qmk_translate.h: Keycode translation between AAKBD and QMK/Vial.
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

#include "keycodes.h"

/// Convert `aakbd_keycode` to a QMK keycode.
/// - Returns: A QMK keycode, `KC_NO` for keycodes that QMK does not support.
uint16_t aakbd_to_qmk(keycode_t aakbd_keycode);

/// Convert `qmk_keycode` to an AAKBD keycode.
/// - Returns: An AAKBD keycode, `EXTENDED(QMK_KEYCODE)` for keycodes that
/// AAKBD doesn't support. (That code is handled in `keys.c.` and passed to
/// `vial_process_qmk_keycode(…)`.)
keycode_t qmk_to_aakbd(uint16_t qmk_keycode);
