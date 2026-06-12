#pragma once

#include <stdint.h>

typedef enum {
    USB_DEVICE_STATE_INIT = 0,
    USB_DEVICE_STATE_CONFIGURED,
} usb_device_state_t;

static inline usb_device_state_t usb_device_state_get_state(void) {
    return USB_DEVICE_STATE_CONFIGURED;
}

static inline uint8_t usb_device_state_get_configure_state(void) {
    return USB_DEVICE_STATE_CONFIGURED;
}
