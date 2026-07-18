/*
Copyright 2012 Jun Wako <wakojun@gmail.com>
Copyright 2013 Oleg Kostyuk <cub.uanic@gmail.com>
Copyright 2015 ZSA Technology Labs Inc (@zsa)
Copyright 2020 Christopher Courtney <drashna@live.com> (@drashna)

Ported from QMK to AAKBD by Kimmo Kulovesi, 2021. Any issues are
almost certainly caused by that and not by the original authors.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#ifndef DEBOUNCE
/// Number of milliseconds to debounce switches. Cherry MX switches should be
/// ok with 5, but some other switches may (according to internet forums)
/// require 15 or more.
#define DEBOUNCE                8
#endif

#define MATRIX_ROWS             14
#define MATRIX_ROWS_PER_SIDE    (MATRIX_ROWS / 2)
#define MATRIX_COLS             6

#ifndef LED_BRIGHTNESS_LO
#define LED_BRIGHTNESS_LO       15
#endif
#ifndef LED_BRIGHTNESS_HI
#define LED_BRIGHTNESS_HI       255
#endif
#define LED_BRIGHTNESS_DEFAULT (LED_BRIGHTNESS_HI)

// Positional key aliases for use in `layers.c` and `macros.c`.
// Each KC_R{row}C{col}{hand} maps to the physical USB keycode at that
// position. The row/col numbering follows the visual layout, rows 1-5
// and thumb rows 6-8. These can be overridden by defining CUSTOM_KEYMAP
// and providing different assignments.
#if !defined(CUSTOM_KEYMAP) || !CUSTOM_KEYMAP

// Left hand, rows 1-5 (main area):
#define KC_R1C1L USB_KEY_EQUALS
#define KC_R1C2L USB_KEY_1
#define KC_R1C3L USB_KEY_2
#define KC_R1C4L USB_KEY_3
#define KC_R1C5L USB_KEY_4
#define KC_R1C6L USB_KEY_5
#define KC_R1C7L USB_KEY_KP_4_LEFT

#define KC_R2C1L USB_KEY_TAB
#define KC_R2C2L USB_KEY_Q
#define KC_R2C3L USB_KEY_W
#define KC_R2C4L USB_KEY_E
#define KC_R2C5L USB_KEY_R
#define KC_R2C6L USB_KEY_T
#define KC_R2C7L USB_KEY_LEFT_CTRL

#define KC_R3C1L USB_KEY_CAPS_LOCK
#define KC_R3C2L USB_KEY_A
#define KC_R3C3L USB_KEY_S
#define KC_R3C4L USB_KEY_D
#define KC_R3C5L USB_KEY_F
#define KC_R3C6L USB_KEY_G

#define KC_R4C1L USB_KEY_LEFT_SHIFT
#define KC_R4C2L USB_KEY_Z
#define KC_R4C3L USB_KEY_X
#define KC_R4C4L USB_KEY_C
#define KC_R4C5L USB_KEY_V
#define KC_R4C6L USB_KEY_B
#define KC_R4C7L USB_KEY_VIRTUAL_APPLE_FN

#define KC_R5C1L USB_KEY_BACKTICK
#define KC_R5C2L USB_KEY_INT_NEXT_TO_LEFT_SHIFT
#define KC_R5C3L USB_KEY_LEFT_ALT
#define KC_R5C4L USB_KEY_LEFT_ARROW
#define KC_R5C5L USB_KEY_RIGHT_ARROW

// Left thumb cluster (rows 6-8):
#define KC_R6C1L USB_KEY_MENU
#define KC_R6C2L USB_KEY_LEFT_GUI

#define KC_R7C1L USB_KEY_HOME

#define KC_R8C1L USB_KEY_SPACE
#define KC_R8C2L USB_KEY_BACKSPACE
#define KC_R8C3L USB_KEY_END

// Right hand, rows 1-5 (main area):
#define KC_R1C1R USB_KEY_KP_6_RIGHT
#define KC_R1C2R USB_KEY_6
#define KC_R1C3R USB_KEY_7
#define KC_R1C4R USB_KEY_8
#define KC_R1C5R USB_KEY_9
#define KC_R1C6R USB_KEY_0
#define KC_R1C7R USB_KEY_DASH

#define KC_R2C1R USB_KEY_RIGHT_CTRL
#define KC_R2C2R USB_KEY_Y
#define KC_R2C3R USB_KEY_U
#define KC_R2C4R USB_KEY_I
#define KC_R2C5R USB_KEY_O
#define KC_R2C6R USB_KEY_P
#define KC_R2C7R USB_KEY_BACKSLASH

#define KC_R3C2R USB_KEY_H
#define KC_R3C3R USB_KEY_J
#define KC_R3C4R USB_KEY_K
#define KC_R3C5R USB_KEY_L
#define KC_R3C6R USB_KEY_SEMICOLON
#define KC_R3C7R USB_KEY_QUOTE

#define KC_R4C1R USB_KEY_INSERT
#define KC_R4C2R USB_KEY_N
#define KC_R4C3R USB_KEY_M
#define KC_R4C4R USB_KEY_COMMA
#define KC_R4C5R USB_KEY_PERIOD
#define KC_R4C6R USB_KEY_SLASH
#define KC_R4C7R USB_KEY_RIGHT_SHIFT

#define KC_R5C3R USB_KEY_UP_ARROW
#define KC_R5C4R USB_KEY_DOWN_ARROW
#define KC_R5C5R USB_KEY_OPEN_BRACKET
#define KC_R5C6R USB_KEY_CLOSE_BRACKET
#define KC_R5C7R USB_KEY_SCROLL_LOCK

// Right thumb cluster (rows 6-8):
#define KC_R6C1R USB_KEY_LEFT_ALT
#define KC_R6C2R USB_KEY_ESC

#define KC_R7C1R USB_KEY_PAGE_UP

#define KC_R8C1R USB_KEY_PAGE_DOWN
#define KC_R8C2R USB_KEY_TAB
#define KC_R8C3R USB_KEY_RETURN

#endif

#if VIAL_ENABLE
#include "vial_config.h"
#endif
