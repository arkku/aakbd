/**
 * kk_ps2.c: A PS/2 host library for AVR
 *
 * The PS/2 CLK pin must be connected to a hardware interrupt pin. The
 * default is PD2/INT0.
 *
 * Copyright (c) 2020-2021 Kimmo Kulovesi, https://arkku.dev/
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
#include <stdio.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "kk_ps2.h"

#define PASTE_(a, b)            a##b
#define PASTE(a, b)             PASTE_(a, b)

#ifndef PS2_PORT
#define PS2_PORT                D
#define PS2_DATA_PIN            1
#define PS2_CLK_PIN             0
#define PS2_CLK_INT_NUM         0
#endif

#define PS2_DDR                 PASTE(DDR, PS2_PORT)
#define PS2_PORT_REG            PASTE(PORT, PS2_PORT)
#define PS2_PIN_REG             PASTE(PIN, PS2_PORT)

#define PS2_CLK_BIT             ((uint8_t) (1U << (PS2_CLK_PIN)))
#define PS2_DATA_BIT            ((uint8_t) (1U << (PS2_DATA_PIN)))

#define PS2_CLK_INT             PASTE(INT, PS2_CLK_INT_NUM)
#define PS2_CLK_INT_VECTOR      PASTE(INT, PASTE(PS2_CLK_INT_NUM, _vect))

#define PS2_CLK_ISC0            PASTE(PASTE(ISC, PS2_CLK_INT_NUM), 0)
#define PS2_CLK_ISC1            PASTE(PASTE(ISC, PS2_CLK_INT_NUM), 1)
#define PS2_CLK_ISC0_BIT        ((uint8_t) (1U << (PS2_CLK_ISC0)))
#define PS2_CLK_ISC1_BIT        ((uint8_t) (1U << (PS2_CLK_ISC1)))

#define PS2_BIT_MASK            (PS2_CLK_BIT | PS2_DATA_BIT)
#define are_ps2_lines_high()    ((PS2_PIN_REG & PS2_BIT_MASK) == PS2_BIT_MASK)

#define ps2_data_bit7()         ((uint8_t) (((PS2_PIN_REG & PS2_DATA_BIT) ? 0x80U : 0U)))

#define ps2_clk_state()         ((uint8_t) (PS2_PIN_REG & PS2_CLK_BIT) ? 1U : 0U)

#define ps2_set_pin_state(pin, state) do { \
        if ((state)) { \
            PS2_PORT_REG |= _BV(pin); \
        } else { \
            PS2_PORT_REG &= ~(_BV(pin)); \
        } \
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

#define INTERNAL_PULL_UP 1

enum {
    PS2_STATE_ERROR = 0,
    PS2_STATE_IDLE,
    PS2_STATE_READ_DATA,
    PS2_STATE_READ_PARITY,
    PS2_STATE_READ_STOP,
    PS2_STATE_WRITE_BEGIN,
    PS2_STATE_WRITE_DATA,
    PS2_STATE_WRITE_PARITY,
    PS2_STATE_WRITE_STOP,
    PS2_STATE_WRITE_ACK,
    PS2_STATE_END
};

/// The state of the (bidirectional) PS/2 bus.
static volatile uint8_t ps2_state = PS2_STATE_ERROR;

/// Number of bits left to read/write.
static volatile uint8_t ps2_bits_left = 0;

/// The parity of the current byte being read or written.
static volatile uint8_t ps2_parity = 0;

/// The current data byte being read or written.
static volatile uint8_t ps2_data_byte = 0;

/// The last error that has occurred.
static volatile char ps2_error = 0;

#ifndef KK_PS2_BUFFER_SIZE
#define KK_PS2_BUFFER_SIZE 256
#endif

static uint8_t ps2_buffer[KK_PS2_BUFFER_SIZE];
static volatile uint8_t ps2_buffer_tail = 0;
static volatile uint8_t ps2_buffer_head = 0;

#define buffer_size ((unsigned int) (sizeof ps2_buffer))

#if KK_PS2_BUFFER_SIZE == 256
#define modulo_buffer_size(x) ((uint8_t) (x))
#elif KK_PS2_BUFFER_SIZE > 256
#error KK_PS2_BUFFER_SIZE too large (max. 256)!
#else
#define modulo_buffer_size(x) ((uint8_t) ((x) % buffer_size))
#endif

static inline void
ps2_data_out (const uint8_t state) {
    if (state) {
        ps2_data_set_input();
        ps2_data_set(INTERNAL_PULL_UP);
    } else {
        ps2_data_set_output();
        ps2_data_set(0);
    }
}

static inline void
ps2_clk_set_low (void) {
    ps2_clk_set_output();
    ps2_clk_set(0);
}

static inline void
ps2_clk_release (void) {
    ps2_clk_set_input();
    ps2_clk_set(INTERNAL_PULL_UP);
}

static inline void
ps2_data_release (void) {
    ps2_data_set_input();
    ps2_data_set(INTERNAL_PULL_UP);
}

static inline void
ps2_enable_interrupt (void) {
    ps2_clk_int_clear_flag();
    ps2_clk_int_enable();
}

static inline void
ps2_disable_interrupt (void) {
    ps2_clk_int_disable();
}

static inline void
ps2_set_error (const char err) {
    ps2_state = PS2_STATE_ERROR;
    ps2_error = err;
}

char
ps2_last_error (void) {
    char msg = ps2_error;
    if (msg == 0 && !ps2_is_ok()) {
        msg = '?';
    }
    return msg;
}

static inline bool
ps2_is_active (void) {
    return ps2_state > PS2_STATE_IDLE;
}

bool
ps2_is_ok (void) {
    return ps2_state != PS2_STATE_ERROR;
}

static bool
ps2_write (const uint8_t data, const bool flush_input) {
    unsigned attempts_remaining = 25000U;
    while (ps2_is_active() && attempts_remaining--) {
        _delay_us(4);
    }

    ps2_disable_interrupt();

    if (!ps2_is_ok()) {
        return false;
    }

    if (ps2_is_active()) {
        ps2_error = PS2_ERROR_BUSY;
        return false;
    }

    ps2_data_release();

    // Pull down CLK to request write access
    ps2_clk_set_low();

    // The specified time is 100 µs, but let's err on the side of caution
    // since some KVMs seem to introduce an additional delay
    _delay_us(160);

    ps2_data_out(0);

    ps2_error = 0;
    ps2_state = PS2_STATE_WRITE_BEGIN;
    ps2_data_byte = data;
    ps2_parity = 0;
    ps2_bits_left = 8;

    if (flush_input) {
        ps2_flush_input();
    }

    ps2_clk_int_on_falling();
    ps2_enable_interrupt();

    // Release the CLK to begin writing
    ps2_clk_release();

    // Wait for the write to begin (note that we use a longer timeout
    // here since the very first write on power-up may take a while)
    attempts_remaining = 62500U;
    while (ps2_state == PS2_STATE_WRITE_BEGIN && attempts_remaining--) {
        _delay_us(8);
    }

    if (ps2_state != PS2_STATE_WRITE_BEGIN) {
        return true;
    } else {
        ps2_set_error(PS2_ERROR_WRITE_BEGIN);
        return false;
    }
}

uint8_t
ps2_bytes_available (void) {
    return modulo_buffer_size(buffer_size + ps2_buffer_tail - ps2_buffer_head);
}

int
ps2_get_byte (void) {
    uint8_t data;
    const uint8_t pos = ps2_buffer_head;

    if (pos == ps2_buffer_tail) {
        return EOF;
    }

    data = ps2_buffer[pos];
    ps2_buffer_head = modulo_buffer_size(pos + 1);

    return data;
}

int
ps2_peek_byte (void) {
    const uint8_t pos = ps2_buffer_head;
    return (pos != ps2_buffer_tail) ? ps2_buffer[pos] : EOF;
}

int
ps2_recv (void) {
    while (ps2_is_ok() && !ps2_bytes_available());
    return ps2_get_byte();
}

int
ps2_recv_timeout (const uint8_t milliseconds) {
    if (ps2_bytes_available()) {
        return ps2_get_byte();
    }

    if (milliseconds == 0U) {
        return ps2_recv();
    }

    unsigned attempts_remaining = 100U * (unsigned) milliseconds;
    while (ps2_is_ok() && !ps2_bytes_available() && attempts_remaining--) {
        _delay_us(10);
    }

    return ps2_get_byte();
}

bool
ps2_send (const uint8_t data, const bool flush_input) {
    if (ps2_write(data, flush_input)) {
        while (ps2_is_active());

        return ps2_is_ok();
    } else {
        return false;
    }
}

uint8_t ps2_reply_timeout_ms = 100U;

int
ps2_command (const uint8_t command) {
    int_fast8_t retries_remaining = 2;
    int reply = EOF;

    do {
        if (!ps2_send(command, true)) {
            return reply;
        }
        reply = ps2_recv_timeout(ps2_reply_timeout_ms);
    } while (reply == PS2_REPLY_RESEND && retries_remaining);

    return reply;
}

bool
ps2_command_ack (const uint8_t command) {
    if (ps2_command(command) == PS2_REPLY_ACK) {
        return true;
    } else {
        ps2_error = PS2_ERROR_COMMAND;
        return false;
    }
}

int
ps2_command_arg (const uint8_t command, const uint8_t arg) {
    const int reply = ps2_command(command);
    if (reply != PS2_REPLY_ACK) {
        return reply;
    }
    return ps2_command(arg);
}

bool
ps2_command_arg_ack (const uint8_t command, const uint8_t arg) {
    if (ps2_command_arg(command, arg) == PS2_REPLY_ACK) {
        return true;
    } else {
        ps2_error = PS2_ERROR_COMMAND;
        return false;
    }
}

/// The interrupt fires on the PS/2 CLK pulse.
ISR (PS2_CLK_INT_VECTOR) {
    // Read the incoming bit from the data line at msb
    uint8_t bit = ps2_data_bit7();
    uint8_t state = ps2_state;

    switch (state++) {
    case PS2_STATE_ERROR:
        return;

    case PS2_STATE_END:
        if (bit || ps2_clk_state()) {
            ps2_clk_int_on_falling();
            ps2_state = PS2_STATE_IDLE;
            return;
        }
        // fallthrough
    case PS2_STATE_IDLE:
        if (bit == 0) {
            ps2_state = state;
            ps2_data_byte = 0;
            ps2_parity = 0;
            ps2_bits_left = 8;
        } else {
            ps2_set_error(PS2_ERROR_START_BIT);
        }
        break;

    case PS2_STATE_READ_DATA:
        ps2_parity ^= bit;

        ps2_data_byte >>= 1;
        ps2_data_byte |= bit;

        if (--ps2_bits_left == 0) {
            ps2_state = state;
        }
        break;

    case PS2_STATE_READ_PARITY:
        ps2_parity ^= bit;

        if (ps2_parity) {
            ps2_state = state;
        } else {
            ps2_set_error(PS2_ERROR_PARITY);
        }
        break;

    case PS2_STATE_READ_STOP:
        ps2_state = PS2_STATE_END;
        ps2_clk_int_on_change();

        if (bit) {
            const uint8_t next_pos = modulo_buffer_size(ps2_buffer_tail + 1);
            if (next_pos != ps2_buffer_head) {
                ps2_buffer[ps2_buffer_tail] = ps2_data_byte;
                ps2_buffer_tail = next_pos;
            }
        } else {
            ps2_set_error(PS2_ERROR_STOP_BIT);
        }
        break;

    case PS2_STATE_WRITE_BEGIN:
        ps2_state = state;
        // fallthrough

    case PS2_STATE_WRITE_DATA:
        bit = ps2_data_byte & 1;
        ps2_data_out(bit);

        ps2_parity ^= bit;
        ps2_data_byte >>= 1;

        if (--ps2_bits_left == 0) {
            ps2_state = state;
        }
        break;

    case PS2_STATE_WRITE_PARITY:
        ps2_data_out(ps2_parity ^ 1);
        ps2_state = state;
        break;

    case PS2_STATE_WRITE_STOP:
        ps2_data_release();
        ps2_state = state;
        break;

    case PS2_STATE_WRITE_ACK:
        if (bit == 0) {
            ps2_state = PS2_STATE_END;
            ps2_clk_int_on_change();
        } else {
            ps2_set_error(PS2_ERROR_WRITE_END);
        }
        break;
    }
}

void
ps2_flush_input (void) {
    ps2_buffer_tail = 0;
    ps2_buffer_head = 0;
}

void
ps2_enable (void) {
    int attempts_remaining = 12000;
    while (ps2_is_active() && attempts_remaining--) {
        _delay_us(4);
    }

    // Note: The timeout is to avoid interrupting an in-progress byte, but
    // since we cannot recognize a broken state, we must proceed regardless

    ps2_disable_interrupt();

    ps2_data_release();
    ps2_clk_set_low();

    ps2_flush_input();

    ps2_error = 0;
    ps2_state = PS2_STATE_IDLE;

    ps2_clk_int_on_falling();
    ps2_enable_interrupt();

    ps2_clk_release();
}
