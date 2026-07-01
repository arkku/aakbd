/**
 * config_check.c: Compile-time config sanity checks.
 *
 * Mainly this tests that various GPIO pin assignments do not conflict with
 * each other. It can't really check against conflicts with other features of
 * the same pins (e.g., SPI hardware mode). This is not in the original QMK,
 * I added this to AAKBD after I spent an hour debugging why a pin doesn't
 * work before finding out that I had mistakenly configured it to conflict
 * with another pin (through an over-complicated local.mk that made it very
 * hard to detect).
 *
 * Copyright 2026 Kimmo Kulovesi
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

#include "gpio.h"

#ifndef LED_NUM_LOCK_PIN
#define LED_NUM_LOCK_PIN 0
#endif
#ifndef LED_CAPS_LOCK_PIN
#define LED_CAPS_LOCK_PIN 0
#endif
#ifndef LED_SCROLL_LOCK_PIN
#define LED_SCROLL_LOCK_PIN 0
#endif
#ifndef LED_COMPOSE_PIN
#define LED_COMPOSE_PIN 0
#endif
#ifndef LED_KANA_PIN
#define LED_KANA_PIN 0
#endif
#ifndef HAPTIC_ENABLE_PIN
#define HAPTIC_ENABLE_PIN 0
#endif
#ifndef SOLENOID_PIN
#define SOLENOID_PIN 0
#endif
#ifndef PS2_STATUS_PIN
#define PS2_STATUS_PIN 0
#endif
#ifndef PS2_ENABLE_PIN
#define PS2_ENABLE_PIN 0
#endif
#ifndef ENABLE_PS2_DEVICE
#define ENABLE_PS2_DEVICE 0
#endif

#define PINS_CONFLICT(a, b) ((a) && (b) && (a) == (b))

_Static_assert(!PINS_CONFLICT(LED_NUM_LOCK_PIN, LED_CAPS_LOCK_PIN),
    "LED_NUM_LOCK_PIN and LED_CAPS_LOCK_PIN conflict");
_Static_assert(!PINS_CONFLICT(LED_NUM_LOCK_PIN, LED_SCROLL_LOCK_PIN),
    "LED_NUM_LOCK_PIN and LED_SCROLL_LOCK_PIN conflict");
_Static_assert(!PINS_CONFLICT(LED_NUM_LOCK_PIN, LED_COMPOSE_PIN),
    "LED_NUM_LOCK_PIN and LED_COMPOSE_PIN conflict");
_Static_assert(!PINS_CONFLICT(LED_NUM_LOCK_PIN, LED_KANA_PIN),
    "LED_NUM_LOCK_PIN and LED_KANA_PIN conflict");
_Static_assert(!PINS_CONFLICT(LED_NUM_LOCK_PIN, HAPTIC_ENABLE_PIN),
    "LED_NUM_LOCK_PIN and HAPTIC_ENABLE_PIN conflict");
_Static_assert(!PINS_CONFLICT(LED_NUM_LOCK_PIN, SOLENOID_PIN),
    "LED_NUM_LOCK_PIN and SOLENOID_PIN conflict");

_Static_assert(!PINS_CONFLICT(LED_CAPS_LOCK_PIN, LED_SCROLL_LOCK_PIN),
    "LED_CAPS_LOCK_PIN and LED_SCROLL_LOCK_PIN conflict");
_Static_assert(!PINS_CONFLICT(LED_CAPS_LOCK_PIN, LED_COMPOSE_PIN),
    "LED_CAPS_LOCK_PIN and LED_COMPOSE_PIN conflict");
_Static_assert(!PINS_CONFLICT(LED_CAPS_LOCK_PIN, LED_KANA_PIN),
    "LED_CAPS_LOCK_PIN and LED_KANA_PIN conflict");
_Static_assert(!PINS_CONFLICT(LED_CAPS_LOCK_PIN, HAPTIC_ENABLE_PIN),
    "LED_CAPS_LOCK_PIN and HAPTIC_ENABLE_PIN conflict");
_Static_assert(!PINS_CONFLICT(LED_CAPS_LOCK_PIN, SOLENOID_PIN),
    "LED_CAPS_LOCK_PIN and SOLENOID_PIN conflict");

_Static_assert(!PINS_CONFLICT(LED_SCROLL_LOCK_PIN, LED_COMPOSE_PIN),
    "LED_SCROLL_LOCK_PIN and LED_COMPOSE_PIN conflict");
_Static_assert(!PINS_CONFLICT(LED_SCROLL_LOCK_PIN, LED_KANA_PIN),
    "LED_SCROLL_LOCK_PIN and LED_KANA_PIN conflict");
_Static_assert(!PINS_CONFLICT(LED_SCROLL_LOCK_PIN, HAPTIC_ENABLE_PIN),
    "LED_SCROLL_LOCK_PIN and HAPTIC_ENABLE_PIN conflict");
_Static_assert(!PINS_CONFLICT(LED_SCROLL_LOCK_PIN, SOLENOID_PIN),
    "LED_SCROLL_LOCK_PIN and SOLENOID_PIN conflict");

_Static_assert(!PINS_CONFLICT(LED_COMPOSE_PIN, LED_KANA_PIN),
    "LED_COMPOSE_PIN and LED_KANA_PIN conflict");
_Static_assert(!PINS_CONFLICT(LED_COMPOSE_PIN, HAPTIC_ENABLE_PIN),
    "LED_COMPOSE_PIN and HAPTIC_ENABLE_PIN conflict");
_Static_assert(!PINS_CONFLICT(LED_COMPOSE_PIN, SOLENOID_PIN),
    "LED_COMPOSE_PIN and SOLENOID_PIN conflict");

_Static_assert(!PINS_CONFLICT(LED_KANA_PIN, HAPTIC_ENABLE_PIN),
    "LED_KANA_PIN and HAPTIC_ENABLE_PIN conflict");
_Static_assert(!PINS_CONFLICT(LED_KANA_PIN, SOLENOID_PIN),
    "LED_KANA_PIN and SOLENOID_PIN conflict");

_Static_assert(!PINS_CONFLICT(HAPTIC_ENABLE_PIN, SOLENOID_PIN),
    "HAPTIC_ENABLE_PIN and SOLENOID_PIN conflict");

#if ENABLE_PS2_DEVICE && defined(PS2_PORT)
#define PASTE2(a, b)        a##b
#define PASTE(a, b)         PASTE2(a, b)

#define PS2_PORT_ADDR       PASTE(PIN, PASTE(PS2_PORT, _ADDRESS))

// PS2_STATUS_PIN and PS2_ENABLE_PIN are raw pin numbers on the PS/2 port
#if PS2_STATUS_PIN
_Static_assert(PS2_STATUS_PIN != PS2_CLK_PIN,
    "PS2_STATUS_PIN conflicts with PS2_CLK_PIN");
_Static_assert(PS2_STATUS_PIN != PS2_DATA_PIN,
    "PS2_STATUS_PIN conflicts with PS2_DATA_PIN");
#endif
#if PS2_ENABLE_PIN
_Static_assert(PS2_ENABLE_PIN != PS2_CLK_PIN,
    "PS2_ENABLE_PIN conflicts with PS2_CLK_PIN");
_Static_assert(PS2_ENABLE_PIN != PS2_DATA_PIN,
    "PS2_ENABLE_PIN conflicts with PS2_DATA_PIN");
#endif
#if PS2_STATUS_PIN && PS2_ENABLE_PIN
_Static_assert(PS2_STATUS_PIN != PS2_ENABLE_PIN,
    "PS2_STATUS_PIN and PS2_ENABLE_PIN conflict");
#endif
// Check against QMK-encoded pins (need port decomposition)
#define PS2_RAW_CONFLICT(pin, raw) ((pin) && \
    (((pin) >> 4) == PS2_PORT_ADDR) && ((pin) & 0x0F) == (raw))

#define PS2_RAW_CHECK(pin, raw, name) \
    _Static_assert(!PS2_RAW_CONFLICT(pin, raw), name " conflicts with " #pin)

#if PS2_STATUS_PIN
PS2_RAW_CHECK(LED_NUM_LOCK_PIN, PS2_STATUS_PIN, "PS2_STATUS_PIN");
PS2_RAW_CHECK(LED_CAPS_LOCK_PIN, PS2_STATUS_PIN, "PS2_STATUS_PIN");
PS2_RAW_CHECK(LED_SCROLL_LOCK_PIN, PS2_STATUS_PIN, "PS2_STATUS_PIN");
PS2_RAW_CHECK(LED_COMPOSE_PIN, PS2_STATUS_PIN, "PS2_STATUS_PIN");
PS2_RAW_CHECK(LED_KANA_PIN, PS2_STATUS_PIN, "PS2_STATUS_PIN");
PS2_RAW_CHECK(HAPTIC_ENABLE_PIN, PS2_STATUS_PIN, "PS2_STATUS_PIN");
PS2_RAW_CHECK(SOLENOID_PIN, PS2_STATUS_PIN, "PS2_STATUS_PIN");
#endif
#if PS2_ENABLE_PIN
PS2_RAW_CHECK(LED_NUM_LOCK_PIN, PS2_ENABLE_PIN, "PS2_ENABLE_PIN");
PS2_RAW_CHECK(LED_CAPS_LOCK_PIN, PS2_ENABLE_PIN, "PS2_ENABLE_PIN");
PS2_RAW_CHECK(LED_SCROLL_LOCK_PIN, PS2_ENABLE_PIN, "PS2_ENABLE_PIN");
PS2_RAW_CHECK(LED_COMPOSE_PIN, PS2_ENABLE_PIN, "PS2_ENABLE_PIN");
PS2_RAW_CHECK(LED_KANA_PIN, PS2_ENABLE_PIN, "PS2_ENABLE_PIN");
PS2_RAW_CHECK(HAPTIC_ENABLE_PIN, PS2_ENABLE_PIN, "PS2_ENABLE_PIN");
PS2_RAW_CHECK(SOLENOID_PIN, PS2_ENABLE_PIN, "PS2_ENABLE_PIN");
#endif

#define PS2_PIN(pin_num)    ((PS2_PORT_ADDR << 4) | (pin_num))
#define PS2_PINS_CONFLICT(pin)  ((pin) && \
    ((pin) == PS2_PIN(PS2_CLK_PIN) || (pin) == PS2_PIN(PS2_DATA_PIN)))

_Static_assert(!PS2_PINS_CONFLICT(LED_NUM_LOCK_PIN),
    "LED_NUM_LOCK_PIN conflicts with PS2 pin");
_Static_assert(!PS2_PINS_CONFLICT(LED_CAPS_LOCK_PIN),
    "LED_CAPS_LOCK_PIN conflicts with PS2 pin");
_Static_assert(!PS2_PINS_CONFLICT(LED_SCROLL_LOCK_PIN),
    "LED_SCROLL_LOCK_PIN conflicts with PS2 pin");
_Static_assert(!PS2_PINS_CONFLICT(LED_COMPOSE_PIN),
    "LED_COMPOSE_PIN conflicts with PS2 pin");
_Static_assert(!PS2_PINS_CONFLICT(LED_KANA_PIN),
    "LED_KANA_PIN conflicts with PS2 pin");
_Static_assert(!PS2_PINS_CONFLICT(HAPTIC_ENABLE_PIN),
    "HAPTIC_ENABLE_PIN conflicts with PS2 pin");
_Static_assert(!PS2_PINS_CONFLICT(SOLENOID_PIN),
    "SOLENOID_PIN conflicts with PS2 pin");
#endif
