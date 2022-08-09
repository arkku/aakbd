/**
 * qmk_main.c: A main program for porting QMK keyboards to AAKBD engine.
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
#include <stdbool.h>

#include <usb_hardware.h>
#include <usbkbd.h>
#include <keys.h>
#include <aakbd.h>

#include "quantum.h"
#include "qmk_port.h"
#include "keyboard.h"

#include <progmem.h>
#include "platforms/bootloader.h"
#include "platforms/timer.h"
#include "platforms/suspend.h"
#include "platforms/usb_device_state.h"

#ifdef BACKLIGHT_ENABLE
#include "backlight.h"
#endif
#ifdef HAPTIC_ENABLE
#include "haptic.h"
#endif
#ifdef LED_MATRIX_ENABLE
#include "led_matrix.h"
#endif
#ifdef RGB_MATRIX_ENABLE
#include "rgb_matrix.h"
#endif
#ifdef ENCODER_ENABLE
#include "encoder.h"
#endif

#define STRIFY(a)           #a
#define STR(a)              STRIFY(a)

#ifdef KEYBOARD_NAME
const char KEYBOARD_FILENAME[] = STR(KEYBOARD_NAME)".c";
#endif

#define TICKS_PER_10MS      10

static uint16_t previous_tick_count;

uint8_t
current_10ms_tick_count (void) {
    return timer_read() / TICKS_PER_10MS;
}

#if defined(HAPTIC_ENABLE)
static void
process_haptic (uint8_t key, bool pressed) {
    if (haptic_get_enable() && !(HAPTIC_OFF_IN_LOW_POWER && usb_is_suspended())) {
        if (pressed) {
            if (haptic_get_feedback() < 2) {
                haptic_play();
            }
        } else {
            if (haptic_get_feedback() > 0) {
                haptic_play();
            }
        }
    }
}
#endif

static inline void
switch_events (uint8_t key, uint8_t row, uint8_t col, bool pressed) {
#if defined(HAPTIC_ENABLE)
#if HAPTIC_ONLY_BY_MACRO
#warning "Haptic feedback is enabled, but must be triggered from macros.c"
#else
    if (key) {
        process_haptic(key, pressed);
    }
#endif
#endif
#if defined(LED_MATRIX_ENABLE)
    process_led_matrix(row, col, pressed);
#endif
#if defined(RGB_MATRIX_ENABLE)
    process_rgb_matrix(row, col, pressed);
#endif
}

static matrix_row_t previous_matrix[MATRIX_ROWS] = { 0 };

static bool
kbd_input (void) {
    bool have_changes = matrix_scan();

    for (int_fast8_t row = 0; row < MATRIX_ROWS; ++row) {
        const matrix_row_t matrix_row = matrix_get_row(row);
        const matrix_row_t matrix_change = matrix_row ^ previous_matrix[row];
        if (matrix_change) {
#ifdef MATRIX_HAS_GHOST
            if (has_ghost_in_row(r, matrix_row)) {
                continue;
            }
#endif

            matrix_row_t column_bit = 1;
            for (int_fast8_t column = 0; column < MATRIX_COLS; ++column, column_bit <<= 1) {
                if (matrix_change & column_bit) {
                    const bool is_key_release = ((matrix_row & column_bit) == 0);
                    const uint8_t key = usb_keycode_for_matrix(row, column);
                    if (key) {
                        process_key(key, is_key_release);
                    }
                    switch_events(key, row, column, !is_key_release);
                }
            }
            previous_matrix[row] = matrix_row;
        }
    }
    return have_changes;
}

static void
protocol_init (void) {
    protocol_pre_init();
    usb_device_state_init();
    usb_init();
    previous_tick_count = timer_read();
    protocol_post_init();
}

void
suspend_power_down_quantum (void) {
    suspend_power_down_kb();

    uint8_t usb_config = usb_is_configured();
    usb_device_state_set_suspend(usb_config != 0, usb_config);

#ifdef BACKLIGHT_ENABLE
    backlight_set(0);
#endif
    led_suspend();
#if defined(LED_MATRIX_ENABLE)
    led_matrix_set_suspend_state(true);
#endif
#if defined(RGB_MATRIX_ENABLE)
    rgb_matrix_set_suspend_state(true);
#endif
}

void
suspend_wakeup_init_quantum (void) {
#ifdef BACKLIGHT_ENABLE
    backlight_init();
#endif

    // Restore LED indicators
    led_wakeup();

    uint8_t usb_config = usb_is_configured();
    usb_device_state_set_resume(usb_config != 0, usb_config);

#if defined(LED_MATRIX_ENABLE)
    led_matrix_set_suspend_state(false);
#endif
#if defined(RGB_MATRIX_ENABLE)
    rgb_matrix_set_suspend_state(false);
#endif

    suspend_wakeup_init_kb();
}

static inline void
keyboard_task (void) {
    const uint16_t now = timer_read();
    if (TIMER_DIFF_FAST(now, previous_tick_count) >= 10) {
        previous_tick_count = now;
        keys_tick(now / TICKS_PER_10MS);
    }

    (void) kbd_input();

#if defined(RGBLIGHT_ENABLE)
    rgblight_task();
#endif

#ifdef LED_MATRIX_ENABLE
    led_matrix_task();
#endif
#ifdef RGB_MATRIX_ENABLE
    rgb_matrix_task();
#endif

#if defined(BACKLIGHT_ENABLE)
#if defined(BACKLIGHT_PIN) || defined(BACKLIGHT_PINS)
    backlight_task();
#endif
#endif

#ifdef ENCODER_ENABLE
    (void) encoder_read();
#endif
#ifdef HAPTIC_ENABLE
    haptic_task();
#endif

    led_task();
}

static inline void
protocol_task (void) {
#ifndef NO_USB_STARTUP_CHECK
    if (usb_is_suspended()) {
        while (usb_is_suspended()) {
            suspend_power_down();
            if (suspend_wakeup_condition()) {
                if (usb_wake_up_host()) {
                    usb_keyboard_reset();
                    delay_milliseconds(200);
                }
            }
        }
        suspend_wakeup_init();
    }
#endif

    usb_tick();
}

void
usb_wake_up_interrupt (void) {
#ifdef NO_USB_STARTUP_CHECK
    suspend_wakeup_init();
#endif
}

void
usb_suspend_interrupt (void) {
#ifdef NO_USB_STARTUP_CHECK
    suspend_power_down();    
#endif
}

int
main (void) {
    platform_setup();
    protocol_setup();
    keyboard_setup();

    protocol_init();
    keyboard_init();

    for (;;) {
        protocol_task();
        keyboard_task();
    }
}

#ifdef ENABLE_I2C
#include "i2c_master.h"
#endif

static void
shutdown_quantum (void) {
    keyboard_init();

    // Tear down USB
    usb_deinit();

#ifdef ENABLE_I2C
    // Also stop I2C if we are using it
    i2c_stop();
#endif

    delay_milliseconds(32);

#ifdef HAPTIC_ENABLE
    haptic_shutdown();
#endif
}

void
keyboard_reset (void) {
    for (int_fast8_t row = 0; row < MATRIX_ROWS; ++row) {
        previous_matrix[row] = 0;
    }
    keyboard_init();
    delay_milliseconds(32);
}

void
jump_to_bootloader (void) {
    shutdown_quantum();
    bootloader_jump();
}

void
matrix_init_quantum (void) {
    reset_keys();
    matrix_init_kb();
}

void
matrix_scan_quantum (void) {
    matrix_scan_kb();
}

#ifdef ENCODER_ENABLE
#ifdef ENCODER_MAP_ENABLE
#error "Do not define ENCODER_MAP_ENABLE, see encoder_update_kb instead."
#endif

bool
encoder_update_kb(uint8_t index, bool clockwise) {
    // To support encoder, simulate keypress from here or implement _user:
    return encoder_update_user(index, clockwise);
}
#endif

bool
matrix_has_keys_pressed (void) {
    for (int_fast8_t row = 0; row < MATRIX_ROWS; ++row) {
        matrix_row_t matrix_row = matrix_get_row(row);
        if (matrix_row) {
            matrix_row_t column_bit = 1;
            for (int_fast8_t column = 0; column < MATRIX_COLS; ++column, column_bit <<= 1) {
                if (matrix_row & column_bit) {
                    const uint8_t key = usb_keycode_for_matrix(row, column);
                    if (key) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}
