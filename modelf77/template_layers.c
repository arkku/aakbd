#include <layers.h>
#include "keymap.h"

/// The default base layer. Layers with a number lower than base layer are
/// ignored.
#define DEFAULT_BASE_LAYER 1

#define WINDOWS_LAYER 2
#define NUM_LOCK_LAYER 3
#define APPLE_FN_LAYER 4
#define WINDOWS_FN_LAYER 5
#define FN_SPACE_LAYER 6

/// The number of layers to make active. The layer numbering starts from 1,
/// so this is also the number of the highest layer. Any layer with a number
/// higher than this will be unused, i.e., setting `LAYER_COUNT 0` will
/// ignore all layers defined below. The maximum layer count is 31.
#define LAYER_COUNT FN_SPACE_LAYER

/// Recognised macro names, see `macros.c`. To define a macro, add the name
/// here, e.g., `MACRO_MY_MACRO`, and then map `MACRO(MACRO_MY_MACRO)` to a
/// key. Remember to use the `MACRO()` wrapper, do not use the macro name
/// directly as a keycode! You can have up to 127 macros.
enum macro {
    MACRO_NOP,
    MACRO_FALLTHROUGH,
    MACRO_WEAK_APPLE_FN
};

#define NUM_ROW_1_COL_1 KEY(KP_7_HOME)
#define NUM_ROW_1_COL_2 KEY(KP_8_UP)
#define NUM_ROW_1_COL_3 KEY(KP_9_PAGE_UP)

#define NUM_ROW_2_COL_1 KEY(KP_4_LEFT)
#define NUM_ROW_2_COL_2 KEY(KP_5)
#define NUM_ROW_2_COL_3 KEY(KP_6_RIGHT)

#define NUM_ROW_3_COL_1 KEY(KP_1_END)
#define NUM_ROW_3_COL_2 KEY(KP_2_DOWN)
#define NUM_ROW_3_COL_3 KEY(KP_3_PAGE_DOWN)

#define NUM_ROW_4_COL_1 KEY(KP_0_INSERT)
#define NUM_ROW_4_COL_2 KEY(UP_ARROW)
#define NUM_ROW_4_COL_3 KEY(KP_COMMA_DEL)

#define NUM_ROW_5_COL_1 KEY(LEFT_ARROW)
#define NUM_ROW_5_COL_2 KEY(DOWN_ARROW)
#define NUM_ROW_5_COL_3 KEY(RIGHT_ARROW)

#ifndef RIGHT_MODIFIERS_ARE_ARROWS
#define RIGHT_MODIFIERS_ARE_ARROWS 0
#endif

#ifndef RIGHT_BLOCK_IS_NUMPAD
#define RIGHT_BLOCK_IS_NUMPAD 0
#endif

// MARK: - Layer 1

#if LAYER_COUNT >= 1
/// Layer 1 is the default base layer. Only the differences to the default
/// mapping need to be defined here.
DEFINE_LAYER(1) {

    // Caps Lock works as a Cmd key when held down, or sends Esc when clicked
    [KEY(CAPS_LOCK)] = CMD_OR(ESC),

    // Apple arrangement for modifiers
    [KEY(LEFT_WIN)] = KEY(ALT),
    [KEY(LEFT_ALT)] = KEY(LEFT_CMD),

#if SPLIT_BACKSPACE
    [KEY(BACKTICK)] = KEY(BACKSPACE),
    [KEY(BACKSPACE)] = KEY(DELETE),
#endif

#if ISO_ENTER && SPLIT_ENTER
    [KEY(ANSI_BACKSLASH)] = KEY(KP_ENTER),
#endif

#if RIGHT_MODIFIERS_ARE_ARROWS
    [KEY(RIGHT_SHIFT)] = KEY(UP_ARROW),
    [KEY(ALT_GR)] = KEY(LEFT_ARROW),
    [KEY(NUM_LOCK)] = KEY(DOWN_ARROW),
    [KEY(RIGHT_CTRL)] = KEY(RIGHT_ARROW),
#if SHORT_SPACE
    [KEY(RIGHT_CMD)] = LAYER_TOGGLE(NUM_LOCK_LAYER),
#endif
#else // ^ RIGHT_MODIFIERS_ARE_ARROWS
    [KEY(NUM_LOCK)] = LAYER_TOGGLE(NUM_LOCK_LAYER),
#endif

    [NUM_ROW_1_COL_1] = KEY(DELETE),
#if RIGHT_BLOCK_IS_NUMPAD
    [NUM_ROW_1_COL_2] = KEY(PRINT_SCREEN),
    [NUM_ROW_1_COL_3] = KEY(F8),

    [NUM_ROW_2_COL_1] = KEY(KP_7_HOME),
    [NUM_ROW_2_COL_2] = KEY(KP_8_UP),
    [NUM_ROW_2_COL_3] = KEY(KP_9_PAGE_UP),

    [NUM_ROW_3_COL_1] = KEY(KP_4_LEFT),
    [NUM_ROW_3_COL_2] = KEY(KP_5),
    [NUM_ROW_3_COL_3] = KEY(KP_6_RIGHT),

    [NUM_ROW_4_COL_1] = KEY(KP_1_END),
    [NUM_ROW_4_COL_2] = KEY(KP_2_DOWN),
    [NUM_ROW_4_COL_3] = KEY(KP_3_PAGE_DOWN),

    [NUM_ROW_5_COL_1] = KEY(KP_0_INSERT),
    [NUM_ROW_5_COL_2] = KEY(KP_COMMA_DEL),
    [NUM_ROW_5_COL_3] = KEY(KP_ENTER),
#else
    [NUM_ROW_1_COL_2] = KEY(F8),
    [NUM_ROW_1_COL_3] = KEY(PAGE_UP),

    [NUM_ROW_2_COL_1] = KEY(INSERT),
    [NUM_ROW_2_COL_2] = KEY(PRINT_SCREEN),
    [NUM_ROW_2_COL_3] = KEY(PAGE_DOWN),

    [NUM_ROW_3_COL_1] = KEY(F10),
    [NUM_ROW_3_COL_2] = KEY(F11),
    [NUM_ROW_3_COL_3] = KEY(F12),

    [NUM_ROW_4_COL_1] = KEY(HOME),
    [NUM_ROW_4_COL_2] = KEY(UP_ARROW),
    [NUM_ROW_4_COL_3] = KEY(END),

    [NUM_ROW_5_COL_1] = KEY(LEFT_ARROW),
    [NUM_ROW_5_COL_2] = KEY(DOWN_ARROW),
    [NUM_ROW_5_COL_3] = KEY(RIGHT_ARROW),
#endif

#if ENABLE_APPLE_FN_KEY
    // Apple reverses these two keycodes on its keyboards, let's undo that
    [KEY(ESC)] = KEY(INT_NEXT_TO_LEFT_SHIFT),
    [KEY(INT_NEXT_TO_LEFT_SHIFT)] = KEY(BACKTICK),

    [KEY_APPLE_FN] = MACRO(MACRO_WEAK_APPLE_FN),
#else
    [KEY(ESC)] = KEY(BACKTICK),

    // Virtual Apple Fn key
    [KEY_APPLE_FN] = LAYER_ON_HOLD(APPLE_FN_LAYER),
#endif
};
#endif

#if LAYER_COUNT >= WINDOWS_LAYER
DEFINE_LAYER(WINDOWS_LAYER) {
    // Caps Lock works as a Ctrl key when held down, or sends Esc when clicked
    [KEY(CAPS_LOCK)] = CTRL_OR(ESC),

    // I seldom need backtick in Windows, so let's just put Esc there as well
    [KEY(ESC)] = KEY(ESC),

    // Undo Apple remaping
    [KEY(INT_NEXT_TO_LEFT_SHIFT)] = KEY(INT_NEXT_TO_LEFT_SHIFT),

    // Restore left Alt, put Alt Gr into the left Windows key
    [KEY(LEFT_WIN)] = KEY(ALT_GR),
    [KEY(LEFT_ALT)] = KEY(LEFT_ALT),

#if !RIGHT_MODIFIERS_ARE_ARROWS
    // Put Windows key on Right Ctrl
    [KEY(RIGHT_CTRL)] = KEY(RIGHT_WIN),
#endif

#if RIGHT_BLOCK_IS_NUMPAD
    [NUM_ROW_1_COL_3] = KEY(PAUSE_BREAK),
#else
    [NUM_ROW_1_COL_2] = KEY(PAUSE_BREAK),
#endif

    [KEY_APPLE_FN] = LAYER_ON_HOLD(WINDOWS_FN_LAYER),
};
#endif

#if LAYER_COUNT >= NUM_LOCK_LAYER
DEFINE_LAYER(NUM_LOCK_LAYER) {
#if RIGHT_BLOCK_IS_NUMPAD
    // Simulated Num Lock (since macOS does not have actual Num Lock support)
    [NUM_ROW_2_COL_1] = KEY(HOME),
    [NUM_ROW_2_COL_2] = KEY(UP_ARROW),
    [NUM_ROW_2_COL_3] = KEY(PAGE_UP),

    [NUM_ROW_3_COL_1] = KEY(LEFT_ARROW),
    [NUM_ROW_3_COL_2] = KEY(DOWN_ARROW),
    [NUM_ROW_3_COL_3] = KEY(RIGHT_ARROW),

    [NUM_ROW_4_COL_1] = KEY(END),
    [NUM_ROW_4_COL_2] = KEY(DOWN_ARROW),
    [NUM_ROW_4_COL_3] = KEY(PAGE_DOWN),

    [NUM_ROW_5_COL_1] = KEY(INSERT),
    [NUM_ROW_5_COL_2] = KEY(DELETE),
#else
    // The right block is not normally a numpad: make it one virtually
    [NUM_ROW_1_COL_2] = KEY(KP_PLUS),
    [NUM_ROW_1_COL_3] = KEY(KP_MINUS),

    [NUM_ROW_2_COL_1] = KEY(KP_7_HOME),
    [NUM_ROW_2_COL_2] = KEY(KP_8_UP),
    [NUM_ROW_2_COL_3] = KEY(KP_9_PAGE_UP),

    [NUM_ROW_3_COL_1] = KEY(KP_4_LEFT),
    [NUM_ROW_3_COL_2] = KEY(KP_5),
    [NUM_ROW_3_COL_3] = KEY(KP_6_RIGHT),

    [NUM_ROW_4_COL_1] = KEY(KP_1_END),
    [NUM_ROW_4_COL_2] = KEY(KP_2_DOWN),
    [NUM_ROW_4_COL_3] = KEY(KP_3_PAGE_DOWN),

    [NUM_ROW_5_COL_1] = KEY(KP_0_INSERT),
    [NUM_ROW_5_COL_2] = KEY(KP_COMMA_DEL),
#endif
    [NUM_ROW_5_COL_3] = KEY(KP_ENTER),
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

    [KEY(CAPS_LOCK)] = LAYER_TOGGLE(WINDOWS_LAYER),
    [KEY(SPACE)] = LAYER_ON_HOLD(FN_SPACE_LAYER),
    //[KEY(Z)] = LAYER_TOGGLE(REMAP_LAYER),

    [KEY(ESC)] = KEY(ESC),
    [KEY(LEFT_CTRL)] = KEY(LEFT_CTRL),
    [KEY(LEFT_WIN)] = KEY(LEFT_ALT),
    [KEY(LEFT_ALT)] = KEY(LEFT_CMD),
    [KEY(LEFT_SHIFT)] = KEY(LEFT_SHIFT),
#if ENABLE_APPLE_FN_KEY
    [KEY(INT_NEXT_TO_LEFT_SHIFT)] = KEY(INT_NEXT_TO_LEFT_SHIFT),
#else
    [KEY(INT_NEXT_TO_LEFT_SHIFT)] = KEY(BACKTICK),
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
#if SPLIT_BACKSPACE
    [KEY(BACKSPACE)] = KEY(NUM_LOCK),
    [KEY(BACKTICK)] = KEY(DELETE),
#else
    [KEY(BACKSPACE)] = KEY(DELETE),
#endif

    [KEY(Q)] = KEY(HOME),
    [KEY(W)] = KEY(UP_ARROW),
    [KEY(E)] = KEY(END),
    [KEY(R)] = KEY(PAGE_UP),
#if DVORAK_MAPPINGS
    [KEY(T)] = CMD(DVORAK_OPEN_BRACKET),
    [KEY(Y)] = CMD(DVORAK_CLOSE_BRACKET),
#else
    [KEY(T)] = CMD(OPEN_BRACKET),
    [KEY(Y)] = CMD(CLOSE_BRACKET),
#endif
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

#if DVORAK_MAPPINGS
    [KEY(X)] = CMD(DVORAK_X),
    [KEY(C)] = CMD(DVORAK_C),
    [KEY(V)] = CMD(DVORAK_V),
#else
    [KEY(X)] = CMD(X),
    [KEY(C)] = CMD(C),
    [KEY(V)] = CMD(V),
#endif
    [KEY(SLASH)] = KEY(RIGHT_SHIFT),

    [NUM_ROW_1_COL_1] = KEY(NUM_LOCK),
#if RIGHT_BLOCK_IS_NUMPAD
    [NUM_ROW_1_COL_2] = CMD_SHIFT(3),
#if APPLE_FN_IS_MODIFIER
    [NUM_ROW_1_COL_3] = APPLE_FN(F8),
#else
    [NUM_ROW_1_COL_3] = KEY(F8),
#endif

    [NUM_ROW_2_COL_1] = KEY(HOME),
    [NUM_ROW_2_COL_2] = KEY(PAGE_UP),
    [NUM_ROW_2_COL_3] = KEY(PAGE_UP),

    [NUM_ROW_3_COL_1] = KEY(HOME),
    [NUM_ROW_3_COL_2] = KEY(PAGE_DOWN),
    [NUM_ROW_3_COL_3] = KEY(END),

    [NUM_ROW_4_COL_1] = KEY(END),
    [NUM_ROW_4_COL_2] = KEY(PAGE_DOWN),
    [NUM_ROW_4_COL_3] = KEY(PAGE_DOWN),

    [NUM_ROW_5_COL_1] = KEY(INSERT),
    [NUM_ROW_5_COL_2] = KEY(DELETE),
    [NUM_ROW_5_COL_3] = KEY(KP_ENTER),
#else // ^ RIGHT_BLOCK_IS_NUMPAD
#if APPLE_FN_IS_MODIFIER
    [NUM_ROW_1_COL_2] = APPLE_FN(F8),
    [NUM_ROW_3_COL_1] = APPLE_FN(F10),
    [NUM_ROW_3_COL_2] = APPLE_FN(F11),
    [NUM_ROW_3_COL_3] = APPLE_FN(F12),
#else
    [NUM_ROW_1_COL_2] = KEY(F8),
    [NUM_ROW_3_COL_1] = KEY(F10),
    [NUM_ROW_3_COL_2] = KEY(F11),
    [NUM_ROW_3_COL_3] = KEY(F12),
#endif
    [NUM_ROW_1_COL_3] = KEY(HOME),

    [NUM_ROW_2_COL_1] = KEY(INSERT),
    [NUM_ROW_2_COL_2] = CMD_SHIFT(3),
    [NUM_ROW_2_COL_3] = KEY(END),

    [NUM_ROW_4_COL_1] = SHIFT(TAB),
    [NUM_ROW_4_COL_2] = KEY(PAGE_UP),
    [NUM_ROW_4_COL_3] = KEY(TAB),

    [NUM_ROW_5_COL_1] = KEY(HOME),
    [NUM_ROW_5_COL_2] = KEY(PAGE_DOWN),
    [NUM_ROW_5_COL_3] = KEY(END),
#endif

#if RIGHT_MODIFIERS_ARE_ARROWS
    [KEY(RIGHT_SHIFT)] = KEY(PAGE_UP),
    [KEY(NUM_LOCK)] = KEY(PAGE_DOWN),
    [KEY(ALT_GR)] = KEY(HOME),
    [KEY(RIGHT_CTRL)] = KEY(END),
    [KEY(RETURN)] = KEY_APPLE_FN,
#else
    [KEY(NUM_LOCK)] = KEY(NUM_LOCK),
    [KEY(RIGHT_SHIFT)] = KEY_APPLE_FN,
    [KEY(RETURN)] = KEY(KP_ENTER),
#endif

    [KEY_APPLE_FN] = LAYER_TOGGLE(APPLE_FN_LAYER),
};
#endif

#if LAYER_COUNT >= WINDOWS_FN_LAYER
DEFINE_LAYER(WINDOWS_FN_LAYER) {
    // Only the differences to APPLE_FN_LAYER - the other layer is combined
    // in macros.c
    [KEY(ESC)] = KEY(BACKTICK),
    [KEY(INT_NEXT_TO_LEFT_SHIFT)] = KEY(ESC),

    [KEY(LEFT_WIN)] = KEY(LEFT_WIN),
    [KEY(LEFT_ALT)] = KEY(LEFT_ALT),

#if SPLIT_BACKSPACE
    [KEY(BACKSPACE)] = KEY(INSERT),
#endif

#if DVORAK_MAPPINGS
    [KEY(T)] = CTRL(DVORAK_OPEN_BRACKET),
    [KEY(Y)] = CTRL(DVORAK_CLOSE_BRACKET),
    [KEY(X)] = CTRL(DVORAK_X),
    [KEY(C)] = CTRL(DVORAK_C),
    [KEY(V)] = CTRL(DVORAK_V),
#else
    [KEY(T)] = CTRL(OPEN_BRACKET),
    [KEY(Y)] = CTRL(CLOSE_BRACKET),
    [KEY(X)] = CTRL(X),
    [KEY(C)] = CTRL(C),
    [KEY(V)] = CTRL(V),
#endif
    [KEY(SLASH)] = KEY(RIGHT_SHIFT),
#if RIGHT_BLOCK_IS_NUMPAD
    [NUM_ROW_1_COL_2] = KEY(PRINT_SCREEN),
    [NUM_ROW_1_COL_3] = KEY(PAUSE_BREAK),
#else // ^ RIGHT_BLOCK_IS_NUMPAD
    [NUM_ROW_1_COL_2] = KEY(PAUSE_BREAK),
    [NUM_ROW_2_COL_2] = KEY(PRINT_SCREEN),
    [NUM_ROW_3_COL_1] = KEY(F10),
    [NUM_ROW_3_COL_2] = KEY(F11),
    [NUM_ROW_3_COL_3] = KEY(F12),
#endif

    [KEY(RETURN)] = KEY(KP_ENTER),

    [KEY_APPLE_FN] = LAYER_TOGGLE(WINDOWS_FN_LAYER),
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
    [KEY(D)] = EXT(PRINT_DEBUG_INFO),
    [KEY(DVORAK_D)] = EXT(PRINT_DEBUG_INFO),
#endif

    [KEY(B)] = EXT(TOGGLE_BOOT_PROTOCOL),
    [KEY(DVORAK_B)] = EXT(TOGGLE_BOOT_PROTOCOL),

    [KEY(SPACE)] = LAYER_TOGGLE(FN_SPACE_LAYER),
};
#endif
