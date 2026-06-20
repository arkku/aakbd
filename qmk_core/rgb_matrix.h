/**
 * rgb_matrix.h: QMK RGB matrix compatibility.
 *
 * This is free software: you can redistribute it and/or modify
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

#pragma once

#include <stdint.h>
#include <stdbool.h>

void rgb_matrix_init(void);
void rgb_matrix_task(void);
void rgb_matrix_set_suspend_state(bool state);
void process_rgb_matrix(uint8_t row, uint8_t col, bool pressed);

/// Set an LED by its hardware index.
void rgb_led_set(uint8_t index, uint8_t r, uint8_t g, uint8_t b);

/// Set the LED under a physical key by its USB keycode.
void rgb_led_set_by_keycode(uint8_t keycode, uint8_t r, uint8_t g, uint8_t b);
