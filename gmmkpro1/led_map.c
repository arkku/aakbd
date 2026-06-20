/**
 * led_map.c: GMMK Pro AW20216S LED register + keycode mapping.
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

#include "aw20216s.h"
#include "led_map.h"
#include "matrix.h"
#include "rgb_matrix.h"
#include "usb_keys.h"

#if ISO_LAYOUT
#include "led_map_iso.c"
#else
#include "led_map_ansi.c"
#endif

#if ISO_LAYOUT
static const uint8_t matrix_to_led[MATRIX_ROWS][MATRIX_COLS] = {
    {  4,255,255, 96, 65, 80,  5, 28 },
    {  8,  2,  9,  0, 10, 76,  1,  7 },
    { 14,  3, 15, 67, 16, 87,  6, 13 },
    { 20, 18, 21, 23, 22, 94, 12, 19 },
    { 25, 30, 26, 31, 27, 32, 29, 24 },
    { 41, 36, 42, 37, 43, 38, 35, 40 },
    { 46, 90, 47, 34, 48, 73, 79, 45 },
    { 52, 39, 53, 98, 54, 83, 44, 51 },
    { 58, 63, 59, 64, 95, 60, 62, 57 },
    { 11, 91, 55, 17, 33, 49,255, 70 },
    {255, 86,255, 61, 97, 66, 50, 56 },
};
#else
static const uint8_t matrix_to_led[MATRIX_ROWS][MATRIX_COLS] = {
    {  4,255,255, 95, 65, 79,  5, 28 },
    {  8,  2,  9,  0, 10, 75,  1,  7 },
    { 14,  3, 15,255, 16, 86,  6, 13 },
    { 20, 18, 21, 23, 22, 94, 12, 19 },
    { 25, 30, 26, 31, 27, 32, 29, 24 },
    { 41, 36, 42, 37, 43, 38, 35, 40 },
    { 46, 89, 47, 34, 48, 72, 78, 45 },
    { 52, 39, 53, 97, 54, 82, 44, 51 },
    { 58, 63, 59, 64,255, 60, 62, 57 },
    { 11, 90, 55, 17, 33, 49,255, 69 },
    {255, 85, 93, 61, 96, 66, 50, 56 },
};
#endif

uint8_t physical_key_to_led[256];

void led_build_keycode_map(void) {
    extern const uint8_t keymaps[][MATRIX_ROWS][MATRIX_COLS];

    for (uint16_t i = 0; i < 256; ++i)
        physical_key_to_led[i] = 0xFF;

    for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
        for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
            uint8_t led = matrix_to_led[row][col];
            if (led != 0xFF) {
                uint8_t kc = keymaps[0][row][col];
                if (kc) {
                    physical_key_to_led[kc] = led;
                }
            }
        }
    }
}

void led_set_color_by_keycode(uint8_t keycode, uint8_t r, uint8_t g, uint8_t b) {
    uint8_t led = physical_key_to_led[keycode];
    if (led != 0xFF) {
        aw20216s_set_color(led, r, g, b);
    }
}

void rgb_matrix_init(void) {
    aw20216s_init(g_aw20216s_leds, AW20216S_LED_COUNT);
    led_build_keycode_map();
}

void rgb_matrix_task(void) {
    aw20216s_flush();
}

void rgb_led_set(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    aw20216s_set_color(index, r, g, b);
}

void rgb_led_set_by_keycode(uint8_t keycode, uint8_t r, uint8_t g, uint8_t b) {
    led_set_color_by_keycode(keycode, r, g, b);
}
