/**
 * gpio.h: GPIO API for ARM Cortex-M with CMSIS.
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

#pragma once

#include <stdint.h>
#include "platform_deps.h"

typedef uint16_t pin_t;

#define PIN(port, num)  (((port) << 4) | (num))
#define PIN_PORT(pin)   (((pin) >> 4) & 0xF)
#define PIN_NUMBER(pin) ((pin) & 0xF)

// Port base: assumes consecutive ports at fixed spacing (0x400 on STM32).
// Override GPIO_PORT if your MCU has a different port layout.
#ifndef GPIO_PORT_SPACING
#define GPIO_PORT_SPACING 0x400
#endif
#define GPIO_PORT(pin)  ((GPIO_TypeDef *)(GPIOA_BASE + (PIN_PORT(pin) * GPIO_PORT_SPACING)))

#define gpio_set_pin_input(pin)             \
    do {                                    \
        GPIO_PORT(pin)->MODER &= ~(3U << (PIN_NUMBER(pin) * 2)); \
        GPIO_PORT(pin)->PUPDR &= ~(3U << (PIN_NUMBER(pin) * 2)); \
    } while (0)

#define gpio_set_pin_input_high(pin)        \
    do {                                    \
        GPIO_PORT(pin)->MODER &= ~(3U << (PIN_NUMBER(pin) * 2)); \
        GPIO_PORT(pin)->PUPDR = (GPIO_PORT(pin)->PUPDR & ~(3U << (PIN_NUMBER(pin) * 2))) \
                                | (1U << (PIN_NUMBER(pin) * 2)); \
    } while (0)

#define gpio_set_pin_output_push_pull(pin)  \
    do {                                    \
        GPIO_PORT(pin)->MODER = (GPIO_PORT(pin)->MODER & ~(3U << (PIN_NUMBER(pin) * 2))) \
                                | (1U << (PIN_NUMBER(pin) * 2)); \
        GPIO_PORT(pin)->OTYPER &= ~(1U << PIN_NUMBER(pin)); \
    } while (0)

#define gpio_set_pin_output(pin) gpio_set_pin_output_push_pull(pin)

#define gpio_write_pin_high(pin)  do { GPIO_PORT(pin)->BSRR = (1U << PIN_NUMBER(pin)); } while (0)
#define gpio_write_pin_low(pin)   do { GPIO_PORT(pin)->BSRR = (1U << (PIN_NUMBER(pin) + 16)); } while (0)
#define gpio_read_pin(pin)        ((GPIO_PORT(pin)->IDR >> PIN_NUMBER(pin)) & 1U)
#define gpio_toggle_pin(pin)      do { GPIO_PORT(pin)->ODR ^= (1U << PIN_NUMBER(pin)); } while (0)

#define gpio_write_pin(pin, level)          \
    do {                                    \
        if (level) gpio_write_pin_high(pin); \
        else       gpio_write_pin_low(pin);  \
    } while (0)

/// Set pin to alternate function mode with the given AF number (0-15).
/// Clears the MODER bits for the pin and sets the AF in the AFR register.
#define gpio_set_pin_alternate(pin, af)                                     \
    do {                                                                    \
        GPIO_PORT(pin)->MODER = (GPIO_PORT(pin)->MODER & ~(3U << (PIN_NUMBER(pin) * 2))) \
                                | (2U << (PIN_NUMBER(pin) * 2));            \
        uint8_t _afr_idx = PIN_NUMBER(pin) >> 3;                            \
        GPIO_PORT(pin)->AFR[_afr_idx] =                                     \
            (GPIO_PORT(pin)->AFR[_afr_idx] & ~(0xF << ((PIN_NUMBER(pin) & 7) * 4))) \
            | ((uint32_t)(af) << ((PIN_NUMBER(pin) & 7) * 4));              \
    } while (0)
