#pragma once

#include <stdbool.h>
#include "led.h"

bool led_update_kb(led_t led_state);
#define led_update_user(x) (1)
