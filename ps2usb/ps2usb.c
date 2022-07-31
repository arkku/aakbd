/**
 * ps2usb.c: A PS/2 to USB keyboard converter for ATMEGA32U4.
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

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include <usb_hardware.h>
#include <usbkbd.h>
#include <aakbd.h>
#include <avrtimer.h>
#include <keys.h>

#include "led.h"
#include "kk_ps2.h"
#include "ps2usb_keys.h"
#include "progmem.h"

#ifndef MAX_ERROR_COUNT
/// The maximum number of PS/2 protocol errors before we will try to reset the
/// device. Repeated errors may indicate that we are out of sync.
#define MAX_ERROR_COUNT 8
#endif

static bool is_kbd_ready = false;
static volatile uint8_t kbd_error_count = 0;
static bool error_handled = false;
static volatile uint8_t kbd_idle_10ms_count = 0;
static uint8_t kbd_key_state;
static uint8_t kbd_led_state = 0U;

#define KEY_FLAG_IS_RELEASE     ((uint8_t) (1U << 0))
#define KEY_FLAG_IS_EXTENDED    ((uint8_t) (1U << 1))
#define KEY_FLAG_OVERFLOW       ((uint8_t) (1U << 2))
#define KEY_FLAG_ERROR          ((uint8_t) (1U << 3))
#define KEY_FLAG_INVALID_STATE  ((uint8_t) (1U << 4))

/// The keycode prefix before a break (release) event.
#define KBD_BREAK_PREFIX        ((uint8_t) 0xF0U)

/// The keycode prefix before an extended keycode.
/// Normally this should not happen in scan code set 3, but some manufacturers
/// have special keys that still send this, so the parser should be able to
/// skip them.
#define KBD_EXTENDED_PREFIX     ((uint8_t) 0xE0U)

#ifdef JDT
#define disable_jtag()          (MCUCR |= (1 << JTD))
#else
#define disable_jtag()          do { } while (0)
#endif
#define cpu_clear_prescaler()   do { CLKPR = (1 << CLKPCE); CLKPR = 0; } while (0)

/// Maximum number of 10 ms ticks (i.e., hundreths of a second) the keyboard
/// can idle before we try to ping it.
#define MAX_IDLE_10MS       250U

#define TICKS_PER_SECOND    (F_CPU / 1024UL)
#define TICKS_PER_10MS      ((TICKS_PER_SECOND / 100UL) + (((TICKS_PER_SECOND % 100UL) >= 50UL) ? 1UL : 0UL))

static volatile uint8_t tick_10ms_count = 0;
static volatile uint8_t tick_320ms_count = 0;
static uint8_t previous_tick;

uint8_t
current_10ms_tick_count (void) {
    return tick_10ms_count;
}

// A state that changes over time, can be used to blink LEDs.
#define blink_state             (tick_320ms_count & 1)

/// Is the 10 ms tick due at `count`?
#define tick_is_due_at(count)   ((previous_tick - (count)) & 0x80U)

/// A less frequent blink state, used for the suspend blink.
#define suspend_blink_state     ((tick_320ms_count % 16) == 0)

#define BOOT_KEY_SKIP_BOOTLOADER  0
#define BOOT_KEY_GO_TO_BOOTLOADER 0x7777U

volatile uint16_t * const boot_key_pointer = (volatile uint16_t *) 0x0800U;

// This fires approx. once per 10 ms, i.e., 100 times per second
ISR (TIMER_COMPA_VECTOR) {
    if ((++tick_10ms_count % 32) == 0) {
        ++tick_320ms_count;
    }
    if (kbd_idle_10ms_count != 255U) {
        ++kbd_idle_10ms_count;
    }
}

inline static void
kbd_reset_key_state (void) {
    kbd_key_state = 0U;
}

static void
kbd_idle_reset (void) {
    timer_reset_counter();
    kbd_idle_10ms_count = 0;
}

static void
kbd_idle_start_counter (void) {
    timer_disable();

    timer_set_prescaler_1024();
    kbd_idle_reset();

    TIMER_OCRA = TICKS_PER_10MS;
    timer_set_ctc_mode();

    timer_enable_compa();
}

static int
kbd_recv_byte (void) {
    int attempts_remaining = 100;
    int byte;
    do {
        wdt_reset();
        byte = ps2_recv_timeout(10);
    } while (byte == EOF && attempts_remaining--);

    return byte;
}

static uint8_t
kbd_set_leds (uint8_t new_state) {
    if (kbd_led_state != new_state) {
        if (ps2_command_arg_ack(PS2_COMMAND_SET_LEDS, new_state)) {
            kbd_led_state = new_state;
        }
    }
    return kbd_led_state;
}

static uint8_t
key_flag (const uint8_t byte) {
    switch (byte) {
    case KBD_BREAK_PREFIX:      return KEY_FLAG_IS_RELEASE;
    case KBD_EXTENDED_PREFIX:   return KEY_FLAG_IS_EXTENDED;
    case 0x00:                  return KEY_FLAG_OVERFLOW;
    case PS2_COMMAND_RESET:     return KEY_FLAG_ERROR;
    case PS2_REPLY_TEST_PASSED: // fallthrough
    case PS2_REPLY_RESEND:      // fallthrough
    case PS2_COMMAND_ECHO:      return KEY_FLAG_INVALID_STATE;
    default:                    return 0U;
    }
}

static bool
kbd_input (void) {
    bool have_changes = false;

    wdt_reset();

    while (ps2_bytes_available() && ps2_is_ok()) {
        uint8_t key = ps2_get_byte();
        const uint8_t prefix_flag = key_flag(key);
        if (prefix_flag) {
            // The byte was a prefix modifying the next scancode
            kbd_key_state |= prefix_flag;
            continue;
        }

        // This is the actual keycode, modified by kbd_key_state

        const bool is_key_release = (kbd_key_state & KEY_FLAG_IS_RELEASE) != 0;
        const bool is_extended = (kbd_key_state & KEY_FLAG_IS_EXTENDED) != 0;
        kbd_key_state = 0U;

        if (is_extended) {
            // Ignore extended keycodes, we should not get them in set 3
            continue;
        }

        have_changes = true;
        process_key(usb_keycode_for_ps2_keycode(key), is_key_release);
    }

    if (kbd_key_state & KEY_FLAG_OVERFLOW) {
        report_keyboard_error(true);
    } else if (kbd_key_state & KEY_FLAG_ERROR) {
        report_keyboard_error(false);
    }

    if (have_changes) {
        kbd_idle_reset();
    } else if (kbd_key_state & KEY_FLAG_INVALID_STATE) {
        ++kbd_error_count;
    }

    return have_changes;
}

static bool
kbd_configure (void) {
    int_fast8_t attempts_remaining = 3;

    do {
        if (ps2_command_arg_ack(PS2_COMMAND_SET_SCAN_CODES, 3U)
            && ps2_command_ack(PS2_COMMAND_ENABLE)
            && ps2_command_ack(PS2_COMMAND_SET_ALL_KEYS_MAKE_BREAK)
        ) {
            is_kbd_ready = true;
            return true;
        } else if (!ps2_is_ok()) {
            ps2_enable();
        }
    } while (attempts_remaining--);

    return false;
}

static bool
kbd_init (const bool do_reset) {
    error_handled = false;

    int byte;

    if (do_reset) {
        is_kbd_ready = false;

        // Allow a longer timeout for reset
        ps2_send_byte(PS2_COMMAND_RESET);
        byte = kbd_recv_byte();

        if (byte != PS2_REPLY_ACK) {
            if (byte == PS2_REPLY_TEST_PASSED) {
                // Maybe the device actually reset just now, let's see
                byte = ps2_recv_timeout(100);
                if (byte == 0x00U) {
                    // Yes, it's probably only just powering up, let's retry
                    return kbd_init(true);
                }
            }
            return false;
        }

        byte = kbd_recv_byte();

        if (byte != PS2_REPLY_TEST_PASSED) {
            return false;
        }
    }

    wdt_reset();

    reset_keys();

    if (kbd_configure()) {
        kbd_reset_key_state();
        kbd_error_count = 0;
        kbd_idle_start_counter();
        (void) kbd_set_leds(PS2_LED_NUM_LOCK_BIT | PS2_LED_CAPS_LOCK_BIT | PS2_LED_SCROLL_LOCK_BIT);
        return true;
    } else {
        return false;
    }
}

static void
setup (const bool is_power_up) {
    int byte;
    int_fast8_t attempts_remaining;

    // If the watchdog resets us during setup, go to bootloader since it
    // probably means the firmware (or hardware) is broken
    if (is_power_up) {
        *boot_key_pointer = BOOT_KEY_GO_TO_BOOTLOADER;
    }

    // Use the watchdog timer to recover from error states
    wdt_reset();
    wdt_enable(WDTO_4S);

    // Disable interrupts during setup
    cli();

    led_set_output();
    led_set(1);
    error_led_set(1);
    previous_tick = tick_10ms_count - 1;

    // Set up the PS/2 port
    ps2_enable();

    if (is_power_up) {
        // Enable pull-ups
        PORTB |= ~DDRB;
        PORTC |= ~DDRC;
        PORTD |= ~(DDRD | (1 << 2)); // Except RXD
#ifdef PORTE
        PORTE |= ~DDRE;
#endif
#ifdef PORTF
        PORTF |= ~DDRF;
#endif

        // Power reduction
#ifdef ADCSRA
        ADCSRA &= ~(1U << ADEN); // ADC
#endif
#ifdef DIDR0
        DIDR0 |= 0xF3U; // ADC
#endif
        DIDR1 |= 1U; // AIN0D
#ifdef DIDR2
        DIDR2 |= 0x3FU; // ADC
        power_adc_disable();
#endif
        power_spi_disable();
        power_usart1_disable();
        power_timer1_disable();
#ifdef TIMSK3
        power_timer2_disable();
#endif
#ifdef TIMSK4
        power_timer3_disable();
#endif
#ifdef TWIE
        power_twi_disable();
#endif

        usb_init();
        wdt_reset();
    }

    timer_disable();

    // Enable interrupts
    sei();

    if (is_power_up) {
        // Give the keyboard some time to start up
        _delay_ms(100);

        // Read any bytes sent by the device on power-up
        byte = kbd_recv_byte();

        led_set(0);

        if (byte == PS2_REPLY_TEST_PASSED) {
            // Try to initialize directly from power-up
            (void) kbd_init(false);
        }
    }

    // Attempt to initialize the device
    attempts_remaining = 10;
    while (!is_kbd_ready && !kbd_init(true) && attempts_remaining--) {
        led_toggle();

        if (!ps2_is_ok()) {
            ps2_enable();
        }
        _delay_ms(200);
        wdt_reset();
    }

    if (ps2_is_ok()) {
        error_led_set(0);
    }

    // Resets after this point should skip the bootloader
    if (is_power_up && *boot_key_pointer == BOOT_KEY_GO_TO_BOOTLOADER) {
        *boot_key_pointer = BOOT_KEY_SKIP_BOOTLOADER;
    }
}

static inline void
update_keyboard_leds (const uint8_t usb_state) {
    if (usb_is_configured()) {
        if (usb_is_suspended()) {
#if SCROLL_LOCK_LED_ON_SUSPEND != 0
            (void) kbd_set_leds(PS2_LED_SCROLL_LOCK_BIT * suspend_blink_state);
#else
            (void) kbd_set_leds(0);
#endif
        } else {
            (void) kbd_set_leds(
                ((usb_state & LED_NUM_LOCK_BIT)      ? PS2_LED_NUM_LOCK_BIT      : 0)    |
                ((usb_state & LED_CAPS_LOCK_BIT)     ? PS2_LED_CAPS_LOCK_BIT     : 0)    |
                (((usb_state & LED_SCROLL_LOCK_BIT) != 0)
#if SCROLL_LOCK_LED_ON_OVERFLOW != 0
                  != (keys_error() && blink_state)
#endif
                                                     ? PS2_LED_SCROLL_LOCK_BIT   : 0)
            );
        }
    }
}

int
main (void) {
    uint8_t byte;

    cpu_clear_prescaler();
    disable_jtag();

    setup(true);

    for (;;)
    {
        byte = tick_10ms_count;
        if (tick_is_due_at(byte)) {
            keys_tick(byte);
            previous_tick = byte;
        }

        if (!ps2_is_ok()) {
            if (error_handled) {
                // Watchdog will cause a reset later
                led_set(blink_state);
                continue;
            }

            byte = ps2_last_error();
            error_handled = true;
            if (byte == PS2_ERROR_PARITY && kbd_error_count <= MAX_ERROR_COUNT) {
                // Try to request a resend after parity error
                ++kbd_error_count;
                ps2_enable();
                ps2_request_resend();
            } else {
                error_led_set(1);
                continue;
            }
        }

        usb_tick();
        if (usb_is_ok()) {
            error_led_set(0);
        } else {
            error_led_set(blink_state);
        }

        if (!is_kbd_ready) {
            continue;
        }

        led_set(0);

        while (kbd_input()) {
            led_toggle();
        }

        update_keyboard_leds(keys_led_state());

        if (kbd_error_count > MAX_ERROR_COUNT) {
            // Try to re-synchronize if we are getting errors
            kbd_init(true);
        }

        if (kbd_idle_10ms_count > MAX_IDLE_10MS) {
            // Ping the keyboard when idle to detect unplugging
            if (ps2_command(PS2_COMMAND_ECHO) != PS2_COMMAND_ECHO) {
                kbd_error_count = MAX_ERROR_COUNT + 1;
            }
            kbd_idle_reset();
        }
    }
}

void
keyboard_reset (void) {
    if (!kbd_init(true)) {
        (void) kbd_set_leds(PS2_LED_NUM_LOCK_BIT | PS2_LED_SCROLL_LOCK_BIT);
    }
    _delay_ms(32);
}

void
jump_to_bootloader (void) {
    // Tear down USB
    usb_deinit();

    // Signal to the bootloader that we want to enable it
    *boot_key_pointer = BOOT_KEY_GO_TO_BOOTLOADER;

    if (ps2_is_ok()) {
        // Indicate bootloader state with scroll lock LED
        (void) kbd_set_leds(PS2_LED_NUM_LOCK_BIT | PS2_LED_SCROLL_LOCK_BIT);
        _delay_ms(32);
        (void) kbd_set_leds(PS2_LED_SCROLL_LOCK_BIT);
        _delay_ms(32);
    }

    cli();

    // Set again in case this changed in some interrupt handler
    *boot_key_pointer = BOOT_KEY_GO_TO_BOOTLOADER;

    // Reset the device state
    ACSR = 0;
#ifdef ADCSRA
    ADCSRA = 0;
#endif
    EECR = 0;
#ifdef EIMSK
    EIMSK = 0;
#endif
#ifdef PCICR
    PCICR = 0;
#endif
    SPCR = 0;
    TIMSK0 = 0;
    TIMSK1 = 0;
#ifdef TIMSK3
    TIMSK3 = 0;
#endif
#ifdef TIMSK4
    TIMSK4 = 0;
#endif
#ifdef TWCR
    TWCR = 0;
#endif
    UCSR1B = 0;
    DDRB = 0;
    DDRC = 0;
    DDRD = 0;
#ifdef PORTE
    DDRE = 0;
#endif
#ifdef PORTF
    DDRF = 0;
#endif
    PORTB = 0;
    PORTC = 0;
    PORTD = 0;
#ifdef PORTE
    PORTE = 0;
#endif
#ifdef PORTF
    PORTF = 0;
#endif

    // Try to let the watchdog reset
    wdt_enable(WDTO_15MS);
    _delay_ms(32);

    // We shouldn't get here, but just in case:
    __asm volatile("jmp 0x7E00");
}

#include "generic_hid.h"
#if ENABLE_GENERIC_HID_ENDPOINT

enum generic_request {
    NONE,
    RESET_KEYBOARD,
    JUMP_TO_BOOTLOADER,
};

uint8_t
handle_generic_hid_report (uint8_t report_id, uint8_t count, uint8_t report[static count], uint8_t response_length[static 1], uint8_t response[static *response_length]) {
    if (count == 0 || GENERIC_HID_FEATURE_SIZE == 0) {
        return RESPONSE_OK;
    }

    enum generic_request request = report[0];

    switch (request) {
    case NONE:
        return RESPONSE_OK;
    case RESET_KEYBOARD:
        kbd_error_count = 0xFF;
        return RESPONSE_OK;
    case JUMP_TO_BOOTLOADER:
        return JUMP_TO_BOOTLOADER;
    default:
        return RESPONSE_ERROR;
    }

    *response_length = 0;
}

#if GENERIC_HID_REPORT_SIZE != 0 && GENERIC_HID_REPORT_SIZE != 8
#error "GENERIC_HID_REPORT_SIZE should be 8 or 0."
#endif

bool
make_generic_hid_report (uint8_t report_id, uint8_t count, uint8_t report[static count]) {
#if GENERIC_HID_REPORT_SIZE
    if (count < 8) {
        return false;
    }
    report[0] = usb_last_error();
    report[1] = ps2_last_error();
    report[2] = kbd_error_count;
    report[3] = keys_error();
    report[4] = usb_is_in_boot_protocol();
    report[5] = kbd_key_state;
    report[6] = usb_address();
    report[7] = kbd_idle_10ms_count;
#endif
    return true;
}
#endif
