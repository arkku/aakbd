/* Copyright 2022 Nick Brassel (@tzarc)
 * Copyright 2026 Kimmo Kulovesi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

/*
 * Bare-metal STM32F3 flash backing store for QMK wear-leveling.
 *
 * This replaces QMK's ChibiOS-backed wear_leveling_efl.c with direct
 * CMSIS register access. Implements the backing_store_* API required by
 * wear_leveling_internal.h.
 *
 * Uses the last flash sector(s) for the backing store, detected from the
 * FLASHSIZE register. For STM32F303xC (256KB flash): 2KB sectors, so
 * WEAR_LEVELING_BACKING_SIZE=2048 uses exactly one sector.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "stm32f3xx.h"
#include "wear_leveling.h"
#include "wear_leveling_internal.h"

/* Flash register unlock keys (from CMSIS stm32f303xc.h) */
/*  FLASH_KEY1 = 0x45670123U, FLASH_KEY2 = 0xCDEF89ABU */

/* STM32F3 flash page size (2KB). Must match MCU. */
#define STM32F3_PAGE_SIZE 2048

static uint32_t wear_leveling_base = 0;   /* flash address of backing store */
static uint32_t wear_leveling_sectors = 0; /* number of sectors used */

static void flash_wait_busy(void) {
    while (FLASH->SR & FLASH_SR_BSY) {
        /* spin */
    }
}

static void flash_clear_errors(void) {
    FLASH->SR = FLASH_SR_PGERR | FLASH_SR_WRPERR | FLASH_SR_EOP;
}

bool backing_store_init(void) {
    /* Detect flash size from hardware register (KB → bytes) */
    uint32_t flash_size = (*(const uint32_t *)FLASHSIZE_BASE) & 0xFFFFU;
    flash_size <<= 10; /* convert KB to bytes */

    /* Calculate last sector(s) for backing store.
     * Working backwards from end of flash to collect WEAR_LEVELING_BACKING_SIZE bytes. */
    uint32_t remaining = WEAR_LEVELING_BACKING_SIZE;
    uint32_t addr = flash_size;
    wear_leveling_sectors = 0;

    while (remaining > 0 && addr > 0) {
        addr -= STM32F3_PAGE_SIZE;
        remaining -= (STM32F3_PAGE_SIZE < remaining) ? STM32F3_PAGE_SIZE : remaining;
        wear_leveling_sectors++;
    }

    if (remaining > 0) {
        return false; /* flash too small */
    }

    wear_leveling_base = FLASH_BASE + addr;
    return true;
}

bool backing_store_unlock(void) {
    flash_wait_busy();
    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;
    flash_wait_busy();
    return true;
}

bool backing_store_erase(void) {
    uint32_t addr = wear_leveling_base;

    for (uint32_t i = 0; i < wear_leveling_sectors; ++i) {
        flash_wait_busy();
        flash_clear_errors();

        FLASH->CR |= FLASH_CR_PER;
        FLASH->AR = addr;
        FLASH->CR |= FLASH_CR_STRT;

        flash_wait_busy();

        FLASH->CR &= ~FLASH_CR_PER;

        if (FLASH->SR & (FLASH_SR_PGERR | FLASH_SR_WRPERR)) {
            return false;
        }

        addr += STM32F3_PAGE_SIZE;
    }

    return true;
}

bool backing_store_write(uint32_t address, backing_store_int_t value) {
    flash_wait_busy();
    flash_clear_errors();

    FLASH->CR |= FLASH_CR_PG;

    /* STM32F3 erased state is 0xFF, algorithm expects 0 → complement */
    *((volatile uint16_t *)(wear_leveling_base + address)) = (uint16_t)~value;

    flash_wait_busy();

    FLASH->CR &= ~FLASH_CR_PG;

    if (FLASH->SR & (FLASH_SR_PGERR | FLASH_SR_WRPERR)) {
        return false;
    }

    return true;
}

bool backing_store_read(uint32_t address, backing_store_int_t *value) {
    /* Complement back: flash reads 0xFF after erase → algorithm sees 0 */
    *value = (backing_store_int_t)~*((volatile uint16_t *)(wear_leveling_base + address));
    return true;
}

bool backing_store_lock(void) {
    FLASH->CR |= FLASH_CR_LOCK;
    return true;
}
