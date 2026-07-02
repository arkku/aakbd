/**
 * kk_ps2_device.c: PS/2 device-mode (delay-based) implementation.
 *
 * Copyright (c) 2026 Kimmo Kulovesi, https://arkku.dev/
 *
 * This is the low level PS/2 device mode implementation. It is surprisingly
 * different from the PS/2 host mode implementation that I did earlier, mainly
 * in that it isn't interrupt-driven. On the PS/2 bus it is the device that
 * controls the clock, so apart from the actual transfer of bytes (where we
 * disable interrupts), we have quite a lot of leeway in the timing.
 *
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
#ifndef ENABLE_PS2_DEVICE
#define ENABLE_PS2_DEVICE 1
#endif

#include "kk_ps2_device.h"
#include "kk_ps2_avr.h"
#include "usbkbd_config.h"

#include <stdio.h>

// MARK:  Configuration

#ifndef KK_PS2_BUFFER_SIZE
#define KK_PS2_BUFFER_SIZE 16
#endif

#if KK_PS2_BUFFER_SIZE > 256
#error "KK_PS2_BUFFER_SIZE must be <= 256"
#endif
#if (256 % KK_PS2_BUFFER_SIZE) != 0
#error "KK_PS2_BUFFER_SIZE must be a power of 2"
#endif

/// Data setup time (microseconds) before the clock pulse on which it is read.
#define DATA_SETUP_US 15

/// The clock pulse duration (clock low time).
#define CLOCK_PULSE_US 39

/// Empirically determined extra processing time after clock pulse, this can
/// be subtracted from the delay to make it more symmetric (the clock low
/// period has no processing time).
#define PROCESSING_AFTER_PULSE_US 2

/// The time the bus needs to be idle before we can write.
#define PS2_BUS_IDLE_TIME_US ((uint8_t) 50U)

/// The time to wait for `PS2_BUS_IDLE_TIME_US` until giving up.
#define PS2_BUS_READY_TIMEOUT_US ((uint8_t) 200U)

// MARK: - State

/// The output buffer. It is intended to be small and flushed frequently:
/// it's better to enqueue higher-level events (like keystrokes), because
/// if it is cancelled in the middle of a multi-byte sequence, you have to
/// re-send the entire thing or a key release may turn into a key press.
static uint8_t ps2_buffer[KK_PS2_BUFFER_SIZE];

/// Output ring buffer head.
static uint8_t ps2_buffer_head = 0;

/// Output ring buffer tail.
static uint8_t ps2_buffer_tail = 0;

/// The number of elements in `ps2_buffer`.
#define ps2_buffer_count    ((uint8_t) (ps2_buffer_head - ps2_buffer_tail))

/// Is the ps2_buffer empty?
#define is_ps2_buffer_empty (ps2_buffer_count == 0)

/// Is the ps2_buffer full?
#define is_ps2_buffer_full  (ps2_buffer_count == KK_PS2_BUFFER_SIZE)

#if KK_PS2_BUFFER_SIZE == 256
#define modulo_buffer_size(x) ((uint8_t) (x))
#else
#define modulo_buffer_size(x) ((uint8_t) ((x) % (sizeof ps2_buffer)))
#endif

/// Global error flag for debug output. Note that this isn't cleared
/// automatically.
static char ps2_device_error = 0;

// MARK: - Lifecycle

void
ps2_device_init (void) {
#ifdef PS2_ENABLE_PIN
    ps2_enable_set_input();
    ps2_enable_set_pull_up();
#endif

#ifdef PS2_STATUS_PIN
    ps2_status_set_output();
    ps2_status_set(1);
#endif

#ifdef PS2_ENABLE_PIN
    // No need to check other pins if the force enable is active
    if (!is_ps2_enable_pin_high())
#endif
    {
        // If the PS/2 lines are floating (not the same as the USB lines, for
        // example), stray capacitance can lead to a misdetection of PS/2
        // drive the pins low for one microsecond to discharge. On the other hand,
        // if the lines are the same as the USB lines, we don't need to do this
        // because they are connected to the USB bus and pulled down -> no PS/2.

        ps2_data_set_output();
        ps2_data_set_low();
        _delay_us(1);
        ps2_data_set_input();

        ps2_clk_set_output();
        ps2_clk_set_low();
        _delay_us(1);
    }

    // PS/2 data lines are inputs, with the pull up on the host side

    ps2_clk_set_input();
    ps2_clk_set(0);
    ps2_data_set_input();
    ps2_data_set(0);

    _delay_us(1);

#ifdef PS2_STATUS_PIN
    ps2_status_set(0);
#endif

    ps2_device_error = 0;
}

bool
ps2_device_host_detected (void) {
#ifdef PS2_ENABLE_PIN
    if (!is_ps2_enable_pin_high()) {
        // The enable pin is active low: force PS/2 mode
        return true;
    }
#endif
    return are_ps2_lines_high();
}

void
ps2_device_attach (void) {
    ps2_clk_release();
    ps2_data_release();
#ifdef PS2_STATUS_PIN
#error
    ps2_status_set_output();
    ps2_status_set(1);
#endif
}

void
ps2_device_shutdown (void) {
    ps2_data_set(0);
    ps2_data_set_input();
    ps2_clk_set(0);
    ps2_clk_set_input();
#ifdef PS2_STATUS_PIN
    ps2_status_set(0);
    ps2_status_set_input();
#endif
#ifdef PS2_ENABLE_PIN
    ps2_enable_set_input();
    ps2_enable_disable_pull_up();
#endif
}

// MARK: - Clock

/// Pulse the clock low for `CLOCK_PULSE_US` µs. Does not delay after!
static inline void
ps2_clock_pulse (void) {
    ps2_clk_set_low();
    ps2_delay_us(CLOCK_PULSE_US);
    ps2_clk_release();
}

/// Pulse the clock for transmission, delay after, then check for inhibit.
/// The delay after the pulse assumes a `DATA_SETUP_US` delay before the
/// next pulse.
/// - Note: Do not ignore the return value! See `ps2_clock_pulse()`.
/// - Returns: Whether the clock is high (if low, host is inhibiting).
static bool
ps2_clock_pulse_tx (void) {
    ps2_clock_pulse();
    ps2_delay_us(CLOCK_PULSE_US - DATA_SETUP_US - PROCESSING_AFTER_PULSE_US);
    if (!is_ps2_clk_high()) {
        ps2_device_error = PS2_ERROR_BUSY;
        return false;
    }
    return true;
}

/// Wait for the bus to be idle at least 50 µs straight (no host inhibit).
/// - Precondition: timeout_us must be >= 50
/// - Returns: `true` if the bus is idle, `false` on timeout.
static inline bool
ps2_wait_idle (uint8_t timeout_us) {
    uint8_t idle_us = 0;

    while (timeout_us--) {
        if (are_ps2_lines_high()) {
            if (++idle_us >= PS2_BUS_IDLE_TIME_US) {
                return are_ps2_lines_high();
            }
        } else {
            idle_us = 0;
            if (timeout_us < PS2_BUS_IDLE_TIME_US) {
                // It can't be idle long enough, might as well give up
                return false;
            }
        }
        ps2_delay_us(1);
    }
    return false;
}

// MARK: - Transmit

/// Send one bit out (setup data, wait, pulse clock).
/// Returns `false` from _caller_ (because this is a macro) on clock inhibit.
#define PS2_DEVICE_TX_BIT(bit)       \
    do {                             \
        ps2_data_set_value((bit));   \
        ps2_delay_us(DATA_SETUP_US); \
        if (!ps2_clock_pulse_tx()) { \
            ps2_data_release();      \
            return false;            \
        }                            \
    } while (0)

static bool
ps2_device_tx_byte_atomic (const uint8_t data) {
    // Start bit
    PS2_DEVICE_TX_BIT(0);

    // Data bits (least significant first)
    uint8_t parity = 0;
    uint8_t byte = data;
    for (uint8_t i = 8; i; --i) {
        PS2_DEVICE_TX_BIT(byte & 1);
        parity ^= (byte & 1);
        byte >>= 1;
    }

    // Parity bit (odd)
    PS2_DEVICE_TX_BIT(parity ^ 1);

    // Stop bit (1)
    ps2_data_release();
    ps2_delay_us(DATA_SETUP_US);
    (void) ps2_clock_pulse_tx();
    ps2_delay_us(DATA_SETUP_US);

    return true;
}

/// Send one byte out if the bus is idle.
/// - Returns: `false` on host inhibit, `true` if the data was sent.
static bool
ps2_device_tx_byte (const uint8_t data) {
    if (!ps2_wait_idle(PS2_BUS_READY_TIMEOUT_US)) {
        return false;
    }

    disable_interrupts();
    if (!are_ps2_lines_high()) {
        // Host inhibiting, maybe there was an interrupt after idle wait
        return false;
    }
    const bool is_success = ps2_device_tx_byte_atomic(data);
    enable_interrupts();

    return is_success;
}

void
ps2_device_resend (void) {
    uint8_t last_pos = modulo_buffer_size(ps2_buffer_head - 1);
    (void) ps2_device_tx_byte(ps2_buffer[last_pos]);
}

// MARK: - Receive

static inline int
ps2_device_recv_atomic (void) {
    uint8_t data = 0;
    uint8_t parity = 0;

    // Start bit
    ps2_delay_us(DATA_SETUP_US);
    if (ps2_data_read()) {
        // Data is high - the host was supposed to pull it low
        ps2_device_error = PS2_ERROR_START_BIT;
        return EOF;
    }

    for (uint8_t bit = 0; bit < 10; ++bit) {
        ps2_clock_pulse();

        if (bit < 8) {
            // Data
            data >>= 1;
            if (ps2_data_read()) {
                data |= 0x80;
                parity ^= 1;
            }
            ps2_delay_us(DATA_SETUP_US);
        } else if (bit == 8) {
            // Parity
            if (ps2_data_read()) {
                parity ^= 1;
            }
            ps2_delay_us(DATA_SETUP_US);
        } else {
            // Stop bit
            if (!ps2_data_read()) {
                ps2_device_error = PS2_ERROR_STOP_BIT;
                ps2_delay_us(CLOCK_PULSE_US - PROCESSING_AFTER_PULSE_US);
                return EOF;
            }
        }

        ps2_delay_us(CLOCK_PULSE_US - DATA_SETUP_US - PROCESSING_AFTER_PULSE_US);

        if (!is_ps2_clk_high()) {
            ps2_device_error = PS2_ERROR_BUSY;
            return EOF;
        }
    }

    if (parity == 0) {
        ps2_device_error = PS2_ERROR_PARITY;
        return EOF;
    }

    // ACK
    ps2_data_set_low();
    ps2_delay_us(DATA_SETUP_US);
    ps2_clock_pulse();
    ps2_delay_us(CLOCK_PULSE_US);
    ps2_data_release();

    return data;
}

int
ps2_device_recv (void) {
    if (are_ps2_lines_high()) {
        // Idle, nothing to receive
        return EOF;
    }

    if (is_ps2_clk_low()) {
        // Host is inhibiting, wait it out
        uint8_t timeout = PS2_BUS_READY_TIMEOUT_US;
        while (--timeout && is_ps2_clk_low()) {
            ps2_delay_us(1);
        }
    }

    disable_interrupts();
    if (is_ps2_clk_low()) {
        // Host is still inhibiting, wait until next main loop iteration
        return EOF;
    }
    const int result = ps2_device_recv_atomic();
    enable_interrupts();

    return result;
}

// MARK: - Output Queue

bool
ps2_device_send (const uint8_t data) {
    if (is_ps2_buffer_full) {
        ps2_device_error = PS2_ERROR_BUFFER_OVERFLOW;
        return false;
    }
    ps2_buffer[modulo_buffer_size(ps2_buffer_head)] = data;
    ++ps2_buffer_head;
    return true;
}

bool
ps2_device_flush (void) {
    while (!is_ps2_buffer_empty) {
        const uint8_t data = ps2_buffer[modulo_buffer_size(ps2_buffer_tail)];
        if (ps2_device_tx_byte(data)) {
            ++ps2_buffer_tail;
        } else {
            return false;
        }
    }
    return true;
}

void
ps2_device_clear_output (void) {
    ps2_buffer_head = ps2_buffer_tail;
}

bool
ps2_device_has_pending_output (void) {
    return !is_ps2_buffer_empty;
}

// MARK: - External queries

char
ps2_device_last_error (void) {
    return ps2_device_error;
}

void
ps2_device_clear_error (void) {
    ps2_device_error = 0;
}
