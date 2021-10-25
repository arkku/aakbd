/**
 * modelf.c: An alternative firmware for Brand New Model F keyboards.
 *
 * Copyright (c) 2021 Kimmo Kulovesi, https://arkku.dev/
 *
 * This program is free software: you can redistribute it and/or modify
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

#include <stdbool.h>
#include <stdint.h>

#include "led.h"

extern bool keyboard_scan_enabled;

void set_leds(int num_lock, int caps_lock, int scroll_lock);

void
matrix_init_user (void) {
    keyboard_scan_enabled = true;
}

void
led_init_ports (void) {
}

bool
led_update_user (led_t led_state) {
    return true;
}

void
led_set (uint8_t new_state) {
    led_t led = { .raw = new_state };

    set_leds(
        led.num_lock,
        led.caps_lock,
        led.scroll_lock
    );
}

#include "generic_hid.h"
#if ENABLE_GENERIC_HID_ENDPOINT
bool
make_generic_hid_report (uint8_t report_id, uint8_t count, uint8_t report[static count]) {
    return true;
}
#endif
