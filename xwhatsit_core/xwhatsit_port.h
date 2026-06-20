/**
 * xwhatsit_port.h: Helper for porting xwhatsit-based QMK keyboards to AAKBD.
 */

#pragma once

#include <stdbool.h>
#include "led.h"

bool led_update_kb(led_t led_state);
#define led_update_user(x) (1)
