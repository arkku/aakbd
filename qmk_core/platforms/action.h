/**
 * action.h: AAKBD compatibility header.
 */

#pragma once

#include <quantum.h>

#define clear_keyboard() keyboard_reset()

void keyboard_wake_up(void);
