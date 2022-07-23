/**
 * layers.c: Layer definitions for advanced key mapping.
 *
 * https://github.com/arkku/aakbd
 *
 * The local file layers.c is ignored by Git so customisation can be done there
 * without being overwritten. The file template_layers.c contains an example
 * layers template.
 *
 * See the file "keycodes.h" and "usb_keys.h" for the available keycodes
 * you can use. The format is `[physical_key] = keycode,`.  The physical key
 * must always be a plain keycode (from "usb_keys.h") or using the helper
 * macro `KEY()`.
 */
#include <layers.h>

#include "device_keymap.h"

/// The number of layers to make active. The layer numbering starts from 1,
/// so this is also the number of the highest layer. Any layer with a number
/// higher than this will be unused, i.e., setting `LAYER_COUNT 0` will
/// ignore all layers defined below. The maximum layer count is 31.
#define LAYER_COUNT 2

/// You can define helper macros to name layers.
#define SYMBOL_LAYER 2

/// Recognised macro names, see `macros.c`. To define a macro, add the name
/// here, e.g., `MACRO_MY_MACRO`, and then map `MACRO(MACRO_MY_MACRO)` to a
/// key. Remember to use the `MACRO()` wrapper, do not use the macro name
/// directly as a keycode! You can have up to 127 macros.
enum macro {
    MACRO_NOP,
    MACRO_FALLTHROUGH,
    MACRO_ERGODOX_LED_LEVEL_UP,
    MACRO_ERGODOX_LED_LEVEL_DOWN,
};

// MARK: - Layer 1

#if LAYER_COUNT >= 1
/// Layer 1 is the default base layer. Only the differences to the default
/// mapping need to be defined here.
DEFINE_LAYER(1) {
    [KEY(ERGODOX_HYPER)]            = EXT(HYPER),
    [KEY(ERGODOX_MEH)]              = EXT(MEH),
    [KEY(ERGODOX_LEFT_BIG_LAYER)]   = LAYER_TOGGLE_STICKY(SYMBOL_LAYER),
    [KEY(ERGODOX_RIGHT_BIG_LAYER)]  = LAYER_TOGGLE_STICKY(SYMBOL_LAYER),
    [KEY(ERGODOX_TOP_LEFT_ARROW)]   = KEY(LEFT_ARROW),
    [KEY(ERGODOX_TOP_RIGHT_ARROW)]  = KEY(RIGHT_ARROW),
    [KEY(ERGODOX_ALT_APPS)]         = ALT_OR(MENU),
    [KEY(ERGODOX_ALT_SHIFT)]        = ALT(LEFT_SHIFT),

    [KEY(Z)]                        = CTRL_OR(Z),
    [KEY(SLASH)]                    = RIGHT_CTRL_OR(SLASH),

    [KEY(ESC)]                      = CTRL_OR(ESC),

#ifdef MEDIA_LAYER
    // AAKBD does not currently implement mouse or media keys
    [KEY(SEMICOLON)]                = LAYER_OR_PLAIN_KEY(MEDIA_LAYER, KEY(SEMICOLON)),
#elif !DVORAK_MAPPINGS
    [KEY(SEMICOLON)]                = ALTGR_OR(SEMICOLON),
#endif

#if ENABLE_APPLE_FN_KEY
    // Apple reverses these two keys on their keyboards
#define USB_KEY_ACTUAL_BACKTICK     USB_KEY_INT_NEXT_TO_LEFT_SHIFT
#define USB_KEY_ACTUAL_INT1         USB_KEY_BACKTICK
    [KEY(ERGODOX_RIGHT_LAYER)]      = KEY(VIRTUAL_APPLE_FN),
#else
#define USB_KEY_ACTUAL_BACKTICK     USB_KEY_BACKTICK
#define USB_KEY_ACTUAL_INT1         USB_KEY_INT_NEXT_TO_LEFT_SHIFT
    [KEY(ERGODOX_RIGHT_LAYER)]      = LAYER_ON_HOLD(SYMBOL_LAYER),
#endif

#if DVORAK_MAPPINGS
    [KEY(OPEN_BRACKET)]             = KEY(DVORAK_OPEN_BRACKET),
    [KEY(CLOSE_BRACKET)]            = KEY(DVORAK_CLOSE_BRACKET),
    [KEY(DASH)]                     = KEY(DVORAK_DASH),
    [KEY(ERGODOX_GRAVE_LAYER)]      = LAYER_OR_PLAIN_KEY(SYMBOL_LAYER, KEY(ACTUAL_INT1)),
    [KEY(EQUALS)]                   = KEY(ACTUAL_BACKTICK),
    [KEY(ERGODOX_ALT_QUOTE)]        = ALTGR_OR(DVORAK_EQUALS),
    [KEY(BACKSLASH)]                = KEY(DVORAK_SLASH),
    [KEY(QUOTE)]                    = CTRL_OR(BACKSLASH),
#else
    [KEY(QUOTE)]                    = CTRL_OR(QUOTE),
    [KEY(ERGODOX_ALT_QUOTE)]        = ALT_OR(ACTUAL_INT1),
    [KEY(ERGODOX_GRAVE_LAYER)]      = LAYER_OR_PLAIN_KEY(SYMBOL_LAYER, KEY(ACTUAL_BACKTICK)),
#endif
};
#endif

// MARK: - Layer 2

#if LAYER_COUNT >= 2
DEFINE_LAYER(SYMBOL_LAYER) {
    [KEY(EQUALS)] = KEY(ESC),

    [KEY(1)] = KEY(F1),
    [KEY(2)] = KEY(F2),
    [KEY(3)] = KEY(F3),
    [KEY(4)] = KEY(F4),
    [KEY(5)] = KEY(F5),
    [KEY(6)] = KEY(F6),
    [KEY(7)] = KEY(F7),
    [KEY(8)] = KEY(F8),
    [KEY(9)] = KEY(F9),
    [KEY(0)] = KEY(F10),
    [KEY(DASH)] = KEY(F11),
    [KEY(BACKSLASH)] = KEY(F12),

    [KEY(Q)] = SHIFT(1),
    [KEY(W)] = SHIFT(2),
    [KEY(T)] = SHIFT(BACKSLASH),
    [KEY(A)] = SHIFT(3),
    [KEY(S)] = SHIFT(4),
    [KEY(D)] = SHIFT(9),
    [KEY(F)] = SHIFT(0),
    [KEY(G)] = KEY(ACTUAL_BACKTICK),
    [KEY(Z)] = SHIFT(5),
    [KEY(X)] = SHIFT(6),
    [KEY(P)] = SHIFT(8),
    [KEY(SLASH)] = KEY(BACKSLASH),
    [KEY(Y)] = KEY(UP_ARROW),
    [KEY(H)] = KEY(DOWN_ARROW),
    [KEY(B)] = SHIFT(ACTUAL_BACKTICK),
    [KEY(N)] = SHIFT(7),
    [KEY(U)] = KEY(7),
    [KEY(I)] = KEY(8),
    [KEY(O)] = KEY(9),
    [KEY(J)] = KEY(4),
    [KEY(K)] = KEY(5),
    [KEY(L)] = KEY(6),
    [KEY(M)] = KEY(1),
    [KEY(COMMA)] = KEY(2),
    [KEY(PERIOD)] = KEY(3),
    [KEY(OPEN_BRACKET)] = KEY(0),

    [KEY(BACKSPACE)] = KEY(DELETE),
    [KEY(RETURN)] = KEY(KP_ENTER),

    [KEY(BACKTICK)] = EXT(ENTER_BOOTLOADER),

    [KEY(ERGODOX_ALT_APPS)] = MACRO(MACRO_ERGODOX_LED_LEVEL_DOWN),
    [KEY(LEFT_CMD)] = MACRO(MACRO_ERGODOX_LED_LEVEL_UP),

    [KEY(RIGHT_SHIFT)] = KEY(CAPS_LOCK),

    [KEY(ERGODOX_RIGHT_LAYER)] = EXT(RESET_LAYERS),

#if DVORAK_MAPPINGS
    [KEY(E)] = SHIFT(DVORAK_OPEN_BRACKET),
    [KEY(R)] = SHIFT(DVORAK_CLOSE_BRACKET),
    [KEY(C)] = KEY(DVORAK_OPEN_BRACKET),
    [KEY(V)] = KEY(DVORAK_CLOSE_BRACKET),
    [KEY(SEMICOLON)] = SHIFT(DVORAK_EQUALS),
    [KEY(UP_ARROW)] = KEY(DVORAK_COMMA),
    [KEY(DOWN_ARROW)] = KEY(DVORAK_PERIOD),
    [KEY(CLOSE_BRACKET)] = KEY(DVORAK_EQUALS),
#else
    [KEY(E)] = SHIFT(OPEN_BRACKET),
    [KEY(R)] = SHIFT(CLOSE_BRACKET),
    [KEY(C)] = KEY(OPEN_BRACKET),
    [KEY(V)] = KEY(CLOSE_BRACKET),
    [KEY(SEMICOLON)] = SHIFT(EQUALS),
    [KEY(UP_ARROW)] = KEY(COMMA),
    [KEY(DOWN_ARROW)] = KEY(PERIOD),
    [KEY(CLOSE_BRACKET)] = KEY(EQUALS),
#endif
};
#endif

// MARK: - Layer 3

#if LAYER_COUNT >= 3
DEFINE_LAYER(3) {
    NONE,
};
#endif

// MARK: - Layer 4

#if LAYER_COUNT >= 4
DEFINE_LAYER(4) {
    NONE,
};
#endif

// MARK: - Layer 5

#if LAYER_COUNT >= 5
DEFINE_LAYER(5) {
    NONE,
};
#endif

// MARK: - Layer 6

#if LAYER_COUNT >= 6
DEFINE_LAYER(6) {
    NONE,
};
#endif

// MARK: - Layer 7

#if LAYER_COUNT >= 7
DEFINE_LAYER(7) {
    NONE,
};
#endif

// MARK: - Layer 8

#if LAYER_COUNT >= 8
DEFINE_LAYER(8) {
    NONE,
};
#endif

// MARK: - Layer 9

#if LAYER_COUNT >= 9
DEFINE_LAYER(9) {
    NONE,
};
#endif

// MARK: - Layer 10

#if LAYER_COUNT >= 10
DEFINE_LAYER(10) {
    NONE,
};
#endif

// MARK: - Layer 11

#if LAYER_COUNT >= 11
DEFINE_LAYER(11) {
    NONE,
};
#endif

// MARK: - Layer 12

#if LAYER_COUNT >= 12
DEFINE_LAYER(12) {
    NONE,
};
#endif

// MARK: - Layer 13

#if LAYER_COUNT >= 13
DEFINE_LAYER(13) {
    NONE,
};,
#endif

// MARK: - Layer 14

#if LAYER_COUNT >= 14
DEFINE_LAYER(14) {
    NONE,
};
#endif

// MARK: - Layer 15

#if LAYER_COUNT >= 15
DEFINE_LAYER(15) {
    NONE,
};
#endif

// MARK: - Layer 16

#if LAYER_COUNT >= 16
DEFINE_LAYER(16) {
    NONE,
};
#endif
