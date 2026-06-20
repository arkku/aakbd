/**
 * host.h: AAKBD <-> QMK compatibility stub.
 */
#pragma once

#include <stdint.h>
#include <keys.h>

#define host_keyboard_leds() keys_led_state()
