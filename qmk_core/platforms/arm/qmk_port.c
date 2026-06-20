/**
 * arm.c: ARM platform support.
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

#include "qmk_port.h"
#include "platform_deps.h"

void protocol_pre_init(void) {
    __disable_irq();
}

void protocol_post_init(void) {
    __enable_irq();
}

void protocol_setup(void) {
    // Enable GPIO clocks for commonly-used ports
#if defined(GPIOA)
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
#endif
#if defined(GPIOB)
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
#endif
#if defined(GPIOC)
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
#endif
#if defined(GPIOD)
    RCC->AHBENR |= RCC_AHBENR_GPIODEN;
#endif
#if defined(GPIOE)
    RCC->AHBENR |= RCC_AHBENR_GPIOEEN;
#endif
    (void) RCC->AHBENR;
}
