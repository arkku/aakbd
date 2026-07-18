/**
 * layer_vial.c: Read-only layer definitions and list of macros.
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

#define FN_LAYER  STATIC_LAYER_1
#define FN_SPACE_LAYER  STATIC_LAYER_2
#define LAYER_COUNT     FN_SPACE_LAYER

enum macro {
    MACRO_LAYER_FN,
    MACRO_ESC_GRAVE,
    MACRO_DEBUG,
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

DEFINE_LAYER(FN_LAYER) {
    [KEY(TAB)] = KEY(CAPS_LOCK),

    [KEY(BACKTICK)] = MACRO(MACRO_ESC_GRAVE),

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
    [KEY(EQUALS)] = KEY(F12),

    [KEY(P)] = KEY(PRINT_SCREEN),
    [KEY(OPEN_BRACKET)] = KEY(SCROLL_LOCK),
    [KEY(CLOSE_BRACKET)] = KEY(PAUSE_BREAK),

    [KEY(Q)] = KEY(HOME),
    [KEY(W)] = KEY(UP_ARROW),
    [KEY(E)] = KEY(END),
    [KEY(R)] = KEY(PAGE_UP),

    [KEY(A)] = KEY(LEFT_ARROW),
    [KEY(S)] = KEY(DOWN_ARROW),
    [KEY(D)] = KEY(RIGHT_ARROW),
    [KEY(F)] = KEY(PAGE_DOWN),

    [KEY(ANSI_BACKSLASH)] = KEY(INSERT),
    [KEY(INT_NEXT_TO_RETURN)] = KEY(INSERT),
    [KEY(BACKSPACE)] = KEY(DELETE),

#if ENABLE_MEDIA_KEYS
    [KEY(COMMA)] = KEY(PREVIOUS_TRACK),
    [KEY(PERIOD)] = KEY(PLAY_PAUSE),
    [KEY(SLASH)] = KEY(NEXT_TRACK),

    [KEY(SEMICOLON)] = KEY(VOLUME_DOWN),
    [KEY(QUOTE)] = KEY(VOLUME_UP),
#endif

    [KEY(SPACE)] = LAYER_ON_HOLD(FN_SPACE_LAYER),
    [KEY(RETURN)] = MACRO(MACRO_READ_ONLY_LAYER),
};

DEFINE_LAYER(FN_SPACE_LAYER) {
    [KEY(ESC)] = EXT(ENTER_BOOTLOADER),
    [KEY(0)] = LAYER_SET_BASE(0),

    [KEY(W)] = MACRO(MACRO_UNSWAP_ALL),
    [KEY(E)] = EXT(EEPROM_RESET),
    [KEY(R)] = EXT(ENTER_BOOTLOADER),

    [KEY(X)] = EXT(RESET_KEYBOARD),

    [KEY(DVORAK_B)] = EXT(TOGGLE_BOOT_PROTOCOL),

    [KEY(SPACE)] = LAYER_TOGGLE(FN_SPACE_LAYER),
    [KEY(RETURN)] = MACRO(MACRO_READ_ONLY_LAYER),
};
