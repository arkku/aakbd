/**
 * rgb_matrix.c: QMK RGB matrix compatibility.
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

#include "rgb_matrix.h"

__attribute__((weak)) void rgb_matrix_init(void) {}
__attribute__((weak)) void rgb_matrix_task(void) {}
__attribute__((weak)) void rgb_matrix_set_suspend_state(bool state) { (void)state; }
__attribute__((weak)) void process_rgb_matrix(uint8_t row, uint8_t col, bool pressed) { (void)row; (void)col; (void)pressed; }
__attribute__((weak)) void rgb_led_set(uint8_t index, uint8_t r, uint8_t g, uint8_t b) { (void)index; (void)r; (void)g; (void)b; }
__attribute__((weak)) void rgb_led_set_by_keycode(uint8_t keycode, uint8_t r, uint8_t g, uint8_t b) { (void)keycode; (void)r; (void)g; (void)b; }
