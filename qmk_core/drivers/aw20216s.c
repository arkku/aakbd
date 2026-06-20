/**
 * aw20216s.c: AW20216S RGB LED driver from QMK, adapted for AAKBD.
 *
 * Copyright 2021 Jasper Chan
 * Copyright 2023 Huckies
 * Copyright 2026 Kimmo Kulovesi
 *
 * This is free software: you can redistribute it and/or modify
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

#include <stdint.h>
#include <stdbool.h>
#include "aw20216s.h"
#include "spi_master.h"
#include "gpio.h"
#include "wait.h"

// Each AW20216S has 12 SW × 18 CS = 216 PWM registers
#define AW20216S_PWM_COUNT 216

#if AW20216S_CHIP_COUNT < 1 || AW20216S_CHIP_COUNT > 2
#error "Only 1 or 2 AW20216S chips supported"
#endif

static uint8_t pwm_buffer[AW20216S_CHIP_COUNT][AW20216S_PWM_COUNT];
static const aw20216s_led_t *leds = 0;
static uint8_t led_count = 0;
static bool initialized = false;

static void aw20216s_write(pin_t cs_pin, uint8_t page, uint8_t reg, const uint8_t *data, uint8_t len) {
    uint8_t header[2] = { AW20216S_ID | (page << 1), reg };
    spi_start(cs_pin, false, 0, 8);
    spi_transmit(header, 2);
    spi_transmit(data, len);
    spi_stop();
}

static void aw20216s_write_register(pin_t cs_pin, uint8_t page, uint8_t reg, uint8_t value) {
    aw20216s_write(cs_pin, page, reg, &value, 1);
}

static void aw20216s_init_chip(pin_t cs_pin) {
    aw20216s_write_register(cs_pin, AW20216S_PAGE_FUNCTION, AW20216S_REG_RESET, AW20216S_RESET_MAGIC);
    wait_ms(2);
    aw20216s_write_register(cs_pin, AW20216S_PAGE_FUNCTION, AW20216S_REG_GLOBAL_CURRENT, AW20216S_GLOBAL_CURRENT_MAX);
    for (uint8_t i = 0; i < AW20216S_PWM_COUNT; ++i) {
        aw20216s_write_register(cs_pin, AW20216S_PAGE_SCALING, i, AW20216S_SCALING_MAX);
    }
    aw20216s_write_register(cs_pin, AW20216S_PAGE_FUNCTION, AW20216S_REG_CONFIGURATION, AW20216S_CONFIGURATION);
    aw20216s_write_register(cs_pin, AW20216S_PAGE_FUNCTION, AW20216S_REG_MIX_FUNCTION, AW20216S_MIX_FUNCTION);
}

void aw20216s_init(const aw20216s_led_t *map, uint8_t count) {
    leds = map;
    led_count = count;

    spi_init();

#ifdef AW20216S_EN_PIN
    gpio_set_pin_output(AW20216S_EN_PIN);
    gpio_write_pin_high(AW20216S_EN_PIN);
    wait_us(1);
#endif

    for (uint8_t i = 0; i < AW20216S_CHIP_COUNT; ++i) {
        const pin_t cs = (i == 0) ? AW20216S_CS_PIN_1 : AW20216S_CS_PIN_2;
        aw20216s_init_chip(cs);
    }

    initialized = true;
}

void aw20216s_set_led_map(const aw20216s_led_t *map, uint8_t count) {
    leds = map;
    led_count = count;
}

void aw20216s_set_color(uint8_t led_index, uint8_t red, uint8_t green, uint8_t blue) {
    if (led_index >= led_count) {
        return;
    }

    const aw20216s_led_t *led = &leds[led_index];
    uint8_t * const buf = pwm_buffer[led->driver_index];
    buf[led->reg_r] = red;
    buf[led->reg_g] = green;
    buf[led->reg_b] = blue;
}

void aw20216s_flush(void) {
    if (!initialized) {
        return;
    }
    for (uint8_t chip = 0; chip < AW20216S_CHIP_COUNT; ++chip) {
        const pin_t cs = (chip == 0) ? AW20216S_CS_PIN_1 : AW20216S_CS_PIN_2;
        aw20216s_write(cs, AW20216S_PAGE_PWM, 0, pwm_buffer[chip], AW20216S_PWM_COUNT);
    }
}
