/**
 * spi_master.c: SPI master for ARM.
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

#include "spi_master.h"
#include "platform_deps.h"
#include "pin_defs.h"

#ifdef STM32F303xC
#define SPI_SCK_PIN  A5
#define SPI_MOSI_PIN A7
#endif

#ifdef MCU_SERIES_STM32F3
#define spi_clock_enable()    do { RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; (void)RCC->APB2ENR; } while (0)
#else
#error "SPI not ported to this MCU series"
#endif

#ifndef SPI_TIMEOUT
#define SPI_TIMEOUT 100
#endif

static pin_t   current_slave_pin     = NO_PIN;
static bool    current_cs_active_low = true;

static void spi_select(void) {
    gpio_write_pin(current_slave_pin, current_cs_active_low ? 0 : 1);
}

static void spi_unselect(void) {
    gpio_write_pin(current_slave_pin, current_cs_active_low ? 1 : 0);
}

void spi_init(void) {
    spi_clock_enable();

    gpio_set_pin_alternate(SPI_SCK_PIN, 5);
    gpio_set_pin_alternate(SPI_MOSI_PIN, 5);

    SPI1->CR1 = 0;
    SPI1->CR2 = 0;
}

bool spi_start(pin_t slavePin, bool lsbFirst, uint8_t mode, uint16_t divisor) {
    spi_start_config_t cfg = { .slave_pin = slavePin, .lsb_first = lsbFirst,
                               .mode = mode, .divisor = divisor, .cs_active_low = true };
    return spi_start_extended(&cfg);
}

bool spi_start_extended(spi_start_config_t *start_config) {
    if (current_slave_pin != NO_PIN || start_config->slave_pin == NO_PIN) {
        return false;
    }

    uint32_t cr1 = SPI_CR1_MSTR | SPI_CR1_SSI | SPI_CR1_SSM;

    if (start_config->lsb_first) {
        cr1 |= SPI_CR1_LSBFIRST;
    }

    if (start_config->mode & 0x02) {
        cr1 |= SPI_CR1_CPOL;
    }
    if (start_config->mode & 0x01) {
        cr1 |= SPI_CR1_CPHA;
    }

    uint16_t rounded = 2;
    while (rounded < start_config->divisor) {
        rounded <<= 1;
    }
    switch (rounded) {
    case 2:   cr1 |= 0; break;
    case 4:   cr1 |= SPI_CR1_BR_0; break;
    case 8:   cr1 |= SPI_CR1_BR_1; break;
    case 16:  cr1 |= SPI_CR1_BR_1 | SPI_CR1_BR_0; break;
    case 32:  cr1 |= SPI_CR1_BR_2; break;
    case 64:  cr1 |= SPI_CR1_BR_2 | SPI_CR1_BR_0; break;
    case 128: cr1 |= SPI_CR1_BR_2 | SPI_CR1_BR_1; break;
    case 256: cr1 |= SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0; break;
    default:  return false;
    }

    SPI1->CR1 = cr1;
    SPI1->CR2 = SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2 | SPI_CR2_FRXTH;
    SPI1->CR1 |= SPI_CR1_SPE;

    current_slave_pin     = start_config->slave_pin;
    current_cs_active_low = start_config->cs_active_low;
    gpio_set_pin_output(current_slave_pin);
    spi_unselect();
    spi_select();

    return true;
}

static spi_status_t spi_wait_txe(void) {
    uint32_t timeout = DWT->CYCCNT + 72000;
    while (!(SPI1->SR & SPI_SR_TXE)) {
        if ((int32_t)(DWT->CYCCNT - timeout) >= 0) {
            SPI1->CR1 = 0;
            return SPI_STATUS_TIMEOUT;
        }
    }
    return SPI_STATUS_SUCCESS;
}

static spi_status_t spi_wait_rxne(void) {
    uint32_t timeout = DWT->CYCCNT + 72000;
    while (!(SPI1->SR & SPI_SR_RXNE)) {
        if ((int32_t)(DWT->CYCCNT - timeout) >= 0) {
            SPI1->CR1 = 0;
            return SPI_STATUS_TIMEOUT;
        }
    }
    return SPI_STATUS_SUCCESS;
}

spi_status_t spi_write(uint8_t data) {
    spi_status_t status = spi_wait_txe();
    if (status < 0) {
        return status;
    }
    *((__IO uint8_t *)&SPI1->DR) = data;
    status = spi_wait_rxne();
    if (status < 0) {
        return status;
    }
    return SPI1->DR;
}

spi_status_t spi_read(void) {
    spi_status_t status = spi_wait_txe();
    if (status < 0) {
        return status;
    }
    *((__IO uint8_t *)&SPI1->DR) = 0;
    status = spi_wait_rxne();
    if (status < 0) {
        return status;
    }
    return SPI1->DR;
}

spi_status_t spi_transmit(const uint8_t *data, uint16_t length) {
    for (uint16_t i = 0; i < length; ++i) {
        spi_status_t status = spi_write(data[i]);
        if (status < 0) {
            return status;
        }
    }
    return SPI_STATUS_SUCCESS;
}

spi_status_t spi_receive(uint8_t *data, uint16_t length) {
    for (uint16_t i = 0; i < length; ++i) {
        spi_status_t status = spi_read();
        if (status >= 0) {
            data[i] = (uint8_t)status;
        } else {
            return status;
        }
    }
    return SPI_STATUS_SUCCESS;
}

void spi_stop(void) {
    if (current_slave_pin != NO_PIN) {
        while (SPI1->SR & SPI_SR_BSY) {
            uint32_t timeout = DWT->CYCCNT + 72000;
            if ((int32_t)(DWT->CYCCNT - timeout) >= 0) {
                break;
            }
        }
        if (SPI1->SR & SPI_SR_OVR) {
            (void)SPI1->DR;
            (void)SPI1->SR;
        }
        while (SPI1->SR & SPI_SR_RXNE) {
            (void)SPI1->DR;
        }
        gpio_set_pin_output(current_slave_pin);
        spi_unselect();
        current_slave_pin = NO_PIN;
        SPI1->CR1 = 0;
    }
}
