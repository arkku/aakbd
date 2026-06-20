/**
 * encoder.c: Rotary encoder quadrature decoder.
 *
 * Polls encoder pins once per main loop iteration and decodes quadrature
 * signals using the standard lookup table method. Encoder events are
 * dispatched via `void handle_encoder_rotation(bool is_clockwise)`.
 * Implement the handler in `macros.c` to take action on rotation.
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

#include <stdint.h>
#include <stdbool.h>

#include "encoder.h"
#include "gpio.h"
#include "platform_deps.h"

#ifdef ENCODER_A_PIN
static const uint8_t encoder_count = 1;

static pin_t encoder_pin_a[1] = { ENCODER_A_PIN };
static pin_t encoder_pin_b[1] = { ENCODER_B_PIN };

static uint8_t encoder_state[1] = {0};
static int8_t  encoder_pulses[1] = {0};
#endif

static const int8_t encoder_LUT[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

// Weak default: override in macros.c to handle encoder rotation
__attribute__((weak)) void handle_encoder_rotation(bool is_clockwise) {
    (void) is_clockwise;
}

void encoder_init(void) {
#ifdef ENCODER_A_PIN
    gpio_set_pin_input_high(ENCODER_A_PIN);
    gpio_set_pin_input_high(ENCODER_B_PIN);

    for (uint8_t i = 0; i < encoder_count; ++i) {
        uint8_t a = gpio_read_pin(encoder_pin_a[i]) ? 1 : 0;
        uint8_t b = gpio_read_pin(encoder_pin_b[i]) ? 1 : 0;
        encoder_state[i] = (a << 0) | (b << 1);
    }

    (void) encoder_LUT;
#endif
}

void encoder_task(void) {
#ifdef ENCODER_A_PIN
    for (uint8_t i = 0; i < encoder_count; ++i) {
        uint8_t a = gpio_read_pin(encoder_pin_a[i]) ? 1 : 0;
        uint8_t b = gpio_read_pin(encoder_pin_b[i]) ? 1 : 0;
        uint8_t state = a | (b << 1);

        if ((encoder_state[i] & 0x3) != state) {
            encoder_state[i] = (encoder_state[i] << 2) | state;
            uint8_t lut_index = encoder_state[i] & 0xF;
            encoder_pulses[i] += encoder_LUT[lut_index];

            if (encoder_pulses[i] >= 4) {
                encoder_pulses[i] = 0;
                handle_encoder_rotation(false);
            } else if (encoder_pulses[i] <= -4) {
                encoder_pulses[i] = 0;
                handle_encoder_rotation(true);
            }
        }
    }
#endif
}
