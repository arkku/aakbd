/**
 * ps2_keys.h: PS/2 set 3 keycodes.
 */

#ifndef KK_PS2_KEYS_H
#define KK_PS2_KEYS_H

/// PS/2 scan code set 3 (and most of set 2, there is plenty of overlap).
enum keycode_ps2 {
    // Function keys
    KEY_ESC = 0x08,
    KEY_F1 = 0x07,
    KEY_F2 = 0x0F,
    KEY_F3 = 0x17,
    KEY_F4 = 0x1F,
    KEY_F5 = 0x27,
    KEY_F6 = 0x2F,
    KEY_F7 = 0x37,
    KEY_F8 = 0x3F,
    KEY_F9 = 0x47,
    KEY_F10 = 0x4F,
    KEY_F11 = 0x56,
    KEY_F12 = 0x5E,

    // Number row
    KEY_BACKTICK = 0x0E,
    KEY_1 = 0x16,
    KEY_2 = 0x1E,
    KEY_3 = 0x26,
    KEY_4 = 0x25,
    KEY_5 = 0x2E,
    KEY_6 = 0x36,
    KEY_7 = 0x3D,
    KEY_8 = 0x3E,
    KEY_9 = 0x46,
    KEY_0 = 0x45,
    KEY_DASH = 0x4E,
    KEY_EQUALS = 0x55,
    KEY_BACKSPACE = 0x66,

    // Top row
    KEY_TAB = 0x0D,
    KEY_Q = 0x15,
    KEY_W = 0x1D,
    KEY_E = 0x24,
    KEY_R = 0x2D,
    KEY_T = 0x2C,
    KEY_Y = 0x35,
    KEY_U = 0x3C,
    KEY_I = 0x43,
    KEY_O = 0x44,
    KEY_P = 0x4D,
    KEY_OPEN_BRACKET = 0x54,
    KEY_CLOSE_BRACKET = 0x5B,

    // The key that is covered by the upper row of the ISO return
    KEY_ANSI_BACKSLASH = 0x5C,

    KEY_RETURN = 0x5A,

    // Home row
    KEY_CAPS_LOCK = 0x14,
    KEY_A = 0x1C,
    KEY_S = 0x1B,
    KEY_D = 0x23,
    KEY_F = 0x2B,
    KEY_G = 0x34,
    KEY_H = 0x33,
    KEY_J = 0x3B,
    KEY_K = 0x42,
    KEY_L = 0x4B,
    KEY_SEMICOLON = 0x4C,
    KEY_QUOTE = 0x52,
    KEY_INT_NEXT_TO_RETURN = 0x53,

    // Bottom row
    KEY_LEFT_SHIFT = 0x12,
    KEY_INT_NEXT_TO_LEFT_SHIFT = 0x13,
    KEY_Z = 0x1A,
    KEY_X = 0x22,
    KEY_C = 0x21,
    KEY_V = 0x2A,
    KEY_B = 0x32,
    KEY_N = 0x31,
    KEY_M = 0x3A,
    KEY_COMMA = 0x41,
    KEY_PERIOD = 0x49,
    KEY_SLASH = 0x4A,
    KEY_RIGHT_SHIFT = 0x59,

    // Modifier keys and space
    KEY_LEFT_CTRL = 0x11,
    KEY_LEFT_ALT = 0x19,
    KEY_SPACE = 0x29,
    KEY_RIGHT_ALT = 0x39,
    KEY_RIGHT_CTRL = 0x58,

    // Scroll lock block
    KEY_PRINT_SCREEN = 0x57,
    KEY_SCROLL_LOCK = 0x5F,
    KEY_PAUSE_BREAK = 0x62,

    // Tenkey block
    KEY_INSERT = 0x67,
    KEY_DELETE = 0x64,
    KEY_HOME = 0x6E,
    KEY_END = 0x65,
    KEY_PAGE_UP = 0x6F,
    KEY_PAGE_DOWN = 0x6D,

    KEY_UP_ARROW = 0x63,
    KEY_LEFT_ARROW = 0x61,
    KEY_DOWN_ARROW = 0x60,
    KEY_RIGHT_ARROW = 0x6A,

    // Keypad
    KEY_NUM_LOCK = 0x76,
    KEY_KP_DIVIDE = 0x77,
    KEY_KP_MULTIPLY = 0x7E,
    KEY_KP_MINUS = 0x84,
    KEY_KP_7_HOME = 0x6C,
    KEY_KP_8_UP = 0x75,
    KEY_KP_9_PAGE_UP = 0x7D,
    KEY_KP_PLUS = 0x7C,
    KEY_KP_4_LEFT = 0x6B,
    KEY_KP_5 = 0x73,
    KEY_KP_6_RIGHT = 0x74,
    KEY_KP_1_END = 0x69,
    KEY_KP_2_DOWN = 0x72,
    KEY_KP_3_PAGE_DOWN = 0x7A,
    KEY_KP_ENTER = 0x79,
    KEY_KP_0_INSERT = 0x70,
    KEY_KP_COMMA_DEL = 0x71,

    // Extra spaces (not populated on ISO keyboards, but can be modified)
    KEY_INT_LEFT_OF_RIGHT_SHIFT = 0x51,
    KEY_INT_LEFT_OF_BACKSPACE = 0x5D,

    // Windows keys
    KEY_LEFT_WIN = 0x8B,
    KEY_RIGHT_WIN = 0x8C,
    KEY_MENU = 0x8D,

    // Additional keys
    KEY_KATAKANA = 0x8A,
    KEY_KANJI = 0x86,
    KEY_HIRAGANA = 0x85,

    // Scancode set 2 codes, where different and not prefixed by E0
#ifdef KK_KEYCODES_INCLUDE_DUPLICATES
    KEY_ANSI_BACKSLASH_SET2 = 0x5D,     // Set 3: KEY_INT_LEFT_OF_BACKSPACE
    KEY_CAPS_LOCK_SET2 = 0x58,          // Set 3: KEY_RIGHT_CTRL
    KEY_LEFT_CTRL_SET2 = 0x14,          // Set 3: KEY_CAPS_LOCK
    KEY_LEFT_ALT_SET2 = 0x11,           // Set 3: KEY_LEFT_CTRL
    KEY_NUM_LOCK_SET2 = 0x77,           // Set 3: KEY_KP_DIVIDE
    KEY_KP_MULTIPLY_SET2 = 0x7C,        // Set 3: KEY_KP_PLUS
    KEY_KP_PLUS_SET2 = 0x79,            // Set 3: KEY_KP_ENTER
    KEY_SCROLL_LOCK_SET2 = 0x7E,        // Set 3: KEY_KP_MULTIPLY
    KEY_ALT_SYSRQ_SET2 = 0x84,          // Set 3: KEY_KP_MINUS
    KEY_ESC_SET2 = 0x76,                // Set 3: KEY_NUM_LOCK
    KEY_F12_SET2 = 0x07,                // Set 3: KEY_F1
#endif
    KEY_F1_SET2 = 0x05,
    KEY_F2_SET2 = 0x06,
    KEY_F3_SET2 = 0x04,
    KEY_F4_SET2 = 0x0C,
    KEY_F5_SET2 = 0x03,
    KEY_F6_SET2 = 0x0B,
    KEY_F7_SET2 = 0x83,
    KEY_F8_SET2 = 0x0A,
    KEY_F9_SET2 = 0x01,
    KEY_F10_SET2 = 0x09,
    KEY_F11_SET2 = 0x78,
    KEY_KP_MINUS_SET2 = 0x7B
};

#endif
