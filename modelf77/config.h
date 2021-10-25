/*
Copyright 2020 Purdea Andrei

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

#pragma once

#include "config_common.h"

/* key matrix size */
#define MATRIX_ROWS 8
#define MATRIX_COLS 11
// Note: physical column are 16, but only 11 are ever used. Column 0..9 match the physical column. Column 10 is physical column 15.

/* Debounce reduces chatter (unintended double-presses) - set 0 if debouncing is not needed */
#ifndef DEBOUNCE
#define DEBOUNCE 5
#endif

#define CONTROLLER_IS_XWHATSIT_MODEL_F_OR_WCASS_MODEL_F
//#define CONTROLLER_IS_XWHATSIT_BEAMSPRING_REV_4
//#define CONTROLLER_IS_THROUGH_HOLE_BEAMSPRING
//#define CONTROLLER_IS_THROUGH_HOLE_MODEL_F

#define CAPSENSE_KEYBOARD_SETTLE_TIME_US 8
#define CAPSENSE_DAC_SETTLE_TIME_US 8
#define CAPSENSE_HARDCODED_SAMPLE_TIME 4

#define CAPSENSE_CAL_ENABLED 1
// #define CAPSENSE_CAL_ENABLED 0
#define CAPSENSE_CAL_DEBUG 1
// #define CAPSENSE_CAL_DEBUG 0
#define CAPSENSE_CAL_INIT_REPS 16
#define CAPSENSE_CAL_EACHKEY_REPS 16
#define CAPSENSE_CAL_BINS 5
#define CAPSENSE_CAL_THRESHOLD_OFFSET 24

#if !CAPSENSE_CAL_ENABLED
#define CAPSENSE_HARDCODED_THRESHOLD 142
#endif

#define CAPSENSE_KEYMAP_COL_TO_PHYSICAL_COL(col) (((col) == 10)?15:(col))

#if HAPTIC_ENABLE
// By default we set up for support of xwhatsit's solenoid driver board.
// Comment out HAPTIC_ENABLE_PIN if you don't have an enable pin:
#define HAPTIC_ENABLE_PIN B7
// We disable haptic feedbeck during USB low power conditions:
#define HAPTIC_OFF_IN_LOW_POWER 1
// Change this if you are using a different pin for the solenoid:
#define SOLENOID_PIN B6
// If you are not using a solenoid then comment out the above, and also in rules.mk, remove "HAPTIC_ENABLE += SOLENOID"
// You can also tune the following for your solenoid:
#define SOLENOID_DEFAULT_DWELL 4
#define SOLENOID_MIN_DWELL 4
//#define SOLENOID_MAX_DWELL 100
#define NO_HAPTIC_MOD
#error "Solenoid support has not been ported to AAKBD (yet?)."
#endif

#define LED_NUM_LOCK_PIN B4
#define LED_CAPS_LOCK_PIN B5
#define LED_SCROLL_LOCK_PIN B6

// Uncomment below if the leds are on when the pin is driving zero:
//#define LED_NUM_LOCK_ACTIVE_LOW
//#define LED_CAPS_LOCK_ACTIVE_LOW
//#define LED_SCROLL_LOCK_ACTIVE_LOW

#include "post_config.h"
