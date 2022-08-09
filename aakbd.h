/**
 * aakbd.h: USB keyboard implementation.
 *
 * Copyright (c) 2021-2022 Kimmo Kulovesi, https://arkku.dev/
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

#ifndef KK_AAKBD_MAIN_H
#define KK_AAKBD_MAIN_H

#include <stdint.h>

/// Jump to bootloader. Normally `usb_jump_to_bootloader()` should be used
/// instead, as it tears down the USB connection and then calls this.
void jump_to_bootloader(void);

/// Reset the physical keyboard. Also resets the USB keyboard and key processing.
void keyboard_reset(void);

/// The current 10 ms tick count, i.e., a monotonically increasing counter that
/// increments approximately once per 10 milliseconds.
uint8_t current_10ms_tick_count(void);

// MARK: - Helper macros

#if defined(__GNUC__) || defined(__clang__)
#define INLINE inline __attribute__((always_inline))
#else
#define INLINE inline
#endif

/// The least significant byte of `word`.
#define LSB(word)                   ((word) & 0xFF)

/// The most signifant byte of `word`.
#define MSB(word)                   ((word) >> 8)

/// The bytes of `word` in little endian order (array construction helper).
#define WORD_BYTES(word)            LSB(word), MSB(word)

/// Form a 16-bit word from `lsb` and `msb`.
#define BYTES_WORD(lsb, msb)        (((msb) << 8) | (lsb))

/// Divide `value` by `n` and round up.
#define DIV_ROUND_BYTE(n, value)    (((value) / (n)) + ((value) & 1))

// MARK: - Platform-specific

#ifdef __AVR__
#include <util/delay.h>
#include <avr/wdt.h>

#define delay_milliseconds(x) _delay_ms(x)
#define reset_watchdog_timer() wdt_reset()
#else
#error "Platform not supported, edit aakbd.h accordingly."
#endif

#endif
