/**
 * system_stm32f3xx.c: System clock configuration for STM32F303xC.
 *
 * Copyright 2026 Kimmo Kulovesi
 *
 * This is free software: you can redistribute it and/or modify
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

#include "stm32f3xx.h"

/* Oscillator frequencies — STM32F303 has 8 MHz HSI and HSE */
#define HSI_HZ  8000000
#define HSE_HZ  8000000

/* System clock frequency when running from PLL (HSE × 9 = 72 MHz) */
#define CPU_HZ  72000000

uint32_t SystemCoreClock = CPU_HZ;

const uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
const uint8_t APBPrescTable[8]  = {0, 0, 0, 0, 1, 2, 3, 4};

void SystemInit(void) {
    // Enable HSE oscillator
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY)) {}

    // Configure flash: 2 wait states for 72 MHz, enable prefetch buffer
    FLASH->ACR = (FLASH->ACR & ~(FLASH_ACR_LATENCY | FLASH_ACR_PRFTBE))
               | FLASH_ACR_LATENCY_2 | FLASH_ACR_PRFTBE;

    // Configure AHB = SYSCLK /1, APB1 = SYSCLK /2, APB2 = SYSCLK /1
    RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2
                               | RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL))
              | RCC_CFGR_HPRE_DIV1
              | RCC_CFGR_PPRE1_DIV2
              | RCC_CFGR_PPRE2_DIV1
              | RCC_CFGR_PLLSRC_HSE_PREDIV
              | RCC_CFGR_PLLMUL9;

    // USB prescaler: PLL / 1.5 = 48 MHz (default reset value, ensure it)
    RCC->CFGR &= ~RCC_CFGR_USBPRE;

    // Enable PLL
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY)) {}

    // Switch system clock to PLL
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {}

    // Enable USB clock
    RCC->APB1ENR |= RCC_APB1ENR_USBEN;
    (void) RCC->APB1ENR;

    // Enable SYSCFG clock (for GPIO configuration)
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    (void) RCC->APB2ENR;

    SystemCoreClock = CPU_HZ;
}

void SystemCoreClockUpdate(void) {
    uint32_t sws, pllmull, pllsource, predivfactor, ahb_shift;

    sws = RCC->CFGR & RCC_CFGR_SWS;

    switch (sws) {
    case RCC_CFGR_SWS_HSI:
        SystemCoreClock = HSI_HZ;
        break;
    case RCC_CFGR_SWS_HSE:
        SystemCoreClock = HSE_HZ;
        break;
    case RCC_CFGR_SWS_PLL:
        pllmull = RCC->CFGR & RCC_CFGR_PLLMUL;
        pllsource = RCC->CFGR & RCC_CFGR_PLLSRC;
        pllmull = (pllmull >> 18) + 2;

        predivfactor = (RCC->CFGR2 & RCC_CFGR2_PREDIV) + 1;

        if (pllsource == RCC_CFGR_PLLSRC_HSE_PREDIV) {
            SystemCoreClock = (HSE_HZ / predivfactor) * pllmull;
        } else {
            SystemCoreClock = (HSI_HZ / 2 / predivfactor) * pllmull;
        }
        break;
    default:
        SystemCoreClock = HSI_HZ;
        break;
    }

    ahb_shift = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4)];
    SystemCoreClock >>= ahb_shift;
}
