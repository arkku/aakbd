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
#include "bootloader.h"
#include "keyboard.h"
#include "progmem.h"
#include "suspend.h"

#define TIMER_NUM 1
#include <avrtimer.h>

#define STRIFY(a)           #a
#define STR(a)              STRIFY(a)

#ifdef KEYBOARD_NAME
const char KEYBOARD_FILENAME[] = STR(KEYBOARD_NAME)".c";
#endif

#define TICKS_PER_SECOND    (F_CPU / 1024UL)
#define TICKS_PER_10MS      ((TICKS_PER_SECOND / 100UL) + (((TICKS_PER_SECOND % 100UL) >= 50UL) ? 1UL : 0UL)

static volatile uint8_t tick_10ms_count = 0;
static uint8_t previous_tick;

uint8_t
current_10ms_tick_count (void) {
    return tick_10ms_count;
}

// A state that changes over time, can be used to blink LEDs.
// This fires approx. once per 10 ms, i.e., 100 times per second
ISR (TIMER_COMPA_VECTOR) {
    ++tick_10ms_count;
}

#define tick_is_due_at(count)   ((previous_tick - (count)) & 0x80U)

static bool
kbd_input (void) {
    static matrix_row_t previous_matrix[MATRIX_ROWS];
    matrix_row_t matrix_change = 0;

    bool have_changes = matrix_scan();

    for (int_fast8_t row = 0; row < MATRIX_ROWS; ++row) {
        matrix_row_t matrix_row = matrix_get_row(row);
        matrix_change = matrix_row ^ previous_matrix[row];
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
                    previous_matrix[row] ^= column_bit;
                }
            }
        }
    }
    return have_changes;
}

static void
protocol_init (void) {
    protocol_pre_init();
    usb_init();
    previous_tick = tick_10ms_count - 1;
    protocol_post_init();
}

void
suspend_power_down_quantum (void) {
    suspend_power_down_kb();

#ifdef BACKLIGHT_ENABLE
    backlight_set(0);
#endif
    led_suspend();
#if defined(RGBLIGHT_SLEEP) && defined(RGBLIGHT_ENABLE)
    rgblight_suspend();
#endif
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

// Wake up underglow
#if defined(RGBLIGHT_SLEEP) && defined(RGBLIGHT_ENABLE)
    rgblight_wakeup();
#endif

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
    const uint8_t now = tick_10ms_count;
    if (tick_is_due_at(now)) {
        keys_tick(now);
        previous_tick = now;
    }

    (void) kbd_input();

    led_task();
}

static inline void
protocol_task (void) {
    keyboard_task();
    usb_tick();
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

    timer_disable();

    wait_ms(32);

#ifdef HAPTIC_ENABLE
    haptic_shutdown();
#endif
}

void
keyboard_reset (void) {
    keyboard_init();
    wait_ms(32);
}

void
jump_to_bootloader (void) {
    shutdown_quantum();
    bootloader_jump();
}

void
matrix_setup() {
}

void
matrix_init_quantum() {
    reset_keys();
    matrix_init_kb();
}

void
matrix_scan_quantum() {
    matrix_scan_kb();
}

uint8_t
usb_keycode_for_matrix (const int8_t row, const int8_t column) {
    if (row < MATRIX_ROWS && column < MATRIX_COLS) {
        return pgm_read_byte(&keymaps[0][row][column]);
    } else {
        return KC_NO;
    }
}
