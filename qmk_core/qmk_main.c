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

#if ENABLE_PS2_DEVICE
#include "ps2_output.h"
#endif

#include "quantum.h"
#include "qmk_port.h"
#include "keyboard.h"

#include <progmem.h>
#include "platforms/bootloader.h"
#include "platforms/timer.h"
#include "platforms/suspend.h"

#define USB_ENUMERATION_TIMEOUT_MS  1200U

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

#define TICKS_PER_10MS      10

static uint16_t previous_tick_count;

uint8_t
current_10ms_tick_count (void) {
    return timer_read() / TICKS_PER_10MS;
}

#ifdef HAPTIC_ENABLE
static uint8_t haptic_usb_is_configured = 0;
#if ENABLE_PS2_DEVICE
#define haptic_is_powered() (!HAPTIC_OFF_IN_LOW_POWER || (haptic_usb_is_configured && !usb_is_suspended()) || is_ps2_scanning_enabled)
#else
#define haptic_is_powered() (!HAPTIC_OFF_IN_LOW_POWER || (haptic_usb_is_configured && !usb_is_suspended()))
#endif

static void
process_haptic (uint8_t key, bool pressed) {
    if (haptic_get_enable() && haptic_is_powered()) {
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
#ifdef HAPTIC_ENABLE
#if HAPTIC_ONLY_BY_MACRO
#warning "Haptic feedback is enabled, but must be triggered from macros.c"
#else
    if (key) {
        process_haptic(key, pressed);
    }
#endif
#endif
#ifdef LED_MATRIX_ENABLE
    process_led_matrix(row, col, pressed);
#endif
#ifdef RGB_MATRIX_ENABLE
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
                        process_key(key, is_key_release, row, column);
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
#if ENABLE_PS2_DEVICE && ENABLE_FALLBACK_TO_PS2_FROM_USB
    // The timer is needed for the fallback, well, timer.
    // It is also called later in keyboard_init(), which resets it later.
    timer_init();
#endif
    protocol_pre_init();

#if ENABLE_PS2_DEVICE
    for (;;) {
        // Reset any previous USB attempt
        usb_bus_detach();
        usb_deinit();

        // Try PS/2 first
        ps2_device_init();
        if (ps2_device_host_detected()) {
            ps2_output_init();

            // NOTE: PS/2 and USB can't both be active at once. Make sure this is
            // the only place where either `usb_init()` or `ps2_output_init()` is
            // called and that they are mutually exclusive.
            protocol_post_init();
            return;
        }
#endif
        usb_init();
        previous_tick_count = timer_read();

        protocol_post_init();

        // Must be called with interrupts enabled so the USB ISRs can handle
        // the USB reset and the first SETUP immediately:
        usb_bus_attach();

#ifdef HAPTIC_ENABLE
        haptic_usb_is_configured = usb_is_configured();
        haptic_notify_usb_device_state_change();
#endif

#if ENABLE_PS2_DEVICE
#if ENABLE_FALLBACK_TO_PS2_FROM_USB
        // Wait for USB to enumerate; loop back to try PS/2 on timeout
        {
            uint16_t usb_start = timer_read();
            while (!usb_is_ok()) {
                if (TIMER_DIFF_FAST(timer_read(), usb_start) >= USB_ENUMERATION_TIMEOUT_MS) {
                    break;
                }
            }
        }

        if (!usb_is_ok()) {
            protocol_pre_init();
            continue;
        }
#endif
        break;
    }
#endif
}

void
suspend_power_down_quantum (void) {
    suspend_power_down_kb();

#ifdef BACKLIGHT_ENABLE
    backlight_set(0);
#endif
    led_suspend();
#ifdef LED_MATRIX_ENABLE
    led_matrix_set_suspend_state(true);
#endif
#ifdef RGB_MATRIX_ENABLE
    rgb_matrix_set_suspend_state(true);
#endif
#ifdef HAPTIC_ENABLE
    haptic_notify_usb_device_state_change();
#endif
}

void
suspend_wakeup_init_quantum (void) {
#ifdef BACKLIGHT_ENABLE
    backlight_init();
#endif

    // Restore LED indicators
    led_wakeup();

#ifdef LED_MATRIX_ENABLE
    led_matrix_set_suspend_state(false);
#endif
#ifdef RGB_MATRIX_ENABLE
    rgb_matrix_set_suspend_state(false);
#endif
#ifdef HAPTIC_ENABLE
    haptic_notify_usb_device_state_change();
#endif

    suspend_wakeup_init_kb();
}

static inline void
keyboard_task (void) {
    const uint16_t now = timer_read();
    if (TIMER_DIFF_FAST(now, previous_tick_count) >= 10) {
        previous_tick_count = now;
        keys_tick(now / TICKS_PER_10MS);

#ifdef HAPTIC_ENABLE
        bool configuration = usb_is_configured();
        if (haptic_usb_is_configured != configuration) {
            haptic_usb_is_configured = configuration;
            haptic_notify_usb_device_state_change();
        }
#endif
    }

#if ENABLE_PS2_DEVICE
    ps2_output_task();
#endif

    (void) kbd_input();

#ifdef RGBLIGHT_ENABLE
    rgblight_task();
#endif
#ifdef LED_MATRIX_ENABLE
    led_matrix_task();
#endif
#ifdef RGB_MATRIX_ENABLE
    rgb_matrix_task();
#endif
#if defined(BACKLIGHT_ENABLE) && (defined(BACKLIGHT_PIN) || defined(BACKLIGHT_PINS))
    backlight_task();
#endif
#ifdef ENCODER_ENABLE
    encoder_task();
#endif
#ifdef HAPTIC_ENABLE
    haptic_task();
#endif

    led_task();
#if VIAL_ENABLE
    keys_vial_task();
#endif
}

static inline void
protocol_task (void) {
#if ENABLE_PS2_DEVICE
    if (ps2_output_is_initialized()) {
        return;
    }
#endif
#ifndef NO_USB_STARTUP_CHECK
    if (usb_is_suspended()) {
        while (usb_is_suspended()) {
            suspend_power_down();
            usb_tick();
            if (suspend_wakeup_condition()) {
                if (usb_wake_up_host()) {
                    break;
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
    reset_keys(false);
    keyboard_init();

    for (;;) {
        protocol_task();
        keyboard_task();
    }
}

static void
shutdown_quantum (void) {
    keyboard_reset();
    usb_keyboard_send_report();

#if ENABLE_PS2_DEVICE
    ps2_output_shutdown();
#endif
    usb_deinit();
    delay_milliseconds(32);

#ifdef HAPTIC_ENABLE
    haptic_usb_is_configured = usb_is_configured();
    haptic_notify_usb_device_state_change();

    haptic_shutdown();
#endif
}

static void
keyboard_reset_or_wake (bool is_wake_up) {
    for (int_fast8_t row = 0; row < MATRIX_ROWS; ++row) {
        previous_matrix[row] = 0;
    }
    usb_keyboard_reset();
    reset_keys(is_wake_up);
    delay_milliseconds(32);
}

void
keyboard_reset (void) {
    keyboard_reset_or_wake(false);
}

void
keyboard_wake_up (void) {
    keyboard_reset_or_wake(true);
}

void
jump_to_bootloader (void) {
#if ENABLE_PS2_DEVICE
    // If the PS/2 is active and USB isn't, jumping to the bootloader (which
    // only works over USB) accomplishes nothing useful, and in case the PS/2
    // pins are shorted to the USB D+ and D- pins the bootloader activating
    // USB is not optimal.
    if (ps2_output_is_active() && !usb_is_configured()) {
        return;
    }
#endif
    shutdown_quantum();
    bootloader_jump();
}

void keyboard_clear_settings(void) {
    eeconfig_init();
#if VIAL_ENABLE
    eeconfig_init_via();
#endif
}

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
