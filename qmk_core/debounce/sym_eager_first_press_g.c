/**
 * sym_eager_first_press_g: Like sym_defer_g, but if exactly one key is
 * pressed when no other debouncing is pending, eagerly pass it without delay.
 * This makes presses more responsive at the cost of less random noise
 * immunity: only noise following actual keypresses is suppressed.
 *
 * Copyright 2017 Alex Ong (original QMK sym_defer_g version)
 * Copyright 2021 Simon Arlott (original QMK sym_defer_g version)
 * Copyright 2026 Kimmo Kulovesi (sym_eager_first_press_g adaptation)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

#include "debounce.h"
#include "timer.h"
#include <string.h>

#ifndef DEBOUNCE
#    define DEBOUNCE 5
#endif

#if DEBOUNCE > 0
static bool debouncing = false;
static fast_timer_t debouncing_time;

void debounce_init(void) {
    debouncing = false;
    debouncing_time = 0;
}

bool debounce(matrix_row_t raw[], matrix_row_t cooked[], bool changed) {
    bool cooked_changed = false;

    if (changed) {
        if (!debouncing) {
            uint8_t keys_changed = 0;
            uint8_t changed_row = 0;

            for (uint8_t i = 0; keys_changed <= 1 && i < MATRIX_ROWS_PER_HAND; ++i) {
#ifdef EAGER_DEBOUNCE_ONLY_LONE_KEY
                if (cooked[i] != 0) {
                    // Already have a key down, debounce all other keys
                    keys_changed = 2;
                    break;
                }
#endif
                const matrix_row_t row_diff = raw[i] ^ cooked[i];
                if (row_diff) {
                    const matrix_row_t presses = raw[i] & ~cooked[i];
                    if (row_diff == presses) {
                        if (presses & (presses - 1)) {
                            // More than 1 press on this row
                            keys_changed = 2;
                            break;
                        }
                        // One press on this row
                        ++keys_changed;
                        changed_row  = i;
                    } else {
                        // A key has been released, debounce
                        keys_changed = 2;
                        break;
                    }
                }
            }

            if (keys_changed == 1) {
                // The only change is one new press, eagerly register it
                const matrix_row_t presses = raw[changed_row] & ~cooked[changed_row];
                if (presses) {
                    cooked[changed_row] |= presses;
                    cooked_changed = true;
#endif
                }
            }
        }

        // Regardless of whether a press was eagerly reported, start debounce
        debouncing = true;
        debouncing_time = timer_read_fast();
    } else if (debouncing && timer_elapsed_fast(debouncing_time) >= DEBOUNCE) {
        const size_t matrix_size = MATRIX_ROWS_PER_HAND * sizeof(matrix_row_t);
        if (memcmp(cooked, raw, matrix_size) != 0) {
            memcpy(cooked, raw, matrix_size);
            cooked_changed = true;
        }
        debouncing = false;
    }

    return cooked_changed;
}
#else // no debouncing.
#    include "none.c"
#endif
