/**
 * ergodox.c: Ergodox Ez QMK to AAKBD adapter.
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

#include <stdint.h>

#include "ergodox_ez.h"
#include "led.h"
#include "generic_hid.h"

keyboard_config_t keyboard_config;

void
led_init_ports (void) {
    ergodox_led_all_on();
    ergodox_board_led_on();
}

void
led_set (uint8_t new_state) {
    led_t led = { .raw = new_state };

    if (led.num_lock) {
        ergodox_right_led_1_on();
    } else {
        ergodox_right_led_1_off();
    }
    if (led.caps_lock) {
        ergodox_right_led_2_on();
    } else {
        ergodox_right_led_2_off();
    }
    if (led.scroll_lock) {
        ergodox_right_led_3_on();
    } else {
        ergodox_right_led_3_off();
    }
}

#if ENABLE_GENERIC_HID_ENDPOINT
#define INCLUDE_USB_HARDWARE_ACCESS
#include "usbkbd.h"
#include "keys.h"

enum generic_request {
    NONE,
    SET_LED_LEVEL,
    JUMP_TO_BOOTLOADER,
};

uint8_t
handle_generic_hid_report (uint8_t report_id, uint8_t count, uint8_t report[static count], uint8_t response_length[static 1], uint8_t response[static *response_length]) {
    if (count == 0 || GENERIC_HID_FEATURE_SIZE == 0) {
        return RESPONSE_OK;
    }

    enum generic_request request = report[0];

    switch (request) {
    case NONE:
        return RESPONSE_OK;
    case SET_LED_LEVEL:
        if (count < 1 || report[1] >= 16) {
            return RESPONSE_ERROR;
        }
        keyboard_config.led_level = report[1];
        eeconfig_update_kb(keyboard_config.raw);
        return RESPONSE_OK;
    case JUMP_TO_BOOTLOADER:
        return JUMP_TO_BOOTLOADER;
    default:
        return RESPONSE_ERROR;
    }
}

#if GENERIC_HID_REPORT_SIZE != 0 && GENERIC_HID_REPORT_SIZE != 8 && GENERIC_HID_REPORT_SIZE != (8 + MATRIX_ROWS)
#error "GENERIC_HID_REPORT_SIZE should be 0, 8, or 8 + MATRIX_ROWS."
#endif

extern matrix_row_t raw_matrix[MATRIX_ROWS];

bool
make_generic_hid_report (uint8_t report_id, uint8_t count, uint8_t report[static count]) {
#if GENERIC_HID_REPORT_SIZE
    if (count < 8) {
        return false;
    }
    report[0] = usb_last_error();
    report[1] = keys_error();
    report[2] = usb_is_in_boot_protocol();
    report[3] = mcp23018_status;
    report[4] = keyboard_config.led_level;
    report[5] = (keyboard_config.disable_layer_led ? 0 : 1) | (keyboard_config.rgb_matrix_enable ? 2 : 0); 
    report[6] = MATRIX_COLS;
    report[7] = MATRIX_ROWS;
    count -= 8;
    for (int_fast8_t i = 0; i < count && i < MATRIX_ROWS; ++i) {
        report[i + 8] = raw_matrix[i];
    }
#endif
    return true;
}
#endif
