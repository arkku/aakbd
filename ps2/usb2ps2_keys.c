/**
 * usb2ps2_keys.c: USB to PS/2 scancode lookup tables.
 *
 * Copyright (c) 2026 Kimmo Kulovesi, https://arkku.dev/
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
#define KK_KEYCODES_INCLUDE_DUPLICATES 1

#include <stdbool.h>
#include "progmem.h"
#include "usb_keys.h"
#include "usbkbd_config.h"
#include "ps2_keys.h"

static const uint8_t PROGMEM set2_table[] = {
    [USB_KEY_A] = KEY_A,
    [USB_KEY_B] = KEY_B,
    [USB_KEY_C] = KEY_C,
    [USB_KEY_D] = KEY_D,
    [USB_KEY_E] = KEY_E,
    [USB_KEY_F] = KEY_F,
    [USB_KEY_G] = KEY_G,
    [USB_KEY_H] = KEY_H,
    [USB_KEY_I] = KEY_I,
    [USB_KEY_J] = KEY_J,
    [USB_KEY_K] = KEY_K,
    [USB_KEY_L] = KEY_L,
    [USB_KEY_M] = KEY_M,
    [USB_KEY_N] = KEY_N,
    [USB_KEY_O] = KEY_O,
    [USB_KEY_P] = KEY_P,
    [USB_KEY_Q] = KEY_Q,
    [USB_KEY_R] = KEY_R,
    [USB_KEY_S] = KEY_S,
    [USB_KEY_T] = KEY_T,
    [USB_KEY_U] = KEY_U,
    [USB_KEY_V] = KEY_V,
    [USB_KEY_W] = KEY_W,
    [USB_KEY_X] = KEY_X,
    [USB_KEY_Y] = KEY_Y,
    [USB_KEY_Z] = KEY_Z,
    [USB_KEY_1] = KEY_1,
    [USB_KEY_2] = KEY_2,
    [USB_KEY_3] = KEY_3,
    [USB_KEY_4] = KEY_4,
    [USB_KEY_5] = KEY_5,
    [USB_KEY_6] = KEY_6,
    [USB_KEY_7] = KEY_7,
    [USB_KEY_8] = KEY_8,
    [USB_KEY_9] = KEY_9,
    [USB_KEY_0] = KEY_0,
    [USB_KEY_RETURN] = KEY_RETURN,
    [USB_KEY_ESC] = KEY_ESC_SET2,
    [USB_KEY_BACKSPACE] = KEY_BACKSPACE,
    [USB_KEY_TAB] = KEY_TAB,
    [USB_KEY_SPACE] = KEY_SPACE,
    [USB_KEY_DASH] = KEY_DASH,
    [USB_KEY_EQUALS] = KEY_EQUALS,
    [USB_KEY_OPEN_BRACKET] = KEY_OPEN_BRACKET,
    [USB_KEY_CLOSE_BRACKET] = KEY_CLOSE_BRACKET,
    [USB_KEY_ANSI_BACKSLASH] = KEY_ANSI_BACKSLASH_SET2,
    [USB_KEY_INT_NEXT_TO_RETURN] = KEY_INT_NEXT_TO_RETURN_SET2,
    [USB_KEY_SEMICOLON] = KEY_SEMICOLON,
    [USB_KEY_QUOTE] = KEY_QUOTE,
    [USB_KEY_BACKTICK] = KEY_BACKTICK,
    [USB_KEY_COMMA] = KEY_COMMA,
    [USB_KEY_PERIOD] = KEY_PERIOD,
    [USB_KEY_SLASH] = KEY_SLASH,
    [USB_KEY_CAPS_LOCK] = KEY_CAPS_LOCK_SET2,
    [USB_KEY_F1] = KEY_F1_SET2,
    [USB_KEY_F2] = KEY_F2_SET2,
    [USB_KEY_F3] = KEY_F3_SET2,
    [USB_KEY_F4] = KEY_F4_SET2,
    [USB_KEY_F5] = KEY_F5_SET2,
    [USB_KEY_F6] = KEY_F6_SET2,
    [USB_KEY_F7] = KEY_F7_SET2,
    [USB_KEY_F8] = KEY_F8_SET2,
    [USB_KEY_F9] = KEY_F9_SET2,
    [USB_KEY_F10] = KEY_F10_SET2,
    [USB_KEY_F11] = KEY_F11_SET2,
    [USB_KEY_F12] = KEY_F12_SET2,
    [USB_KEY_F13] = KEY_F13_SET2,
    [USB_KEY_F14] = KEY_F14_SET2,
    [USB_KEY_F15] = KEY_F15_SET2,
    [USB_KEY_F16] = KEY_F16_SET2,
    [USB_KEY_F17] = KEY_F17_SET2,
    [USB_KEY_F18] = KEY_F18_SET2,
    [USB_KEY_F19] = KEY_F19_SET2,
    [USB_KEY_F20] = KEY_F20_SET2,
    [USB_KEY_F21] = KEY_F21_SET2,
    [USB_KEY_F22] = KEY_F22_SET2,
    [USB_KEY_F23] = KEY_F23_SET2,
    [USB_KEY_F24] = KEY_F24_SET2,
    [USB_KEY_PRINT_SCREEN] = KEY_PRINT_SCREEN,
    [USB_KEY_SCROLL_LOCK] = KEY_SCROLL_LOCK_SET2,
    [USB_KEY_PAUSE_BREAK] = KEY_PAUSE_BREAK,
    [USB_KEY_INSERT] = EXTENDED_KEY_INSERT_SET2,
    [USB_KEY_HOME] = EXTENDED_KEY_HOME_SET2,
    [USB_KEY_PAGE_UP] = EXTENDED_KEY_PAGE_UP_SET2,
    [USB_KEY_DELETE] = EXTENDED_KEY_DELETE_SET2,
    [USB_KEY_END] = EXTENDED_KEY_END_SET2,
    [USB_KEY_PAGE_DOWN] = EXTENDED_KEY_PAGE_DOWN_SET2,
    [USB_KEY_RIGHT_ARROW] = EXTENDED_KEY_RIGHT_ARROW_SET2,
    [USB_KEY_LEFT_ARROW] = EXTENDED_KEY_LEFT_ARROW_SET2,
    [USB_KEY_DOWN_ARROW] = EXTENDED_KEY_DOWN_ARROW_SET2,
    [USB_KEY_UP_ARROW] = EXTENDED_KEY_UP_ARROW_SET2,
    [USB_KEY_NUM_LOCK] = KEY_NUM_LOCK_SET2,
    [USB_KEY_KP_DIVIDE] = EXTENDED_KEY_KP_DIVIDE_SET2,
    [USB_KEY_KP_MULTIPLY] = KEY_KP_MULTIPLY_SET2,
    [USB_KEY_KP_MINUS] = KEY_KP_MINUS_SET2,
    [USB_KEY_KP_PLUS] = KEY_KP_PLUS_SET2,
    [USB_KEY_KP_ENTER] = EXTENDED_KEY_KP_ENTER_SET2,
    [USB_KEY_KP_1_END] = KEY_KP_1_END,
    [USB_KEY_KP_2_DOWN] = KEY_KP_2_DOWN,
    [USB_KEY_KP_3_PAGE_DOWN] = KEY_KP_3_PAGE_DOWN,
    [USB_KEY_KP_4_LEFT] = KEY_KP_4_LEFT,
    [USB_KEY_KP_5] = KEY_KP_5,
    [USB_KEY_KP_6_RIGHT] = KEY_KP_6_RIGHT,
    [USB_KEY_KP_7_HOME] = KEY_KP_7_HOME,
    [USB_KEY_KP_8_UP] = KEY_KP_8_UP,
    [USB_KEY_KP_9_PAGE_UP] = KEY_KP_9_PAGE_UP,
    [USB_KEY_KP_0_INSERT] = KEY_KP_0_INSERT,
    [USB_KEY_KP_COMMA_DEL] = KEY_KP_COMMA_DEL,
    [USB_KEY_INT_NEXT_TO_LEFT_SHIFT] = KEY_INT_NEXT_TO_LEFT_SHIFT_SET2,
    [USB_KEY_INT_LEFT_OF_BACKSPACE] = KEY_INT_LEFT_OF_BACKSPACE_SET2,
    [USB_KEY_INT_LEFT_OF_RIGHT_SHIFT] = KEY_INT_LEFT_OF_RIGHT_SHIFT,
    [USB_KEY_MENU] = EXTENDED_KEY_MENU_SET2,
    [USB_KEY_POWER] = EXTENDED_KEY_POWER_SET2,
    [USB_KEY_KATAKANA] = KEY_KATAKANA,
    [USB_KEY_KANJI] = KEY_KANJI,
    [USB_KEY_HIRAGANA] = KEY_HIRAGANA,
    [USB_KEY_LEFT_CTRL] = KEY_LEFT_CTRL_SET2,
    [USB_KEY_LEFT_SHIFT] = KEY_LEFT_SHIFT,
    [USB_KEY_LEFT_ALT] = KEY_LEFT_ALT_SET2,
    [USB_KEY_LEFT_WIN] = EXTENDED_KEY_LEFT_WIN_SET2,
    [USB_KEY_RIGHT_CTRL] = EXTENDED_KEY_RIGHT_CTRL_SET2,
    [USB_KEY_RIGHT_SHIFT] = KEY_RIGHT_SHIFT,
    [USB_KEY_RIGHT_ALT] = EXTENDED_KEY_RIGHT_ALT_SET2,
    [USB_KEY_RIGHT_WIN] = EXTENDED_KEY_RIGHT_WIN_SET2,
};

static bool
is_extended_set2_key (const uint8_t key) {
    if (key >= USB_KEY_INSERT && key <= USB_KEY_UP_ARROW) {
        return true;
    }
    if (key >= USB_KEY_KP_DIVIDE && key <= USB_KEY_KP_ENTER) {
        return key == USB_KEY_KP_DIVIDE || key == USB_KEY_KP_ENTER;
    }
    switch (key) {
    case USB_KEY_MENU:
    case USB_KEY_POWER:
    case USB_KEY_LEFT_WIN:
    case USB_KEY_RIGHT_CTRL:
    case USB_KEY_RIGHT_ALT:
    case USB_KEY_RIGHT_WIN:
        return true;
    default:
        return false;
    }
}

// Set 2 to Set 1 scancode conversion table
#if ENABLE_PS2_DEVICE_SET_1
static const uint8_t PROGMEM set2_to_set1[] = {
    0xFF, 0x43, 0x41, 0x3F, 0x3D, 0x3B, 0x3C, 0x58, 0x64, 0x44, 0x42, 0x40, 0x3E, 0x0F, 0x29, 0x59,
    0x65, 0x38, 0x2A, 0x70, 0x1D, 0x10, 0x02, 0x5A, 0x66, 0x71, 0x2C, 0x1F, 0x1E, 0x11, 0x03, 0x5B,
    0x67, 0x2E, 0x2D, 0x20, 0x12, 0x05, 0x04, 0x5C, 0x68, 0x39, 0x2F, 0x21, 0x14, 0x13, 0x06, 0x5D,
    0x69, 0x31, 0x30, 0x23, 0x22, 0x15, 0x07, 0x5E, 0x6A, 0x72, 0x32, 0x24, 0x16, 0x08, 0x09, 0x5F,
    0x6B, 0x33, 0x25, 0x17, 0x18, 0x0B, 0x0A, 0x60, 0x6C, 0x34, 0x35, 0x26, 0x27, 0x19, 0x0C, 0x61,
    0x6D, 0x73, 0x28, 0x74, 0x1A, 0x0D, 0x62, 0x6E, 0x3A, 0x36, 0x1C, 0x1B, 0x75, 0x2B, 0x63, 0x76,
    0x55, 0x56, 0x77, 0x78, 0x79, 0x7A, 0x0E, 0x7B, 0x7C, 0x4F, 0x7D, 0x4B, 0x47, 0x7E, 0x7F, 0x6F,
    0x52, 0x53, 0x50, 0x4C, 0x4D, 0x48, 0x01, 0x45, 0x57, 0x4E, 0x51, 0x4A, 0x37, 0x49, 0x46, 0x54,
    0x80, 0x81, 0x82, 0x41, 0x54, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
};
#endif // ENABLE_PS2_DEVICE_SET_1

// Set 3
#if ENABLE_PS2_DEVICE_SET_3

static const uint8_t PROGMEM set3_table[] = {
    [USB_KEY_A] = KEY_A,
    [USB_KEY_B] = KEY_B,
    [USB_KEY_C] = KEY_C,
    [USB_KEY_D] = KEY_D,
    [USB_KEY_E] = KEY_E,
    [USB_KEY_F] = KEY_F,
    [USB_KEY_G] = KEY_G,
    [USB_KEY_H] = KEY_H,
    [USB_KEY_I] = KEY_I,
    [USB_KEY_J] = KEY_J,
    [USB_KEY_K] = KEY_K,
    [USB_KEY_L] = KEY_L,
    [USB_KEY_M] = KEY_M,
    [USB_KEY_N] = KEY_N,
    [USB_KEY_O] = KEY_O,
    [USB_KEY_P] = KEY_P,
    [USB_KEY_Q] = KEY_Q,
    [USB_KEY_R] = KEY_R,
    [USB_KEY_S] = KEY_S,
    [USB_KEY_T] = KEY_T,
    [USB_KEY_U] = KEY_U,
    [USB_KEY_V] = KEY_V,
    [USB_KEY_W] = KEY_W,
    [USB_KEY_X] = KEY_X,
    [USB_KEY_Y] = KEY_Y,
    [USB_KEY_Z] = KEY_Z,
    [USB_KEY_1] = KEY_1,
    [USB_KEY_2] = KEY_2,
    [USB_KEY_3] = KEY_3,
    [USB_KEY_4] = KEY_4,
    [USB_KEY_5] = KEY_5,
    [USB_KEY_6] = KEY_6,
    [USB_KEY_7] = KEY_7,
    [USB_KEY_8] = KEY_8,
    [USB_KEY_9] = KEY_9,
    [USB_KEY_0] = KEY_0,
    [USB_KEY_RETURN] = KEY_RETURN,
    [USB_KEY_ESC] = KEY_ESC,
    [USB_KEY_BACKSPACE] = KEY_BACKSPACE,
    [USB_KEY_TAB] = KEY_TAB,
    [USB_KEY_SPACE] = KEY_SPACE,
    [USB_KEY_DASH] = KEY_DASH,
    [USB_KEY_EQUALS] = KEY_EQUALS,
    [USB_KEY_OPEN_BRACKET] = KEY_OPEN_BRACKET,
    [USB_KEY_CLOSE_BRACKET] = KEY_CLOSE_BRACKET,
    [USB_KEY_ANSI_BACKSLASH] = KEY_ANSI_BACKSLASH,
    [USB_KEY_INT_NEXT_TO_RETURN] = KEY_INT_NEXT_TO_RETURN,
    [USB_KEY_SEMICOLON] = KEY_SEMICOLON,
    [USB_KEY_QUOTE] = KEY_QUOTE,
    [USB_KEY_BACKTICK] = KEY_BACKTICK,
    [USB_KEY_COMMA] = KEY_COMMA,
    [USB_KEY_PERIOD] = KEY_PERIOD,
    [USB_KEY_SLASH] = KEY_SLASH,
    [USB_KEY_CAPS_LOCK] = KEY_CAPS_LOCK,
    [USB_KEY_F1] = KEY_F1,
    [USB_KEY_F2] = KEY_F2,
    [USB_KEY_F3] = KEY_F3,
    [USB_KEY_F4] = KEY_F4,
    [USB_KEY_F5] = KEY_F5,
    [USB_KEY_F6] = KEY_F6,
    [USB_KEY_F7] = KEY_F7,
    [USB_KEY_F8] = KEY_F8,
    [USB_KEY_F9] = KEY_F9,
    [USB_KEY_F10] = KEY_F10,
    [USB_KEY_F11] = KEY_F11,
    [USB_KEY_F12] = KEY_F12,
    [USB_KEY_F13] = KEY_F13,
    [USB_KEY_F14] = KEY_F14,
    [USB_KEY_F15] = KEY_F15,
    [USB_KEY_F16] = KEY_F16,
    [USB_KEY_F17] = KEY_F17,
#if ENABLE_MEDIA_KEYS
    [USB_KEY_VOLUME_MUTE] = KEY_MUTE,
    [USB_KEY_VOLUME_UP] = KEY_VOLUME_UP,
    [USB_KEY_VOLUME_DOWN] = KEY_VOLUME_DOWN,
    [USB_KEY_NEXT_TRACK] = KEY_NEXT_TRACK,
    [USB_KEY_PREVIOUS_TRACK] = KEY_PREVIOUS_TRACK,
    [USB_KEY_STOP] = KEY_STOP,
#endif
    [USB_KEY_PRINT_SCREEN] = KEY_PRINT_SCREEN,
    [USB_KEY_SCROLL_LOCK] = KEY_SCROLL_LOCK,
    [USB_KEY_PAUSE_BREAK] = KEY_PAUSE_BREAK,
    [USB_KEY_INSERT] = KEY_INSERT,
    [USB_KEY_HOME] = KEY_HOME,
    [USB_KEY_PAGE_UP] = KEY_PAGE_UP,
    [USB_KEY_DELETE] = KEY_DELETE,
    [USB_KEY_END] = KEY_END,
    [USB_KEY_PAGE_DOWN] = KEY_PAGE_DOWN,
    [USB_KEY_RIGHT_ARROW] = KEY_RIGHT_ARROW,
    [USB_KEY_LEFT_ARROW] = KEY_LEFT_ARROW,
    [USB_KEY_DOWN_ARROW] = KEY_DOWN_ARROW,
    [USB_KEY_UP_ARROW] = KEY_UP_ARROW,
    [USB_KEY_NUM_LOCK] = KEY_NUM_LOCK,
    [USB_KEY_KP_DIVIDE] = KEY_KP_DIVIDE,
    [USB_KEY_KP_MULTIPLY] = KEY_KP_MULTIPLY,
    [USB_KEY_KP_MINUS] = KEY_KP_MINUS,
    [USB_KEY_KP_PLUS] = KEY_KP_PLUS,
    [USB_KEY_KP_ENTER] = KEY_KP_ENTER,
    [USB_KEY_KP_1_END] = KEY_KP_1_END,
    [USB_KEY_KP_2_DOWN] = KEY_KP_2_DOWN,
    [USB_KEY_KP_3_PAGE_DOWN] = KEY_KP_3_PAGE_DOWN,
    [USB_KEY_KP_4_LEFT] = KEY_KP_4_LEFT,
    [USB_KEY_KP_5] = KEY_KP_5,
    [USB_KEY_KP_6_RIGHT] = KEY_KP_6_RIGHT,
    [USB_KEY_KP_7_HOME] = KEY_KP_7_HOME,
    [USB_KEY_KP_8_UP] = KEY_KP_8_UP,
    [USB_KEY_KP_9_PAGE_UP] = KEY_KP_9_PAGE_UP,
    [USB_KEY_KP_0_INSERT] = KEY_KP_0_INSERT,
    [USB_KEY_KP_COMMA_DEL] = KEY_KP_COMMA_DEL,
    [USB_KEY_LEFT_CTRL] = KEY_LEFT_CTRL,
    [USB_KEY_LEFT_SHIFT] = KEY_LEFT_SHIFT,
    [USB_KEY_LEFT_ALT] = KEY_LEFT_ALT,
    [USB_KEY_LEFT_WIN] = KEY_LEFT_WIN,
    [USB_KEY_RIGHT_CTRL] = KEY_RIGHT_CTRL,
    [USB_KEY_RIGHT_SHIFT] = KEY_RIGHT_SHIFT,
    [USB_KEY_RIGHT_ALT] = KEY_RIGHT_ALT,
    [USB_KEY_RIGHT_WIN] = KEY_RIGHT_WIN,
    [USB_KEY_MENU] = KEY_MENU,
    [USB_KEY_POWER] = EXTENDED_KEY_POWER_SET2,
    [USB_KEY_KP_EQUALS] = KEY_KP_EQUALS,
    [USB_KEY_INT_NEXT_TO_LEFT_SHIFT] = KEY_INT_NEXT_TO_LEFT_SHIFT,
    [USB_KEY_INT_LEFT_OF_BACKSPACE] = KEY_INT_LEFT_OF_BACKSPACE,
    [USB_KEY_INT_LEFT_OF_RIGHT_SHIFT] = KEY_INT_LEFT_OF_RIGHT_SHIFT,
    [USB_KEY_KATAKANA] = KEY_KATAKANA,
    [USB_KEY_KANJI] = KEY_KANJI,
    [USB_KEY_HIRAGANA] = KEY_HIRAGANA,
};

#endif // ENABLE_PS2_DEVICE_SET_3

#if ENABLE_MEDIA_KEYS
static const uint8_t PROGMEM set2_media_scancodes[] = {
    [USB_KEY_VOLUME_MUTE - USB_KEY_VIRTUAL_MEDIA_1] = EXTENDED_KEY_VOLUME_MUTE_SET2,
    [USB_KEY_VOLUME_UP - USB_KEY_VIRTUAL_MEDIA_1] = EXTENDED_KEY_VOLUME_UP_SET2,
    [USB_KEY_VOLUME_DOWN - USB_KEY_VIRTUAL_MEDIA_1] = EXTENDED_KEY_VOLUME_DOWN_SET2,
    [USB_KEY_PLAY_PAUSE - USB_KEY_VIRTUAL_MEDIA_1] = EXTENDED_KEY_PLAY_PAUSE_SET2,
    [USB_KEY_NEXT_TRACK - USB_KEY_VIRTUAL_MEDIA_1] = EXTENDED_KEY_NEXT_TRACK_SET2,
    [USB_KEY_PREVIOUS_TRACK - USB_KEY_VIRTUAL_MEDIA_1] = EXTENDED_KEY_PREVIOUS_TRACK_SET2,
    [USB_KEY_FAST_FORWARD - USB_KEY_VIRTUAL_MEDIA_1] = EXTENDED_KEY_NEXT_TRACK_SET2,
#if MEDIA_KEYS_COUNT > 7
    [USB_KEY_REWIND - USB_KEY_VIRTUAL_MEDIA_1] = EXTENDED_KEY_PREVIOUS_TRACK_SET2,
#endif
};

#endif // ENABLE_MEDIA_KEYS

uint8_t
ps2_scancode_for_usb_keycode (const uint8_t key, const uint8_t set) {
    uint8_t result = 0;

#if ENABLE_PS2_DEVICE_SET_3
    if (set == 3) {
        if (key < sizeof set3_table) {
            result = pgm_read_byte(set3_table + key);
        }
    } else
#endif
    {
        if (key < sizeof set2_table) {
            result = pgm_read_byte(set2_table + key);
        }
#if ENABLE_PS2_DEVICE_SET_1
        if (set == 1 && result) {
            result = pgm_read_byte(set2_to_set1 + result);
        }
#endif
    }

#if ENABLE_MEDIA_KEYS
    if (!result && key >= USB_KEY_VIRTUAL_MEDIA_1 && key < USB_KEY_VIRTUAL_MEDIA_1 + MEDIA_KEYS_COUNT) {
        uint8_t media_idx = key - USB_KEY_VIRTUAL_MEDIA_1;
        if (media_idx < sizeof set2_media_scancodes) {
            result = pgm_read_byte(set2_media_scancodes + media_idx);
        }
#if ENABLE_PS2_DEVICE_SET_1
        if (set == 1 && result) {
            result = pgm_read_byte(set2_to_set1 + result);
        }
#endif
    }
#endif

    return result;
}

bool
is_extended_ps2_key (const uint8_t key, const uint8_t set) {
#if ENABLE_PS2_DEVICE_SET_3
    if (set != 3)
#endif
    {
        if (is_extended_set2_key(key)) {
            return true;
        }
    }

#if ENABLE_MEDIA_KEYS
    if (key >= USB_KEY_VIRTUAL_MEDIA_1 && key < USB_KEY_VIRTUAL_MEDIA_1 + MEDIA_KEYS_COUNT) {
        return true;
    }
#endif

    return false;
}
