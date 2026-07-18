/**
 * layer_vial.c: Read-only layer definitions and the list of macros.
 *
 * https://github.com/arkku/aakbd
 *
 * The local file layers_vial.c is ignored by Git so customisation can be
 * done there without being overwritten. The file template_layers_vial.c
 * contains an example set of configuration.
 *
 * There shouldn't be much need to edit this, though: you can edit other
 * layers through Vial, these are just "free extras" (they do not consume
 * EEPROM space so you can have more layers total).
 */
#include <layers.h>

#include "device_keymap.h"

#define ERGODOX_LAYER STATIC_LAYER_1
#define FN_LAYER  STATIC_LAYER_2
#define MEDIA_LAYER  STATIC_LAYER_3
#define LAYER_COUNT MEDIA_LAYER

enum macro {
    MACRO_LAYER_FN,
    MACRO_ESC_GRAVE,
    MACRO_LED_BRIGHT,
    MACRO_LED_DIM,
    MACRO_UNSWAP_ALL,

    // Note: All macros below this point are "markers" without special action!
    MACRO_READ_ONLY_LAYER,
    MACRO_NUM_LED_ON,
    MACRO_NUM_LED_OFF,
    MACRO_CAPS_LED_ON,
    MACRO_SCROLL_LED_ON,
#if ENABLE_HOST_FINGERPRINT
    MACRO_WIN_LAYER,
    MACRO_MAC_LAYER,
    MACRO_LINUX_LAYER,
#endif
#if ENABLE_PS2_DEVICE
    MACRO_PS2_LAYER,
#endif
    COUNT_OF_MACROS,
};

// Ergodox EZ default layer
DEFINE_LAYER(ERGODOX_LAYER) {
    // Left hand
    [KC_R1C1L] = KEY(EQUALS),
    [KC_R1C2L] = KEY(1),
    [KC_R1C3L] = KEY(2),
    [KC_R1C4L] = KEY(3),
    [KC_R1C5L] = KEY(4),
    [KC_R1C6L] = KEY(5),
    [KC_R1C7L] = KEY(LEFT),

    [KC_R2C1L] = KEY(DELETE),
    [KC_R2C2L] = KEY(Q),
    [KC_R2C3L] = KEY(W),
    [KC_R2C4L] = KEY(E),
    [KC_R2C5L] = KEY(R),
    [KC_R2C6L] = KEY(T),
    [KC_R2C7L] = LAYER_TOGGLE(FN_LAYER),

    [KC_R3C1L] = KEY(BACKSPACE),
    [KC_R3C2L] = KEY(A),
    [KC_R3C3L] = KEY(S),
    [KC_R3C4L] = KEY(D),
    [KC_R3C5L] = KEY(F),
    [KC_R3C6L] = KEY(G),

    [KC_R4C1L] = KEY(LEFT_SHIFT),
    [KC_R4C2L] = CTRL_OR(Z),
    [KC_R4C3L] = KEY(X),
    [KC_R4C4L] = KEY(C),
    [KC_R4C5L] = KEY(V),
    [KC_R4C6L] = KEY(B),
    [KC_R4C7L] = KEY_HYPER,

    [KC_R5C1L] = LAYER_OR_PLAIN_KEY(FN_LAYER, KEY(BACKTICK)),
    [KC_R5C2L] = KEY(QUOTE),
    [KC_R5C3L] = SHIFT(LEFT_ALT),
    [KC_R5C4L] = KEY(LEFT),
    [KC_R5C5L] = KEY(RIGHT),

    [KC_R6C1L] = ALT_OR(MENU),
    [KC_R6C2L] = KEY(LEFT_GUI),

    [KC_R7C1L] = KEY(HOME),

    [KC_R8C1L] = MACRO(MACRO_READ_ONLY_LAYER),
    [KC_R8C2L] = KEY(BACKSPACE),
    [KC_R8C3L] = KEY(END),

    // Right hand
    [KC_R1C1R] = KEY(RIGHT),
    [KC_R1C2R] = KEY(6),
    [KC_R1C3R] = KEY(7),
    [KC_R1C4R] = KEY(8),
    [KC_R1C5R] = KEY(9),
    [KC_R1C6R] = KEY(0),
    [KC_R1C7R] = KEY(DASH),

    [KC_R2C1R] = LAYER_TOGGLE(FN_LAYER),
    [KC_R2C2R] = KEY(Y),
    [KC_R2C3R] = KEY(U),
    [KC_R2C4R] = KEY(I),
    [KC_R2C5R] = KEY(O),
    [KC_R2C6R] = KEY(P),
    [KC_R2C7R] = KEY(BACKSLASH),

    [KC_R3C2R] = KEY(H),
    [KC_R3C3R] = KEY(J),
    [KC_R3C4R] = KEY(K),
    [KC_R3C5R] = KEY(L),
    [KC_R3C6R] = LAYER_OR_PLAIN_KEY(MEDIA_LAYER, KEY(SEMICOLON)),
    [KC_R3C7R] = CMD_OR(QUOTE),

    [KC_R4C1R] = KEY_MEH,
    [KC_R4C2R] = KEY(N),
    [KC_R4C3R] = KEY(M),
    [KC_R4C4R] = KEY(COMMA),
    [KC_R4C5R] = KEY(PERIOD),
    [KC_R4C6R] = CTRL_OR(SLASH),
    [KC_R4C7R] = KEY(RIGHT_SHIFT),

    [KC_R5C3R] = KEY(UP_ARROW),
    [KC_R5C4R] = KEY(DOWN_ARROW),
    [KC_R5C5R] = KEY(OPEN_BRACKET),
    [KC_R5C6R] = KEY(CLOSE_BRACKET),
    [KC_R5C7R] = LAYER_TOGGLE_STICKY(FN_LAYER),

    [KC_R6C1R] = KEY(LEFT_ALT),
    [KC_R6C2R] = CTRL_OR(ESC),

    [KC_R7C1R] = KEY(PAGE_UP),

    [KC_R8C1R] = KEY(PAGE_DOWN),
    [KC_R8C2R] = KEY(TAB),
    [KC_R8C3R] = KEY(RETURN),
};

DEFINE_LAYER(FN_LAYER) {
    [KC_R1C1L] = MACRO(MACRO_ESC_GRAVE),
    [KC_R1C2L] = KEY(F1),
    [KC_R1C3L] = KEY(F2),
    [KC_R1C4L] = KEY(F3),
    [KC_R1C5L] = KEY(F4),
    [KC_R1C6L] = KEY(F5),

    [KC_R1C2R] = KEY(F6),
    [KC_R1C3R] = KEY(F7),
    [KC_R1C4R] = KEY(F8),
    [KC_R1C5R] = KEY(F9),
    [KC_R1C6R] = KEY(F10),
    [KC_R1C7R] = KEY(F11),
    [KC_R2C7R] = KEY(F12),

    [KC_R2C2L] = SHIFT(1),
    [KC_R2C3L] = SHIFT(2),
#if DVORAK_MAPPINGS
    [KC_R2C4L] = SHIFT(DVORAK_OPEN_BRACKET),
    [KC_R2C5L] = SHIFT(DVORAK_CLOSE_BRACKET),
#else
    [KC_R2C4L] = SHIFT(OPEN_BRACKET),
    [KC_R2C5L] = SHIFT(CLOSE_BRACKET),
#endif
    [KC_R2C6L] = SHIFT(ANSI_BACKSLASH),

    [KC_R3C2L] = SHIFT(3),
    [KC_R3C3L] = SHIFT(4),
    [KC_R3C4L] = SHIFT(9),
    [KC_R3C5L] = SHIFT(0),
    [KC_R3C6L] = KEY(BACKTICK),

    [KC_R4C2L] = SHIFT(5),
    [KC_R4C3L] = SHIFT(6),
#if DVORAK_MAPPINGS
    [KC_R4C4L] = KEY(DVORAK_OPEN_BRACKET),
    [KC_R4C5L] = KEY(DVORAK_CLOSE_BRACKET),
#else
    [KC_R4C4L] = KEY(OPEN_BRACKET),
    [KC_R4C5L] = KEY(CLOSE_BRACKET),
#endif
    [KC_R4C6L] = SHIFT(BACKTICK),

    [KC_R5C2L] = KEY(EQUALS),

    [KC_R2C2R] = KEY(UP_ARROW),
    [KC_R3C2R] = KEY(DOWN_ARROW),
    [KC_R4C2R] = SHIFT(7),

    [KC_R2C3R] = KEY(KP_7),
    [KC_R2C4R] = KEY(KP_8),
    [KC_R2C5R] = KEY(KP_9),
    [KC_R2C6R] = KEY(KP_MULTIPLY),

    [KC_R3C3R] = KEY(KP_4),
    [KC_R3C4R] = KEY(KP_5),
    [KC_R3C5R] = KEY(KP_6),
    [KC_R3C6R] = KEY(KP_PLUS),

    [KC_R4C3R] = KEY(KP_1),
    [KC_R4C4R] = KEY(KP_2),
    [KC_R4C5R] = KEY(KP_3),
    [KC_R4C6R] = KEY(KP_DIVIDE),

    [KC_R5C3R] = KEY(KP_0),
    [KC_R5C4R] = KEY(KP_COMMA),
    [KC_R5C5R] = KEY(KP_EQUALS),
    [KC_R5C6R] = KEY(KP_MINUS),

    [KC_R8C1L] = MACRO(MACRO_READ_ONLY_LAYER),
};

DEFINE_LAYER(MEDIA_LAYER) {
    [KC_R1C7R] = EXT(ENTER_BOOTLOADER),
    [KC_R1C7R] = LAYER_SET_BASE(1),

    [KC_R2C2L] = KEY(HOME),
    [KC_R2C3L] = KEY(UP_ARROW),
    [KC_R2C4L] = KEY(END),
    [KC_R2C5L] = KEY(PAGE_UP),

    [KC_R3C2L] = KEY(LEFT_ARROW),
    [KC_R3C3L] = KEY(DOWN_ARROW),
    [KC_R3C4L] = KEY(RIGHT_ARROW),
    [KC_R3C5L] = KEY(PAGE_DOWN),

#if ENABLE_MEDIA_KEYS
    [KC_R3C7R] = KEY(PLAY_PAUSE),

    [KC_R4C4R] = KEY(PREVIOUS_TRACK),
    [KC_R4C5R] = KEY(NEXT_TRACK),

    [KC_R5C3R] = KEY(VOLUME_UP),
    [KC_R5C4R] = KEY(VOLUME_DOWN),
    [KC_R5C5R] = KEY(VOLUME_MUTE),

    [KC_R8C3R] = KEY(BROWSE_BACK),
#endif

    [KC_R8C1L] = MACRO(MACRO_READ_ONLY_LAYER),
};

