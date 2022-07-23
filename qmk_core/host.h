#ifndef KK_QMK_HOST_H
#define KK_QMK_HOST_H

#include <stdint.h>

extern uint8_t keyboard_leds;

#define host_keyboard_leds() (keyboard_leds)

#endif
