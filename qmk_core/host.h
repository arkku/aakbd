#ifndef KK_QMK_HOST_H
#define KK_QMK_HOST_H

#include <stdint.h>
#include <keys.h>

#define host_keyboard_leds() keys_led_state()

#endif
