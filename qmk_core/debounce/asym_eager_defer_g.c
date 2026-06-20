/**
 * asym_eager_defer_g: Key presses are registered eagerly with no debouncing,
 * but key releases are debounced with a single, global timer. This has no
 * noise immunity against random presses or crosstalk, but does suppress
 * single-switch bounce without the per-key overhead.
 *
 * Copyright 2026 Kimmo Kulovesi (adapted asym_eager_defer_g version)
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
#define DEBOUNCE 5
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

    // Eager press: immediately register any newly pressed keys
    for (uint8_t i = 0; i < MATRIX_ROWS_PER_HAND; i++) {
        matrix_row_t press = raw[i] & ~cooked[i];
        if (press) {
            cooked[i] |= press;
            cooked_changed = true;
        }
    }

    // Global debounce timer for releases: reset on any raw change
    if (changed) {
        debouncing = true;
        debouncing_time = timer_read_fast();
    }

    if (debouncing && timer_elapsed_fast(debouncing_time) >= DEBOUNCE) {
        for (uint8_t i = 0; i < MATRIX_ROWS_PER_HAND; i++) {
            matrix_row_t release = ~raw[i] & cooked[i];
            if (release) {
                cooked[i] &= ~release;
                cooked_changed = true;
            }
        }
        debouncing = false;
    }

    return cooked_changed;
}

#else
#include "none.c"
#endif
