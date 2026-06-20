/**
 * led_map.h: GMMK Pro keycode-to-LED mapping for AW20216S.
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
#include "aw20216s.h"

#if ISO_LAYOUT
#define AW20216S_LED_COUNT 99
#else
#define AW20216S_LED_COUNT 98
#endif

/// USB keycode to LED index, 0xFF = no LED.
extern uint8_t physical_key_to_led[256];
#define LED_NONE 0xFF

/// AW20216S LED register map (from QMK).
extern const aw20216s_led_t g_aw20216s_leds[AW20216S_LED_COUNT];

/// Build keycode to LED map from the default keymap.
void led_build_keycode_map(void);

/// Set LED by USB keycode — no-op if keycode has no LED.
void led_set_color_by_keycode(uint8_t keycode, uint8_t r, uint8_t g, uint8_t b);

// Edge LED indices (left side, back to front)
#define LED_EDGE_LEFT_1   68
#define LED_EDGE_LEFT_2   71
#define LED_EDGE_LEFT_3   74
#define LED_EDGE_LEFT_4   77
#define LED_EDGE_LEFT_5   81
#define LED_EDGE_LEFT_6   84
#define LED_EDGE_LEFT_7   88
#define LED_EDGE_LEFT_8   92

// Edge LED indices (right side, back to front)
#define LED_EDGE_RIGHT_1  69
#define LED_EDGE_RIGHT_2  72
#define LED_EDGE_RIGHT_3  75
#define LED_EDGE_RIGHT_4  78
#define LED_EDGE_RIGHT_5  82
#define LED_EDGE_RIGHT_6  85
#define LED_EDGE_RIGHT_7  89
#define LED_EDGE_RIGHT_8  93
