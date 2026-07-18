/*
Copyright 2011, 2012, 2013 Jun Wako <wakojun@gmail.com>
Copyright 2022 Kimmo Kulovesi (trimmed/tweaked for AAKBD)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "keyboard.h"
#include "eeconfig.h"
#ifdef EEPROM_DRIVER
#include "eeprom_driver.h"
#endif
#include "matrix.h"
#include "timer.h"
#include "led.h"

#ifdef BACKLIGHT_ENABLE
#include "backlight.h"
#endif
#ifdef HAPTIC_ENABLE
#include "haptic.h"
#endif
#ifdef LED_MATRIX_ENABLE
#include "led_matrix.h"
#endif
#ifdef RGB_MATRIX_ENABLE
#include "rgb_matrix.h"
#endif
#ifdef ENCODER_ENABLE
#include "encoder.h"
#endif
#if VIAL_ENABLE
#include "vial.h"
#endif
#include "bootloader.h"
#ifdef BOOTMAGIC_ENABLE
#include "bootmagic.h"
#endif

__attribute__((weak)) void keyboard_pre_init_user(void) {}

__attribute__((weak)) void keyboard_pre_init_kb(void) { keyboard_pre_init_user(); }

__attribute__((weak)) void keyboard_post_init_user() {}

__attribute__((weak)) void keyboard_post_init_kb(void) { keyboard_post_init_user(); }

__attribute__((weak)) void matrix_setup(void) {}

void quantum_init(void) {
    led_init_ports();
#if defined(EEPROM_DRIVER) && !defined(__AVR__)
    eeprom_driver_init();
#endif
#ifdef BACKLIGHT_ENABLE
    backlight_init_ports();
#endif
#ifdef LED_MATRIX_ENABLE
    led_matrix_init();
#endif
#ifdef RGB_MATRIX_ENABLE
    rgb_matrix_init();
#endif
#ifdef HAPTIC_ENABLE
    haptic_init();
#endif
#if VIAL_ENABLE
    vial_init();
#endif
}

void keyboard_setup(void) {
    matrix_setup();
    keyboard_pre_init_kb();
}

void keyboard_init(void) {
    timer_init();
    matrix_init();

#ifdef BOOTMAGIC_ENABLE
    bootmagic();
#endif

    if (!eeconfig_is_enabled()) {
        eeconfig_init();
    }

    quantum_init();

#ifdef BACKLIGHT_ENABLE
    backlight_init();
#endif
#ifdef ENCODER_ENABLE
    encoder_init();
#endif

    keyboard_post_init_kb(); /* Always keep this last */
}
