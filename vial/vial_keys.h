/**
 * vial_keys.h: Vial hooks for AAKBD key processing (`keys.c`).
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
#include "usbkbd.h"
#include "keycodes.h"
#include "dynamic_keymap.h"

/// The total number of layers (dynamic + static).
uint8_t vial_total_layer_count(void);

/// Reads the keycode for `physical_key` on the given _static_ `layer`.
uint16_t vial_read_progmem_keycode(uint8_t layer, uint8_t physical_key);

/// Runtime one-shot tap toggle.
extern uint8_t oneshot_tap_toggle;

/// Runtime one-shot timeout in ms.
extern uint16_t oneshot_timeout_ms;

/// Runtime combo timeout in ms.
extern uint16_t vial_combo_timeout_ms;

/// Runtime tap-hold timeout in ms.
extern uint16_t vial_tap_hold_timeout_ms;

/// Mask of modifiers where the presence of any will force Grave Esc to
/// produce Esc. Configurable through Vial.
extern uint8_t grave_esc_override_mask;

#if ENABLE_AUTOSHIFT
extern uint16_t autoshift_timeout_ms;
extern uint8_t autoshift_flags;
#endif

#if VIAL_TAP_DANCE_COUNT > 0
/// Called periodically.
void vial_tap_dance_task(void);

/// Called to interrupt an in-progress tap-dance when another key is pressed.
void vial_tap_dance_interrupt(void);
#endif

#if ENABLE_AUTOSHIFT
/// Called by `process_keycode` on press and release.
bool vial_autoshift_process(keycode_t keycode, uint8_t key, bool is_release);

/// Called periodically.
void vial_autoshift_task(void);

/// Called to reset the autoshift state.
void vial_autoshift_reset(void);
#endif

#if VIAL_COMBO_COUNT > 0
/// Called from `process_keycode` on press.
bool combo_handle_press(uint16_t keycode, uint8_t physical_key, uint8_t row, uint8_t col, uint8_t data);

/// Called from `process_keycode` on release.
bool combo_handle_release(uint8_t physical_key, uint16_t *keycode, uint8_t *data);

/// Called periodically as part of `keys_tick()`.
void combo_task(uint8_t tick_10ms_count);

/// Called to reset the combo state.
void vial_reset_combo(void);
#endif

#ifndef VIAL_COMBO_TIMEOUT_MS
#define VIAL_COMBO_TIMEOUT_MS 200
#endif
#ifndef VIAL_TAP_HOLD_TIMEOUT_MS
#define VIAL_TAP_HOLD_TIMEOUT_MS 200
#endif
