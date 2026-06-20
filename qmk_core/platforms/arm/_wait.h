/**
 * _wait.h: ARM wait / delay functions.
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

#ifndef CPU_CLOCK
#define CPU_CLOCK SystemCoreClock
#endif

#define wait_ms(ms)                                     \
    do {                                                \
        extern uint32_t timer_read32(void);             \
        uint32_t _wait_start = timer_read32();          \
        while ((uint32_t)(timer_read32() - _wait_start) < (ms)) { } \
    } while (0)

#define wait_us(us) wait_cpuclock((uint32_t)(us) * (CPU_CLOCK / 1000000))

#if defined(DWT)
__attribute__((always_inline)) static inline void wait_cpuclock(unsigned int n) {
    uint32_t _start = DWT->CYCCNT;
    while ((uint32_t)(DWT->CYCCNT - _start) < n) { }
}
#else
// Fallback for chips without DWT. Each loop iteration (subs + bne) takes ~2 cycles.
#warning "wait_cpuclock: no DWT available — using cycle-approximate busy loop"
__attribute__((always_inline)) static inline void wait_cpuclock(unsigned int n) {
    n >>= 1;
    if (n > 0) {
        __asm__ volatile (
            "1: subs %0, #1\n"
            "   bne 1b\n"
            : "+r"(n)
            :
            : "cc"
        );
    }
}
#endif

// 0.25 microseconds for GPIO input settling
#ifndef GPIO_INPUT_PIN_DELAY
#define GPIO_INPUT_PIN_DELAY (CPU_CLOCK / 1000000L / 4)
#endif

#define waitInputPinDelay() wait_cpuclock(GPIO_INPUT_PIN_DELAY)

