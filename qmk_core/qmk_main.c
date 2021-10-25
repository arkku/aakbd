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

#ifndef F_CPU
/// Crystal frequency.
#define F_CPU   16000000UL
#endif

#include <stdint.h>
#include <stdbool.h>

#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

#define INCLUDE_USB_HARDWARE_ACCESS
#include <usbkbd.h>
#include <keys.h>
#include <main.h>

#include "timer.h"

#define TIMER_NUM 1
#include <avrtimer.h>

#include "quantum.h"
#include "matrix.h"
#include "keymap.h"
#include "bootloader.h"
#include "led.h"

#define BOOT_KEY_SKIP_BOOTLOADER  0
#define BOOT_KEY_GO_TO_BOOTLOADER 0x7777U

#ifdef JDT
#define disable_jtag()          (MCUCR |= (1 << JTD))
#else
#define disable_jtag()          do { } while (0)
#endif
#define cpu_clear_prescaler()   do { CLKPR = (1 << CLKPCE); CLKPR = 0; } while (0)

#define STRIFY(a)           #a
#define STR(a)              STRIFY(a)

#ifdef XWHATSIT
const char KEYBOARD_FILENAME[] = STR(KEYBOARD_NAME)".c";
#endif

#define TICKS_PER_SECOND    (F_CPU / 1024UL)
#define TICKS_PER_10MS      ((TICKS_PER_SECOND / 100UL) + (((TICKS_PER_SECOND % 100UL) >= 50UL) ? 1UL : 0UL)

static volatile uint8_t tick_10ms_count = 0;
static volatile uint8_t tick_320ms_count = 0;
static uint8_t previous_tick;

uint8_t
current_10ms_tick_count (void) {
    return tick_10ms_count;
}

// A state that changes over time, can be used to blink LEDs.
// This fires approx. once per 10 ms, i.e., 100 times per second
ISR (TIMER_COMPA_VECTOR) {
    if ((++tick_10ms_count % 32) == 0) {
        ++tick_320ms_count;
    }
}

#define blink_state             (tick_320ms_count & 1)

/// A less frequent blink state, used for the suspend blink.
#define suspend_blink_state     ((tick_320ms_count % 16) == 0)

#define tick_is_due_at(count)   ((previous_tick - (count)) & 0x80U)

volatile uint16_t * const boot_key_pointer = (volatile uint16_t *) 0x0800U;

static uint8_t kbd_led_state = 0;

static inline uint8_t
kbd_set_leds (uint8_t new_state) {
    if (kbd_led_state != new_state) {
        kbd_led_state = new_state;
        led_set(new_state);
    }
    return kbd_led_state;
}

void
suspend_wakeup_init_user (void) {
    wdt_reset();
    wdt_enable(WDTO_4S);
}

static bool
kbd_input (void) {
    static matrix_row_t previous_matrix[MATRIX_ROWS];
    matrix_row_t matrix_change = 0;

    wdt_reset();

    bool have_changes = matrix_scan();

    for (int_fast8_t row = 0; row < MATRIX_ROWS; ++row) {
        matrix_row_t matrix_row = matrix_get_row(row);
        matrix_change = matrix_row ^ previous_matrix[row];
        if (matrix_change) {
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

static bool
kbd_init (void) {
    wdt_reset();
    matrix_init();
    return true;
}

void
keyboard_reset (void) {
    if (!kbd_init()) {
        (void) kbd_set_leds(LED_NUM_LOCK_BIT | LED_SCROLL_LOCK_BIT);
    }
    _delay_ms(32);
}

static void
setup (const bool is_power_up) {
    // If the watchdog resets us during setup, go to bootloader since it
    // probably means the firmware (or hardware) is broken
    *boot_key_pointer = BOOT_KEY_GO_TO_BOOTLOADER;

    // Use the watchdog timer to recover from error states
    wdt_reset();
    wdt_enable(WDTO_4S);

    led_init_ports();

    if (is_power_up) {
        // Disable interrupts during setup
        cli();

        usb_init();
        wdt_reset();
        previous_tick = tick_10ms_count - 1;

        // Enable interrupts
        sei();

        matrix_setup();
    }

    if (kbd_init()) {
        // Resets after this point should skip the bootloader
        if (is_power_up && *boot_key_pointer == BOOT_KEY_GO_TO_BOOTLOADER) {
            *boot_key_pointer = BOOT_KEY_SKIP_BOOTLOADER;
        }
    }
}

static inline void
update_keyboard_leds (const uint8_t usb_state) {
    if (usb_is_configured()) {
        if (usb_is_suspended()) {
            (void) kbd_set_leds(0);
        } else {
            (void) kbd_set_leds(usb_state |
#if SCROLL_LOCK_LED_ON_OVERFLOW
                ((keys_error() && blink_state) ? LED_SCROLL_LOCK_BIT : 0)
#endif
            );
        }
    }
}

int
main (void) {
    cpu_clear_prescaler();
    disable_jtag();

    setup(true);

    for (;;) {
        const uint8_t now = tick_10ms_count;
        if (tick_is_due_at(now)) {
            keys_tick(now);
            previous_tick = now;
        }

        (void) kbd_input();
        update_keyboard_leds(keys_led_state());
        wdt_reset();

        usb_tick();
    }
}

#ifdef ENABLE_I2C
#include "i2c_master.h"
#endif

void
jump_to_bootloader (void) {
    // Tear down USB
    usb_deinit();

#ifdef ENABLE_I2C
    // Also stop I2C if we are using it
    i2c_stop();
#endif

    _delay_ms(32);

    wdt_disable();

    cli();

    timer_disable();

    *boot_key_pointer = BOOT_KEY_GO_TO_BOOTLOADER;
    bootloader_jump();
}

void
matrix_setup() {
    // Need timer for debounce
    timer_init();
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

#define EECONFIG_KEYBOARD ((uint32_t *) 15)

uint32_t
eeconfig_read_kb(void) {
    return eeprom_read_dword(EECONFIG_KEYBOARD);
}

void
eeconfig_update_kb (uint32_t val) {
    eeprom_update_dword(EECONFIG_KEYBOARD, val);
}

uint8_t
usb_keycode_for_matrix (const int8_t row, const int8_t column) {
    if (row < MATRIX_ROWS && column < MATRIX_COLS) {
        return pgm_read_byte(&keymaps[0][row][column]);
    } else {
        return KC_NO;
    }
}
