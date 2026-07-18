// SPDX-License-Identifier: GPL-3.0-or-later
// gmmkpro1.c: GMMK Pro device-specific hooks and generic HID endpoint.

#include "generic_hid.h"
#include "keys.h"
#include "usb_hardware.h"
#include "rgb_matrix.h"
#include "aw20216s.h"
#include "led_map.h"
#include "gpio.h"
#include "wait.h"

#if ENABLE_GENERIC_HID_ENDPOINT

#define USB_KEYBOARD_ACCESS_STATE
#include "usbkbd.h"

#if !VIAL_ENABLE
enum generic_request {
    GENERIC_REQUEST_NONE,
    GENERIC_REQUEST_JUMP_TO_BOOTLOADER,
};

uint8_t
handle_generic_hid_report (uint8_t report_id, uint8_t count,
                           uint8_t report[static count],
                           uint8_t response_length[static 1],
                           uint8_t response[static *response_length]) {
    if (count == 0) {
        return RESPONSE_OK;
    }
    switch (report[0]) {
    case GENERIC_REQUEST_NONE:
        return RESPONSE_OK;
    case GENERIC_REQUEST_JUMP_TO_BOOTLOADER:
        return RESPONSE_JUMP_TO_BOOTLOADER;
    default:
        return RESPONSE_ERROR;
    }
}
#endif // ^ !VIAL_ENABLE

bool
make_generic_hid_report (uint8_t report_id, uint8_t count,
                         uint8_t report[static count]) {
#if !VIAL_ENABLE
    if (count < 8) {
        return false;
    }
    report[0] = usb_last_error();
    report[1] = keys_error();
    report[2] = usb_keyboard_is_in_boot_protocol;
    report[3] = usb_keyboard_leds;
    report[4] = usb_keys_modifier_flags;
    report[5] = usb_keys_extended_flags & 0xFF;
    report[6] = MATRIX_COLS;
    report[7] = MATRIX_ROWS;
    return true;
#else
    return false;
#endif
}

#endif // ^ ENABLE_GENERIC_HID_ENDPOINT

void
rgb_matrix_set_suspend_state (bool is_suspended) {
    if (is_suspended) {
        gpio_write_pin_low(AW20216S_EN_PIN);
    } else {
        gpio_write_pin_high(AW20216S_EN_PIN);
        wait_us(1);
        aw20216s_init(g_aw20216s_leds, AW20216S_LED_COUNT);
    }
}
