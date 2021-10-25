#ifndef KK_QMK_KEYMAP_H
#define KK_QMK_KEYMAP_H

#include <stdint.h>
#include "usb_keys.h"

uint8_t usb_keycode_for_matrix(const int8_t row, const int8_t column);

#define KC_NO   0

#define KC_EQL  USB_KEY_EQUALS
#define KC_1    USB_KEY_1
#define KC_2    USB_KEY_2
#define KC_3    USB_KEY_3
#define KC_4    USB_KEY_4
#define KC_5    USB_KEY_5
#define KC_6    USB_KEY_6
#define KC_7    USB_KEY_7
#define KC_8    USB_KEY_8
#define KC_9    USB_KEY_9
#define KC_0    USB_KEY_0
#define KC_DEL  USB_KEY_DELETE
#define KC_A    USB_KEY_A
#define KC_B    USB_KEY_B
#define KC_C    USB_KEY_C
#define KC_D    USB_KEY_D
#define KC_E    USB_KEY_E
#define KC_F    USB_KEY_F
#define KC_G    USB_KEY_G
#define KC_H    USB_KEY_H
#define KC_I    USB_KEY_I
#define KC_J    USB_KEY_J
#define KC_K    USB_KEY_K
#define KC_L    USB_KEY_L
#define KC_M    USB_KEY_M
#define KC_N    USB_KEY_N
#define KC_O    USB_KEY_O
#define KC_P    USB_KEY_P
#define KC_Q    USB_KEY_Q
#define KC_R    USB_KEY_R
#define KC_S    USB_KEY_S
#define KC_T    USB_KEY_T
#define KC_U    USB_KEY_U
#define KC_V    USB_KEY_V
#define KC_W    USB_KEY_W
#define KC_X    USB_KEY_X
#define KC_Y    USB_KEY_Y
#define KC_Z    USB_KEY_Z
#define KC_LEFT USB_KEY_LEFT_ARROW
#define KC_RGHT USB_KEY_RIGHT_ARROW
#define KC_RIGHT USB_KEY_RIGHT_ARROW
#define KC_UP   USB_KEY_UP_ARROW
#define KC_DOWN USB_KEY_DOWN_ARROW
#define KC_BSPC USB_KEY_BACKSPACE
#define KC_QUOT USB_KEY_QUOTE
#define KC_COMM USB_KEY_COMMA
#define KC_DOT  USB_KEY_PERIOD
#define KC_LBRC USB_KEY_OPEN_BRACKET
#define KC_RBRC USB_KEY_CLOSE_BRACKET
#define KC_SCLN USB_KEY_SEMICOLON
#define KC_LGUI USB_KEY_LEFT_CMD
#define KC_APP  USB_KEY_MENU
#define KC_MINS USB_KEY_DASH
#define KC_BSLS USB_KEY_BACKSLASH
#define KC_SLSH USB_KEY_SLASH
#define KC_GRV  USB_KEY_BACKTICK
#define KC_SPC  USB_KEY_SPACE
#define KC_ESC  USB_KEY_ESC
#define KC_PGUP USB_KEY_PAGE_UP
#define KC_PGDN USB_KEY_PAGE_DOWN
#define KC_TAB  USB_KEY_TAB
#define KC_ENT  USB_KEY_RETURN
#define KC_HOME USB_KEY_HOME
#define KC_END  USB_KEY_END
#define KC_LALT USB_KEY_LEFT_ALT
#define KC_RALT USB_KEY_RIGHT_ALT
#define KC_LSFT USB_KEY_LEFT_SHIFT
#define KC_RSFT USB_KEY_RIGHT_SHIFT
#define KC_INT1 USB_KEY_INT_NEXT_TO_LEFT_SHIFT
#define KC_CAPS USB_KEY_CAPS_LOCK
#define KC_SLCK USB_KEY_SCROLL_LOCK
#define KC_INS  USB_KEY_INSERT
#define KC_F1   USB_KEY_F1
#define KC_F2   USB_KEY_F2
#define KC_F3   USB_KEY_F3
#define KC_F4   USB_KEY_F4
#define KC_F5   USB_KEY_F5
#define KC_F6   USB_KEY_F6
#define KC_F7   USB_KEY_F7
#define KC_F8   USB_KEY_F8
#define KC_F9   USB_KEY_F9
#define KC_F10  USB_KEY_F10
#define KC_F11  USB_KEY_F11
#define KC_F12  USB_KEY_F12
#define KC_APFN USB_KEY_VIRTUAL_APPLE_FN
#define KC_P1       USB_KEY_KP_1
#define KC_P2       USB_KEY_KP_2
#define KC_P3       USB_KEY_KP_3
#define KC_P4       USB_KEY_KP_4
#define KC_P5       USB_KEY_KP_5
#define KC_P6       USB_KEY_KP_6
#define KC_P7       USB_KEY_KP_7
#define KC_P8       USB_KEY_KP_8
#define KC_P9       USB_KEY_KP_9
#define KC_P0       USB_KEY_KP_0
#define KC_PDOT     USB_KEY_KP_DOT
#define KC_LCTRL    USB_KEY_LEFT_CTRL
#define KC_LCTL     USB_KEY_LEFT_CTRL
#define KC_RCTL     USB_KEY_RIGHT_CTRL
#define KC_RCTRL    USB_KEY_RIGHT_CTRL
#define KC_NLCK     USB_KEY_NUM_LOCK
#define KC_NUBS     USB_KEY_INT_NEXT_TO_LEFT_SHIFT
#define KC_NUHS     USB_KEY_INT_NEXT_TO_RETURN

#endif
