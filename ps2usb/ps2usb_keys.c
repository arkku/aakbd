/**
 * ps2usb_keys.c: PS/2 scancode to USB mapping (sets 2 and 3).
 *
 * Copyright (c) 2021 Kimmo Kulovesi, https://arkku.dev/
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
#include <stdbool.h>
#include <stdint.h>
#include <avr/pgmspace.h>

#define KK_KEYCODES_INCLUDE_DUPLICATES 1

#include "ps2usb_keys.h"
#include "ps2_keys.h"
#include <usb_keys.h>
#include <usbkbd_config.h>

static const uint8_t PROGMEM ps2_to_usb_set3[] = {
    // Function keys
    [KEY_ESC] = USB_KEY_ESC,
    [KEY_F1] = USB_KEY_F1,
    [KEY_F2] = USB_KEY_F2,
    [KEY_F3] = USB_KEY_F3,
    [KEY_F4] = USB_KEY_F4,
    [KEY_F5] = USB_KEY_F5,
    [KEY_F6] = USB_KEY_F6,
    [KEY_F7] = USB_KEY_F7,
    [KEY_F8] = USB_KEY_F8,
    [KEY_F9] = USB_KEY_F9,
    [KEY_F10] = USB_KEY_F10,
    [KEY_F11] = USB_KEY_F11,
    [KEY_F12] = USB_KEY_F12,
    [KEY_F13] = USB_KEY_F13,
    [KEY_F14] = USB_KEY_F14,
    [KEY_F15] = USB_KEY_F15,
    [KEY_F16] = USB_KEY_F16,
    [KEY_F17] = USB_KEY_F17,

    // Number row
    [KEY_BACKTICK] = USB_KEY_BACKTICK,
    [KEY_1] = USB_KEY_1,
    [KEY_2] = USB_KEY_2,
    [KEY_3] = USB_KEY_3,
    [KEY_4] = USB_KEY_4,
    [KEY_5] = USB_KEY_5,
    [KEY_6] = USB_KEY_6,
    [KEY_7] = USB_KEY_7,
    [KEY_8] = USB_KEY_8,
    [KEY_9] = USB_KEY_9,
    [KEY_0] = USB_KEY_0,
    [KEY_DASH] = USB_KEY_DASH,
    [KEY_EQUALS] = USB_KEY_EQUALS,
    [KEY_BACKSPACE] = USB_KEY_BACKSPACE,
    [KEY_INT_LEFT_OF_BACKSPACE] = USB_KEY_INT_LEFT_OF_BACKSPACE,

    // Top row
    [KEY_TAB] = USB_KEY_TAB,
    [KEY_Q] = USB_KEY_Q,
    [KEY_W] = USB_KEY_W,
    [KEY_E] = USB_KEY_E,
    [KEY_R] = USB_KEY_R,
    [KEY_T] = USB_KEY_T,
    [KEY_Y] = USB_KEY_Y,
    [KEY_U] = USB_KEY_U,
    [KEY_I] = USB_KEY_I,
    [KEY_O] = USB_KEY_O,
    [KEY_P] = USB_KEY_P,
    [KEY_OPEN_BRACKET] = USB_KEY_OPEN_BRACKET,
    [KEY_CLOSE_BRACKET] = USB_KEY_CLOSE_BRACKET,

    [KEY_ANSI_BACKSLASH] = USB_KEY_ANSI_BACKSLASH,
    [KEY_RETURN] = USB_KEY_RETURN,

    // Home row
    [KEY_CAPS_LOCK] = USB_KEY_CAPS_LOCK,
    [KEY_A] = USB_KEY_A,
    [KEY_S] = USB_KEY_S,
    [KEY_D] = USB_KEY_D,
    [KEY_F] = USB_KEY_F,
    [KEY_G] = USB_KEY_G,
    [KEY_H] = USB_KEY_H,
    [KEY_J] = USB_KEY_J,
    [KEY_K] = USB_KEY_K,
    [KEY_L] = USB_KEY_L,
    [KEY_SEMICOLON] = USB_KEY_SEMICOLON,
    [KEY_QUOTE] = USB_KEY_QUOTE,
    [KEY_INT_NEXT_TO_RETURN] = USB_KEY_INT_NEXT_TO_RETURN,

    // Bottom row
    [KEY_LEFT_SHIFT] = USB_KEY_LEFT_SHIFT,
    [KEY_INT_NEXT_TO_LEFT_SHIFT] = USB_KEY_INT_NEXT_TO_LEFT_SHIFT,
    [KEY_Z] = USB_KEY_Z,
    [KEY_X] = USB_KEY_X,
    [KEY_C] = USB_KEY_C,
    [KEY_V] = USB_KEY_V,
    [KEY_B] = USB_KEY_B,
    [KEY_N] = USB_KEY_N,
    [KEY_M] = USB_KEY_M,
    [KEY_COMMA] = USB_KEY_COMMA,
    [KEY_PERIOD] = USB_KEY_PERIOD,
    [KEY_SLASH] = USB_KEY_SLASH,
    [KEY_RIGHT_SHIFT] = USB_KEY_RIGHT_SHIFT,
    [KEY_INT_LEFT_OF_RIGHT_SHIFT] = USB_KEY_INT_LEFT_OF_RIGHT_SHIFT,

    // Modifier row
    [KEY_LEFT_CTRL] = USB_KEY_LEFT_CTRL,
    [KEY_LEFT_WIN] = USB_KEY_LEFT_WIN,
    [KEY_LEFT_ALT] = USB_KEY_LEFT_ALT,
    [KEY_SPACE] = USB_KEY_SPACE,
    [KEY_RIGHT_ALT] = USB_KEY_RIGHT_ALT,
    [KEY_RIGHT_WIN] = USB_KEY_RIGHT_WIN,
    [KEY_MENU] = USB_KEY_MENU,
    [KEY_RIGHT_CTRL] = USB_KEY_RIGHT_CTRL,

    // Tenkey block
    [KEY_PRINT_SCREEN] = USB_KEY_PRINT_SCREEN,
    [KEY_SCROLL_LOCK] = USB_KEY_SCROLL_LOCK,
    [KEY_PAUSE_BREAK] = USB_KEY_PAUSE_BREAK,
    [KEY_INSERT] = USB_KEY_INSERT,
    [KEY_DELETE] = USB_KEY_DELETE,
    [KEY_HOME] = USB_KEY_HOME,
    [KEY_END] = USB_KEY_END,
    [KEY_PAGE_UP] = USB_KEY_PAGE_UP,
    [KEY_PAGE_DOWN] = USB_KEY_PAGE_DOWN,

    [KEY_UP_ARROW] = USB_KEY_UP_ARROW,
    [KEY_LEFT_ARROW] = USB_KEY_LEFT_ARROW,
    [KEY_DOWN_ARROW] = USB_KEY_DOWN_ARROW,
    [KEY_RIGHT_ARROW] = USB_KEY_RIGHT_ARROW,

    // Keypad
    [KEY_NUM_LOCK] = USB_KEY_NUM_LOCK,
    [KEY_KP_DIVIDE] = USB_KEY_KP_DIVIDE,
    [KEY_KP_MULTIPLY] = USB_KEY_KP_MULTIPLY,
    [KEY_KP_MINUS] = USB_KEY_KP_MINUS,
    [KEY_KP_7_HOME] = USB_KEY_KP_7_HOME,
    [KEY_KP_8_UP] = USB_KEY_KP_8_UP,
    [KEY_KP_9_PAGE_UP] = USB_KEY_KP_9_PAGE_UP,
    [KEY_KP_PLUS] = USB_KEY_KP_PLUS,
    [KEY_KP_4_LEFT] = USB_KEY_KP_4_LEFT,
    [KEY_KP_5] = USB_KEY_KP_5,
    [KEY_KP_6_RIGHT] = USB_KEY_KP_6_RIGHT,
    [KEY_KP_1_END] = USB_KEY_KP_1_END,
    [KEY_KP_2_DOWN] = USB_KEY_KP_2_DOWN,
    [KEY_KP_3_PAGE_DOWN] = USB_KEY_KP_3_PAGE_DOWN,
    [KEY_KP_ENTER] = USB_KEY_KP_ENTER,
    [KEY_KP_0_INSERT] = USB_KEY_KP_0_INSERT,
    [KEY_KP_COMMA_DEL] = USB_KEY_KP_COMMA_DEL,

    [KEY_KATAKANA] = USB_KEY_KATAKANA,
    [KEY_KANJI] = USB_KEY_KANJI,
    [KEY_HIRAGANA] = USB_KEY_HIRAGANA,

#if ENABLE_MEDIA_KEYS
    [KEY_MUTE] = USB_KEY_VOLUME_MUTE,
    [KEY_VOLUME_UP] = USB_KEY_VOLUME_UP,
    [KEY_VOLUME_DOWN] = USB_KEY_VOLUME_DOWN,
    [KEY_NEXT_TRACK] = USB_KEY_NEXT_TRACK,
    [KEY_PREVIOUS_TRACK] = USB_KEY_PREVIOUS_TRACK,
    [KEY_STOP] = USB_KEY_STOP,
#endif

    // Non-overlapping set 2 keys (in case keyboard sends wrong keycode)
    [KEY_F1_SET2] = USB_KEY_F1,
    [KEY_F2_SET2] = USB_KEY_F2,
    [KEY_F3_SET2] = USB_KEY_F3,
    [KEY_F4_SET2] = USB_KEY_F4,
    [KEY_F5_SET2] = USB_KEY_F5,
    [KEY_F6_SET2] = USB_KEY_F6,
    [KEY_F8_SET2] = USB_KEY_F8,
    [KEY_F9_SET2] = USB_KEY_F9,
    [KEY_F10_SET2] = USB_KEY_F10,
    [KEY_F11_SET2] = USB_KEY_F11,
    [KEY_F14_SET2] = USB_KEY_F14,
    [KEY_F15_SET2] = USB_KEY_F15,
    [KEY_F16_SET2] = USB_KEY_F16,
    [KEY_F17_SET2] = USB_KEY_F17,
    [KEY_KP_MINUS_SET2] = USB_KEY_KP_MINUS,
};

// Set 2 non-extended (single-byte) scancode → USB keycode.
// Extended keys (E0-prefix) and the multi-byte Pause are handled separately.
// Some entries use Set 3 scancode constants as indices; these are non-standard
// Set 2 codes accepted for compatibility with keyboards that send them.
static const uint8_t PROGMEM ps2_to_usb_set2[] = {
    // Function keys
    [KEY_ESC_SET2] = USB_KEY_ESC,
    [KEY_F1_SET2] = USB_KEY_F1,
    [KEY_F2_SET2] = USB_KEY_F2,
    [KEY_F3_SET2] = USB_KEY_F3,
    [KEY_F4_SET2] = USB_KEY_F4,
    [KEY_F5_SET2] = USB_KEY_F5,
    [KEY_F6_SET2] = USB_KEY_F6,
    [KEY_F7_SET2] = USB_KEY_F7,
    [KEY_F8_SET2] = USB_KEY_F8,
    [KEY_F9_SET2] = USB_KEY_F9,
    [KEY_F10_SET2] = USB_KEY_F10,
    [KEY_F11_SET2] = USB_KEY_F11,
    [KEY_F12_SET2] = USB_KEY_F12,
    [KEY_F13_SET2] = USB_KEY_F13,
    [KEY_F14_SET2] = USB_KEY_F14,
    [KEY_F15_SET2] = USB_KEY_F15,
    [KEY_F16_SET2] = USB_KEY_F16,
    [KEY_F17_SET2] = USB_KEY_F17,
    [KEY_F18_SET2] = USB_KEY_F18,
    [KEY_F19_SET2] = USB_KEY_F19,
    [KEY_F20_SET2] = USB_KEY_F20,
    [KEY_F21_SET2] = USB_KEY_F21,
    [KEY_F22_SET2] = USB_KEY_F22,
    [KEY_F23_SET2] = USB_KEY_F23,
    [KEY_F24_SET2] = USB_KEY_F24,

    // Number row
    [KEY_BACKTICK] = USB_KEY_BACKTICK,
    [KEY_1] = USB_KEY_1,
    [KEY_2] = USB_KEY_2,
    [KEY_3] = USB_KEY_3,
    [KEY_4] = USB_KEY_4,
    [KEY_5] = USB_KEY_5,
    [KEY_6] = USB_KEY_6,
    [KEY_7] = USB_KEY_7,
    [KEY_8] = USB_KEY_8,
    [KEY_9] = USB_KEY_9,
    [KEY_0] = USB_KEY_0,
    [KEY_DASH] = USB_KEY_DASH,
    [KEY_EQUALS] = USB_KEY_EQUALS,
    [KEY_INT_LEFT_OF_BACKSPACE_SET2] = USB_KEY_INT_LEFT_OF_BACKSPACE,
    [KEY_BACKSPACE] = USB_KEY_BACKSPACE,

    // Top row
    [KEY_TAB] = USB_KEY_TAB,
    [KEY_Q] = USB_KEY_Q,
    [KEY_W] = USB_KEY_W,
    [KEY_E] = USB_KEY_E,
    [KEY_R] = USB_KEY_R,
    [KEY_T] = USB_KEY_T,
    [KEY_Y] = USB_KEY_Y,
    [KEY_U] = USB_KEY_U,
    [KEY_I] = USB_KEY_I,
    [KEY_O] = USB_KEY_O,
    [KEY_P] = USB_KEY_P,
    [KEY_OPEN_BRACKET] = USB_KEY_OPEN_BRACKET,
    [KEY_CLOSE_BRACKET] = USB_KEY_CLOSE_BRACKET,

    // Note: In set 2 both ISO and ANSI backslash share the same keycode so we
    // can't map them separately like in set 3. So we have to pick one (both
    // keys will work, they just can't be told apart).
#if !defined(ISO_LAYOUT) || (ISO_LAYOUT == 1)
    [KEY_INT_NEXT_TO_RETURN_SET2] = USB_KEY_INT_NEXT_TO_RETURN,
#else
    [KEY_ANSI_BACKSLASH_SET2] = USB_KEY_ANSI_BACKSLASH,
#endif
    [KEY_RETURN] = USB_KEY_RETURN,

    // Home row
    [KEY_CAPS_LOCK_SET2] = USB_KEY_CAPS_LOCK,
    [KEY_A] = USB_KEY_A,
    [KEY_S] = USB_KEY_S,
    [KEY_D] = USB_KEY_D,
    [KEY_F] = USB_KEY_F,
    [KEY_G] = USB_KEY_G,
    [KEY_H] = USB_KEY_H,
    [KEY_J] = USB_KEY_J,
    [KEY_K] = USB_KEY_K,
    [KEY_L] = USB_KEY_L,
    [KEY_SEMICOLON] = USB_KEY_SEMICOLON,
    [KEY_QUOTE] = USB_KEY_QUOTE,

    // Bottom row
    [KEY_LEFT_SHIFT] = USB_KEY_LEFT_SHIFT,
    [KEY_INT_NEXT_TO_LEFT_SHIFT_SET2] = USB_KEY_INT_NEXT_TO_LEFT_SHIFT,
    [KEY_Z] = USB_KEY_Z,
    [KEY_X] = USB_KEY_X,
    [KEY_C] = USB_KEY_C,
    [KEY_V] = USB_KEY_V,
    [KEY_B] = USB_KEY_B,
    [KEY_N] = USB_KEY_N,
    [KEY_M] = USB_KEY_M,
    [KEY_COMMA] = USB_KEY_COMMA,
    [KEY_PERIOD] = USB_KEY_PERIOD,
    [KEY_SLASH] = USB_KEY_SLASH,

    [KEY_INT_LEFT_OF_RIGHT_SHIFT] = USB_KEY_INT_LEFT_OF_RIGHT_SHIFT,
    [KEY_RIGHT_SHIFT] = USB_KEY_RIGHT_SHIFT,

    // Modifier row (right Ctrl/Alt and both GUI are extended keys in set 2)
    [KEY_LEFT_CTRL_SET2] = USB_KEY_LEFT_CTRL,
    [KEY_LEFT_ALT_SET2] = USB_KEY_LEFT_ALT,
    [KEY_SPACE] = USB_KEY_SPACE,

    // The tenkey block is almost all extended keys in set 2
    [KEY_SCROLL_LOCK_SET2] = USB_KEY_SCROLL_LOCK,

    // Keypad (KP Enter and KP / are extended keys in set 2)
    [KEY_NUM_LOCK_SET2] = USB_KEY_NUM_LOCK,
    [KEY_KP_MULTIPLY_SET2] = USB_KEY_KP_MULTIPLY,
    [KEY_KP_MINUS_SET2] = USB_KEY_KP_MINUS,
    [KEY_KP_PLUS_SET2] = USB_KEY_KP_PLUS,
    [KEY_KP_1_END] = USB_KEY_KP_1_END,
    [KEY_KP_2_DOWN] = USB_KEY_KP_2_DOWN,
    [KEY_KP_3_PAGE_DOWN] = USB_KEY_KP_3_PAGE_DOWN,
    [KEY_KP_4_LEFT] = USB_KEY_KP_4_LEFT,
    [KEY_KP_5] = USB_KEY_KP_5,
    [KEY_KP_6_RIGHT] = USB_KEY_KP_6_RIGHT,
    [KEY_KP_7_HOME] = USB_KEY_KP_7_HOME,
    [KEY_KP_8_UP] = USB_KEY_KP_8_UP,
    [KEY_KP_9_PAGE_UP] = USB_KEY_KP_9_PAGE_UP,
    [KEY_KP_0_INSERT] = USB_KEY_KP_0_INSERT,
    [KEY_KP_COMMA_DEL] = USB_KEY_KP_COMMA_DEL,
};

uint8_t
usb_keycode_for_ps2_extended_keycode (const uint8_t ps2_ext_code) {
    switch (ps2_ext_code) {
    case EXTENDED_KEY_RIGHT_ALT_SET2: return USB_KEY_RIGHT_ALT;
    case EXTENDED_KEY_RIGHT_CTRL_SET2: return USB_KEY_RIGHT_CTRL;
    case EXTENDED_KEY_LEFT_WIN_SET2: return USB_KEY_LEFT_WIN;
    case EXTENDED_KEY_RIGHT_WIN_SET2: return USB_KEY_RIGHT_WIN;
    case EXTENDED_KEY_MENU_SET2: return USB_KEY_MENU;
    case EXTENDED_KEY_POWER_SET2: return USB_KEY_POWER;
    case EXTENDED_KEY_KP_DIVIDE_SET2: return USB_KEY_KP_DIVIDE;
    case EXTENDED_KEY_KP_ENTER_SET2: return USB_KEY_KP_ENTER;
    case EXTENDED_KEY_KP_EQUALS_SET2: return USB_KEY_KP_EQUALS;
    case EXTENDED_KEY_END_SET2: return USB_KEY_END;
    case EXTENDED_KEY_LEFT_ARROW_SET2: return USB_KEY_LEFT_ARROW;
    case EXTENDED_KEY_HOME_SET2: return USB_KEY_HOME;
    case EXTENDED_KEY_INSERT_SET2: return USB_KEY_INSERT;
    case EXTENDED_KEY_DELETE_SET2: return USB_KEY_DELETE;
    case EXTENDED_KEY_DOWN_ARROW_SET2: return USB_KEY_DOWN_ARROW;
    case EXTENDED_KEY_RIGHT_ARROW_SET2: return USB_KEY_RIGHT_ARROW;
    case EXTENDED_KEY_UP_ARROW_SET2: return USB_KEY_UP_ARROW;
    case EXTENDED_KEY_PRINT_SCREEN_SET2: return USB_KEY_PRINT_SCREEN;
    case EXTENDED_KEY_PAGE_DOWN_SET2: return USB_KEY_PAGE_DOWN;
    case EXTENDED_KEY_CTRL_PAUSE_SET2: return USB_KEY_PAUSE_BREAK;
    case EXTENDED_KEY_PAGE_UP_SET2: return USB_KEY_PAGE_UP;
#if ENABLE_MEDIA_KEYS
    case EXTENDED_KEY_VOLUME_MUTE_SET2: return USB_KEY_VOLUME_MUTE;
    case EXTENDED_KEY_VOLUME_UP_SET2: return USB_KEY_VOLUME_UP;
    case EXTENDED_KEY_VOLUME_DOWN_SET2: return USB_KEY_VOLUME_DOWN;
    case EXTENDED_KEY_PLAY_PAUSE_SET2: return USB_KEY_PLAY_PAUSE;
    case EXTENDED_KEY_NEXT_TRACK_SET2: return USB_KEY_NEXT_TRACK;
    case EXTENDED_KEY_PREVIOUS_TRACK_SET2: return USB_KEY_PREVIOUS_TRACK;
#endif
    default: return 0;
    }
}

uint8_t
usb_keycode_for_ps2_keycode (const uint8_t ps2_code, const uint8_t scancode_set) {
    switch (scancode_set) {
        case 3:
            if (ps2_code < sizeof ps2_to_usb_set3) {
                return pgm_read_byte(ps2_to_usb_set3 + ps2_code);
            }
            return 0;
        case 2:
            if (ps2_code < sizeof ps2_to_usb_set2) {
                return pgm_read_byte(ps2_to_usb_set2 + ps2_code);
            }
            return 0;
        default:
            return 0;
    }
}
