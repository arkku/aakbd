/*
Copyright 2020 Purdea Andrei
Copyright 2021 Kimmo Kulovesi (modified for AAKBD)

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

#include "usb_keys.h"

#define MATRIX_ROWS 8
#define MATRIX_COLS 7

#ifndef DEBOUNCE
#define DEBOUNCE 5
#endif

#define CONTROLLER_IS_XWHATSIT_MODEL_F_OR_WCASS_MODEL_F

#define CAPSENSE_KEYBOARD_SETTLE_TIME_US 8
#define CAPSENSE_DAC_SETTLE_TIME_US 8
#define CAPSENSE_HARDCODED_SAMPLE_TIME 4

#ifndef CAPSENSE_CAL_ENABLED
#define CAPSENSE_CAL_ENABLED 1
#endif

#ifndef CAPSENSE_CAL_DEBUG
#define CAPSENSE_CAL_DEBUG 0
#endif

#define CAPSENSE_CAL_INIT_REPS 16
#define CAPSENSE_CAL_EACHKEY_REPS 16

#ifndef CAPSENSE_CAL_BINS
#define CAPSENSE_CAL_BINS 4
#endif

#ifndef CAPSENSE_CAL_THRESHOLD_OFFSET
#define CAPSENSE_CAL_THRESHOLD_OFFSET 20
#endif

#ifndef CAPSENSE_CAL_THRESHOLD_OFFSET_DYNAMIC
#define CAPSENSE_CAL_THRESHOLD_OFFSET_DYNAMIC 0
#endif

#ifndef CAPSENSE_CAL_MERGE_BINS
#define CAPSENSE_CAL_MERGE_BINS 1
#endif

#ifndef CAPSENSE_CAL_AUTOSAVE
#define CAPSENSE_CAL_AUTOSAVE 0
#endif

#define CAPSENSE_HARDCODED_THRESHOLD 146

#define CAPSENSE_KEYMAP_COL_TO_PHYSICAL_COL(col) ((col >= 2)?(col + 9):(col))

#if HAPTIC_ENABLE
// By default we set up for support of xwhatsit's solenoid driver board.
#ifdef HAPTIC_ENABLE_PIN
#if HAPTIC_ENABLE_PIN == 0
#undef HAPTIC_ENABLE_PIN
#endif
#else
#define HAPTIC_ENABLE_PIN B7
#endif
// We disable haptic feedback during USB low power conditions:
#define HAPTIC_OFF_IN_LOW_POWER 1
// Change this if you are using a different pin for the solenoid:
#ifndef SOLENOID_PIN
#define SOLENOID_PIN B6
#endif
// You can also tune the following for your solenoid:
#ifndef SOLENOID_DEFAULT_DWELL
#define SOLENOID_DEFAULT_DWELL 20
#endif
#define SOLENOID_MIN_DWELL 20
//#define SOLENOID_MAX_DWELL 100
#ifndef SOLENOID_ENABLE
#define SOLENOID_ENABLE
#endif
#else
#define NO_HAPTIC_MOD
#endif

#ifndef LED_NUM_LOCK_PIN
#define LED_NUM_LOCK_PIN B5
#endif
#ifndef LED_CAPS_LOCK_PIN
#define LED_CAPS_LOCK_PIN B4
#endif
#ifndef LED_SCROLL_LOCK_PIN
//#define LED_SCROLL_LOCK_PIN B6
#endif

// Uncomment below if the leds are on when the pin is driving zero:
//#define LED_NUM_LOCK_ACTIVE_LOW
//#define LED_CAPS_LOCK_ACTIVE_LOW
//#define LED_SCROLL_LOCK_ACTIVE_LOW

// Since this keyboard has no natural layout, it will need to be customized
// either in `layers.c` or through Vial. Vial is easy, but `layers.c` is based
// on the names of keys, which for F50 do not exist here for anything other
// than the numpad section on the right. The non-numpad keys therefore have
// these helper aliases KC_R1C1 stands for "keycode row 1 column 1", which is
// the top left corner. There are 5 rows and 6 columns (the two 3×5 areas). You
// can use these directly in `layers.c` like `[KC_R1C1] = KEY(ESC)`.

#if !defined(CUSTOM_KEYMAP) || !CUSTOM_KEYMAP

// Left block:

#define KC_R1C1  USB_KEY_DELETE
#define KC_R1C2  USB_KEY_HOME
#define KC_R1C3  USB_KEY_PAGE_UP

#define KC_R2C1  USB_KEY_INSERT
#define KC_R2C2  USB_KEY_END
#define KC_R2C3  USB_KEY_PAGE_DOWN

#define KC_R3C1  USB_KEY_F16
#define KC_R3C2  USB_KEY_F17
#define KC_R3C3  USB_KEY_F18

#if defined(MEDIA_KEYS_ENDPOINT) && MEDIA_KEYS_ENDPOINT

#define KC_R4C1  USB_KEY_VOLUME_MUTE
#define KC_R4C2  USB_KEY_VOLUME_DOWN
#define KC_R4C3  USB_KEY_VOLUME_UP

#define KC_R5C1  USB_KEY_PREVIOUS_TRACK
#define KC_R5C2  USB_KEY_PLAY_PAUSE
#define KC_R5C3  USB_KEY_NEXT_TRACK

#else

#define KC_R4C1  USB_KEY_F19
#define KC_R4C2  USB_KEY_F20
#define KC_R4C3  USB_KEY_F21

#define KC_R5C1  USB_KEY_F22
#define KC_R5C2  USB_KEY_F23
#define KC_R5C3  USB_KEY_F24

#endif

// Right block:

#define KC_R1C4  USB_KEY_F10
#define KC_R1C5  USB_KEY_F11
#define KC_R1C6  USB_KEY_F12

#define KC_R2C4  USB_KEY_F7
#define KC_R2C5  USB_KEY_F8
#define KC_R2C6  USB_KEY_F9

#define KC_R3C4  USB_KEY_F4
#define KC_R3C5  USB_KEY_F5
#define KC_R3C6  USB_KEY_F6

#define KC_R4C4  USB_KEY_F1
#define KC_R4C5  USB_KEY_F2
#define KC_R4C6  USB_KEY_F3

#define KC_R5C4  USB_KEY_PRINT_SCREEN
#define KC_R5C5  USB_KEY_SCROLL_LOCK
#define KC_R5C6  USB_KEY_PAUSE_BREAK

#endif

#include "post_config.h"
