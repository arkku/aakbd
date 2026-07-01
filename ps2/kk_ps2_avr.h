/**
 * kk_ps2_avr.h: AVR pin macros and helpers for PS/2.
 *
 * Copyright (c) 2026 Kimmo Kulovesi, https://arkku.dev/
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
#ifndef KK_PS2_AVR_H
#define KK_PS2_AVR_H

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define PASTE_(a, b)            a##b
#define PASTE(a, b)             PASTE_(a, b)

#define PS2_DDR                 PASTE(DDR, PS2_PORT)
#define PS2_PORT_REG            PASTE(PORT, PS2_PORT)
#define PS2_PIN_REG             PASTE(PIN, PS2_PORT)

#define PS2_CLK_BIT             ((uint8_t) (1U << (PS2_CLK_PIN)))
#define PS2_DATA_BIT            ((uint8_t) (1U << (PS2_DATA_PIN)))

#define PS2_BIT_MASK            (PS2_CLK_BIT | PS2_DATA_BIT)

#define PS2_CLK_INT             PASTE(INT, PS2_CLK_INT_NUM)
#define PS2_CLK_INT_VECTOR      PASTE(INT, PASTE(PS2_CLK_INT_NUM, _vect))

#define PS2_CLK_ISC0            PASTE(PASTE(ISC, PS2_CLK_INT_NUM), 0)
#define PS2_CLK_ISC1            PASTE(PASTE(ISC, PS2_CLK_INT_NUM), 1)
#define PS2_CLK_ISC0_BIT        ((uint8_t) (1U << (PS2_CLK_ISC0)))
#define PS2_CLK_ISC1_BIT        ((uint8_t) (1U << (PS2_CLK_ISC1)))

#define ps2_set_pin_state(pin, state) do { \
        if ((state)) { PS2_PORT_REG |= _BV(pin); } \
        else { PS2_PORT_REG &= ~(_BV(pin)); } \
    } while (0)

#define ps2_clk_set(state)          ps2_set_pin_state(PS2_CLK_PIN, (state))
#define ps2_data_set(state)         ps2_set_pin_state(PS2_DATA_PIN, (state))

#define ps2_clk_int_clear_flag()    do { EIFR = _BV(PS2_CLK_INT); } while (0)
#define ps2_clk_int_enable()        do { EIMSK |= _BV(PS2_CLK_INT); } while (0)
#define ps2_clk_int_disable()       do { EIMSK &= ~(_BV(PS2_CLK_INT)); } while (0)
#define ps2_clk_int_on_falling()    do { EICRA = (EICRA & ~PS2_CLK_ISC0_BIT) | PS2_CLK_ISC1_BIT; } while (0)
#define ps2_clk_int_on_change()     do { EICRA = (EICRA & ~PS2_CLK_ISC1_BIT) | PS2_CLK_ISC0_BIT; } while (0)

#define ps2_clk_set_input()         do { PS2_DDR &= ~PS2_CLK_BIT; } while (0)
#define ps2_clk_set_output()        do { PS2_DDR |= PS2_CLK_BIT; } while (0)
#define ps2_data_set_input()        do { PS2_DDR &= ~PS2_DATA_BIT; } while (0)
#define ps2_data_set_output()       do { PS2_DDR |= PS2_DATA_BIT; } while (0)

#define disable_interrupts()        cli()
#define enable_interrupts()         sei()

#define INTERNAL_PULL_UP 1

#define ps2_delay_us(us)            _delay_us(us)

static inline void
ps2_clk_set_low (void) {
    ps2_clk_set(0);
    ps2_clk_set_output();
}

static inline void
ps2_clk_release (void) {
    ps2_clk_set_input();
    ps2_clk_set(INTERNAL_PULL_UP);
}

static inline void
ps2_data_set_low (void) {
    ps2_data_set(0);
    ps2_data_set_output();
}

static inline void
ps2_data_release (void) {
    ps2_data_set_input();
    ps2_data_set(INTERNAL_PULL_UP);
}

static inline void
ps2_data_set_value (const uint8_t high) {
    if (high) {
        ps2_data_release();
    } else {
        ps2_data_set_low();
    }
}

#define are_ps2_lines_high()    ((PS2_PIN_REG & PS2_BIT_MASK) == PS2_BIT_MASK)
#define ps2_data_bit7()         ((uint8_t) (((PS2_PIN_REG & PS2_DATA_BIT) ? 0x80U : 0U)))
#define is_ps2_clk_high()       ((uint8_t) (PS2_PIN_REG & PS2_CLK_BIT) ? 1U : 0U)
#define is_ps2_clk_low()        ((uint8_t) (PS2_PIN_REG & PS2_CLK_BIT) ? 0U : 1U)
#define ps2_data_read()         ((uint8_t) (PS2_PIN_REG & PS2_DATA_BIT) ? 1U : 0U)

#ifdef PS2_STATUS_PIN
#define ps2_status_set_output()         do { PS2_DDR |= (1U << PS2_STATUS_PIN); } while (0)
#define ps2_status_set_input()          do { PS2_DDR &= ~(1U << PS2_STATUS_PIN); } while (0)
#define ps2_status_set(on)              do { \
    if (on) { \
        PS2_PORT_REG |= (1U << PS2_STATUS_PIN); \
    } else { \
        PS2_PORT_REG &= ~(1U << PS2_STATUS_PIN); \
    } \
} while (0)
#endif

#ifdef PS2_ENABLE_PIN
#define ps2_enable_set_input()          do { PS2_DDR &= ~(1U << PS2_ENABLE_PIN); } while (0)
#define ps2_enable_set_pull_up()        do { PS2_PORT_REG |= (1U << PS2_ENABLE_PIN); } while (0)
#define ps2_enable_disable_pull_up()    do { PS2_PORT_REG &= ~(1U << PS2_ENABLE_PIN); } while (0)
#define is_ps2_enable_pin_high()        ((PS2_PIN_REG & (1U << PS2_ENABLE_PIN)) != 0)
#endif

#endif
