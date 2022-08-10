#include <layers.h>
#include "keymap.h"

#define DEFAULT_BASE_LAYER 1
#define APPLE_FN_LAYER 2
#define FN_SPACE_LAYER 3

#define LAYER_COUNT FN_SPACE_LAYER

enum macro {
    MACRO_NOP,
    MACRO_FALLTHROUGH,
    MACRO_SAVE_CALIBRATION,
    MACRO_UNSAVE_CALIBRATION,
    MACRO_DEBUG_CALIBRATION,
    MACRO_WEAK_APPLE_FN,
    MACRO_TOGGLE_SOLENOID,
};

#ifndef RIGHT_MODIFIERS_ARE_ARROWS
#define RIGHT_MODIFIERS_ARE_ARROWS 0
#endif

#ifndef APPLE_ARRANGEMENT
#if ENABLE_APPLE_FN_KEY
#define APPLE_ARRANGEMENT 1
#else
#define APPLE_ARRANGEMENT 0
#endif
#endif

// MARK: - Layer 1

#if LAYER_COUNT >= 1
/// Layer 1 is the default base layer. Only the differences to the default
/// mapping need to be defined here.
DEFINE_LAYER(1) {
#if APPLE_ARRANGEMENT
    [KEY(LEFT_WIN)] = KEY(ALT),
    [KEY(LEFT_ALT)] = KEY(LEFT_CMD),
#endif

#if ENABLE_APPLE_FN_KEY
    [KEY(INT_NEXT_TO_LEFT_SHIFT)] = KEY(BACKTICK),
#endif

#if ISO_ENTER && SPLIT_ENTER
    [KEY(ANSI_BACKSLASH)] = KEY(KP_ENTER),
#endif

#if RIGHT_MODIFIERS_ARE_ARROWS
    [KEY(RIGHT_SHIFT)] = KEY(UP_ARROW),
    [KEY(ALT_GR)] = KEY(LEFT_ARROW),
    [KEY(NUM_LOCK)] = KEY(DOWN_ARROW),
    [KEY(RIGHT_CTRL)] = KEY(RIGHT_ARROW),
#else // ^ RIGHT_MODIFIERS_ARE_ARROWS
    [KEY(NUM_LOCK)] = KEY(RIGHT_CMD),
#if SPLIT_RIGHT_SHIFT
#if ENABLE_APPLE_FN_KEY
    [KEY(RIGHT_CTRL)] = MACRO(MACRO_WEAK_APPLE_FN),
#else
    [KEY(RIGHT_CTRL)] = LAYER_ON_HOLD(APPLE_FN_LAYER),
#endif
#endif
#endif

#if ENABLE_APPLE_FN_KEY
    [KEY_APPLE_FN] = MACRO(MACRO_WEAK_APPLE_FN),
#else
    [KEY_APPLE_FN] = LAYER_ON_HOLD(APPLE_FN_LAYER),
#endif
};
#endif

#if LAYER_COUNT >= APPLE_FN_LAYER
DEFINE_LAYER(APPLE_FN_LAYER) {
    // The default action of a layer is to pass through to layers below, and
    // ultimately to the key's default action. However, if you wish to disable
    // keys that you don't explicitly define, use the following line at the
    // beginning of the layer:
    DISABLE_ALL_KEYS_NOT_DEFINED_BELOW,

    [KEY(TAB)] = KEY(CAPS_LOCK),

    [KEY(SPACE)] = LAYER_ON_HOLD(FN_SPACE_LAYER),

#if ENABLE_APPLE_FN_KEY
    [KEY(ESC)] = KEY(INT_NEXT_TO_LEFT_SHIFT),
    [KEY(INT_NEXT_TO_LEFT_SHIFT)] = KEY(INT_NEXT_TO_LEFT_SHIFT),
#else
    [KEY(ESC)] = KEY(BACKTICK),
    [KEY(INT_NEXT_TO_LEFT_SHIFT)] = KEY(BACKTICK),
#endif

    [KEY(LEFT_SHIFT)] = KEY(LEFT_SHIFT),
    [KEY(LEFT_CTRL)] = KEY(LEFT_CTRL),

#if APPLE_ARRANGEMENT
    [KEY(LEFT_WIN)] = KEY(LEFT_ALT),
    [KEY(LEFT_ALT)] = KEY(LEFT_CMD),
#else
    [KEY(LEFT_WIN)] = KEY(LEFT_WIN),
    [KEY(LEFT_ALT)] = KEY(ALT_GR),
#endif

    // Fn + number = F-keys
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

    // Convenience shortcuts
    [KEY(BACKSPACE)] = KEY(DELETE),

    [KEY(Q)] = KEY(HOME),
    [KEY(W)] = KEY(UP_ARROW),
    [KEY(E)] = KEY(END),
    [KEY(R)] = KEY(PAGE_UP),
    [KEY(O)] = KEY(PRINT_SCREEN),
    [KEY(P)] = KEY(SCROLL_LOCK),
    [KEY(OPEN_BRACKET)] = KEY(F11),
    [KEY(CLOSE_BRACKET)] = KEY(F12),

    [KEY(A)] = KEY(LEFT_ARROW),
    [KEY(S)] = KEY(DOWN_ARROW),
    [KEY(D)] = KEY(RIGHT_ARROW),
    [KEY(F)] = KEY(PAGE_DOWN),
    [KEY(G)] = KEY(INSERT),
    [KEY(H)] = KEY(DELETE),
    [KEY(L)] = KEY(NUM_LOCK),
    [KEY(SEMICOLON)] = KEY(PAUSE_BREAK),

#if APPLE_ARRANGEMENT
#if DVORAK_MAPPINGS
    [KEY(X)] = CMD(DVORAK_X),
    [KEY(C)] = CMD(DVORAK_C),
    [KEY(V)] = CMD(DVORAK_V),
#else
    [KEY(X)] = CMD(X),
    [KEY(C)] = CMD(C),
    [KEY(V)] = CMD(V),
#endif
#else
#if DVORAK_MAPPINGS
    [KEY(X)] = CTRL(DVORAK_X),
    [KEY(C)] = CTRL(DVORAK_C),
    [KEY(V)] = CTRL(DVORAK_V),
#else
    [KEY(X)] = CTRL(X),
    [KEY(C)] = CTRL(C),
    [KEY(V)] = CTRL(V),
#endif
#endif
    [KEY(SLASH)] = KEY(RIGHT_SHIFT),

#if RIGHT_MODIFIERS_ARE_ARROWS
    [KEY(RIGHT_SHIFT)] = KEY(PAGE_UP),
    [KEY(NUM_LOCK)] = KEY(PAGE_DOWN),
    [KEY(ALT_GR)] = KEY(HOME),
    [KEY(RIGHT_CTRL)] = KEY(END),
#if ENABLE_APPLE_FN_KEY
    [KEY(RETURN)] = KEY_APPLE_FN,
#else
    [KEY(RETURN)] = KEY(KP_ENTER),
#endif
#else
    [KEY(NUM_LOCK)] = KEY(NUM_LOCK),
#if ENABLE_APPLE_FN_KEY
    [KEY(RIGHT_SHIFT)] = KEY_APPLE_FN,
#endif
    [KEY(RETURN)] = KEY(KP_ENTER),
#endif

    [KEY_APPLE_FN] = LAYER_TOGGLE(APPLE_FN_LAYER),
};
#endif

#if LAYER_COUNT >= FN_SPACE_LAYER
DEFINE_LAYER(FN_SPACE_LAYER) {
    DISABLE_ALL_KEYS_NOT_DEFINED_BELOW,

    [KEY(ESC)] = EXT(RESET_KEYBOARD),
    [KEY(BACKSPACE)] = EXT(RESET_LAYERS),

    [KEY(R)] = EXT(ENTER_BOOTLOADER),
    [KEY(DVORAK_R)] = EXT(ENTER_BOOTLOADER),

#if ENABLE_SIMULATED_TYPING
    [KEY(1)] = EXT(PRINT_DEBUG_INFO),
    [KEY(2)] = MACRO(MACRO_DEBUG_CALIBRATION),
#endif

    [KEY(DVORAK_S)] = MACRO(MACRO_TOGGLE_SOLENOID),
    [KEY(S)] = MACRO(MACRO_TOGGLE_SOLENOID),

    [KEY(DVORAK_U)] = MACRO(MACRO_UNSAVE_CALIBRATION),
    [KEY(U)] = MACRO(MACRO_UNSAVE_CALIBRATION),
    
    [KEY(DVORAK_C)] = MACRO(MACRO_SAVE_CALIBRATION),
    [KEY(C)] = MACRO(MACRO_SAVE_CALIBRATION),

    [KEY(B)] = EXT(TOGGLE_BOOT_PROTOCOL),
    [KEY(DVORAK_B)] = EXT(TOGGLE_BOOT_PROTOCOL),

    [KEY(SPACE)] = LAYER_TOGGLE(FN_SPACE_LAYER),
};
#endif
