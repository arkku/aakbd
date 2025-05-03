#include <layers.h>
#include "keymap.h"

/// The default base layer. Layers with a number lower than base layer are
/// ignored.
#define DEFAULT_BASE_LAYER 1

#define DVORAK_LAYER 2
#define IPAD_LAYER 3
#define WINDOWS_LAYER 4
#define APPLE_FN_LAYER 5
#define WINDOWS_FN_LAYER 6
#define FN_SPACE_LAYER 7

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
    MACRO_SAVE_CALIBRATION,
    MACRO_UNSAVE_CALIBRATION,
    MACRO_DEBUG_CALIBRATION,
    MACRO_WEAK_APPLE_FN,
    MACRO_TOGGLE_SOLENOID,
    MACRO_IPAD_A_O,
    MACRO_IPAD_AE_OE,
    MACRO_CALIBRATE,
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

#ifndef ESC_IS_BACKTICK
#define ESC_IS_BACKTICK 0
#endif

// MARK: - Layer 1

#if LAYER_COUNT >= 1
/// Layer 1 is the default base layer. Only the differences to the default
/// mapping need to be defined here.
DEFINE_LAYER(1) {
#if ESC_IS_BACKTICK
    [KEY(ESC)] = KEY(BACKTICK),
#endif

    [KEY(CAPS_LOCK)] = CMD_OR(ESC),
    [KEY(LEFT_WIN)] = KEY(ALT),
    [KEY(LEFT_ALT)] = KEY(LEFT_CMD),

#if SPLIT_BACKSPACE
    [KEY(BACKTICK)] = KEY(BACKSPACE),
    [KEY(BACKSPACE)] = KEY(DELETE),
#endif

#if ISO_ENTER && SPLIT_ENTER
    [KEY(ANSI_BACKSLASH)] = ALT(BACKSPACE),
#endif

#if RIGHT_MODIFIERS_ARE_ARROWS
    [KEY(RIGHT_SHIFT)] = KEY(UP_ARROW),
    [KEY(ALT_GR)] = KEY(LEFT_ARROW),
    [KEY(NUM_LOCK)] = KEY(DOWN_ARROW),
    [KEY(RIGHT_CTRL)] = KEY(RIGHT_ARROW),
#if SHORT_SPACE
    [KEY(RIGHT_CMD)] = KEY(ALT_GR),
#endif
#else // ^ RIGHT_MODIFIERS_ARE_ARROWS
    [KEY(NUM_LOCK)] = EXT(KEYLOCK),
#endif

#if SPLIT_RIGHT_SHIFT
#if ENABLE_APPLE_FN_KEY
    [KEY_APPLE_FN] = MACRO(MACRO_WEAK_APPLE_FN),
#else
    [KEY_APPLE_FN] = LAYER_ON_HOLD(APPLE_FN_LAYER),
#endif
#else // ^ SPLIT_RIGHT_SHIFT
#if ENABLE_APPLE_FN_KEY
    [KEY(RIGHT_CTRL)] = MACRO(MACRO_WEAK_APPLE_FN),
#else
    [KEY(RIGHT_CTRL)] = LAYER_ON_HOLD(APPLE_FN_LAYER),
#endif
#endif // ^ !SPLIT_RIGHT_SHIFT
};
#endif

#if LAYER_COUNT >= DVORAK_LAYER
DEFINE_LAYER(DVORAK_LAYER) {
    // This remaps keys such that you can type with Dvorak layout if the
    // operating system is set to US QWERTY layout.
    [KEY(DASH)] = KEY(OPEN_BRACKET),
    [KEY(EQUALS)] = KEY(CLOSE_BRACKET),

    [KEY(Q)] = KEY(QUOTE),
    [KEY(W)] = KEY(COMMA),
    [KEY(E)] = KEY(PERIOD),
    [KEY(R)] = KEY(P),
    [KEY(T)] = KEY(Y),
    [KEY(Y)] = KEY(F),
    [KEY(U)] = KEY(G),
    [KEY(I)] = KEY(C),
    [KEY(O)] = KEY(R),
    [KEY(P)] = KEY(L),
    [KEY(OPEN_BRACKET)] = KEY(DASH),
    [KEY(CLOSE_BRACKET)] = KEY(EQUALS),

    [KEY(A)] = KEY(A),
    [KEY(S)] = KEY(O),
    [KEY(D)] = KEY(E),
    [KEY(F)] = KEY(U),
    [KEY(G)] = KEY(I),
    [KEY(H)] = KEY(D),
    [KEY(J)] = KEY(H),
    [KEY(K)] = KEY(T),
    [KEY(L)] = KEY(N),
    [KEY(SEMICOLON)] = KEY(S),
    [KEY(QUOTE)] = KEY(SLASH),

    [KEY(Z)] = KEY(SEMICOLON),
    [KEY(X)] = KEY(Q),
    [KEY(C)] = KEY(J),
    [KEY(V)] = KEY(K),
    [KEY(B)] = KEY(X),
    [KEY(N)] = KEY(B),
    [KEY(M)] = KEY(M),
    [KEY(COMMA)] = KEY(W),
    [KEY(PERIOD)] = KEY(V),
    [KEY(SLASH)] = KEY(Z),
};
#endif

#if LAYER_COUNT >= IPAD_LAYER
DEFINE_LAYER(IPAD_LAYER) {
    [KEY(INT_NEXT_TO_LEFT_SHIFT)] = MACRO(MACRO_IPAD_AE_OE),
    [KEY(A)] = MACRO(MACRO_IPAD_A_O),
    [KEY(DVORAK_O)] = MACRO(MACRO_IPAD_A_O),
    [KEY(ESC)] = KEY(BACKTICK),
};
#endif

#if LAYER_COUNT >= WINDOWS_LAYER
DEFINE_LAYER(WINDOWS_LAYER) {
    // Caps Lock works as a Ctrl key when held down, or sends Esc when clicked
    [KEY(CAPS_LOCK)] = CTRL_OR(ESC),

#if ISO_ENTER && SPLIT_ENTER
    [KEY(ANSI_BACKSLASH)] = CTRL(BACKSPACE),
#endif

    // Undo any remapping of these
    [KEY(ESC)] = KEY(BACKTICK),
    [KEY(INT_NEXT_TO_LEFT_SHIFT)] = KEY(INT_NEXT_TO_LEFT_SHIFT),

    // Restore left Alt, put Alt Gr into the left Windows key
    [KEY(LEFT_WIN)] = KEY(ALT_GR),
    [KEY(LEFT_ALT)] = KEY(LEFT_ALT),

#if !RIGHT_MODIFIERS_ARE_ARROWS
    // Put Windows key on Right Ctrl
    [KEY(RIGHT_CTRL)] = KEY(RIGHT_WIN),
#endif

    [KEY_APPLE_FN] = LAYER_ON_HOLD(WINDOWS_FN_LAYER),
};
#endif

#if LAYER_COUNT >= APPLE_FN_LAYER
DEFINE_LAYER(APPLE_FN_LAYER) {
    [KEY(TAB)] = KEY(CAPS_LOCK),

    [KEY(CAPS_LOCK)] = LAYER_TOGGLE(WINDOWS_LAYER),
    [KEY(SPACE)] = LAYER_ON_HOLD(FN_SPACE_LAYER),

#if ISO_ENTER && SPLIT_ENTER
    [KEY(ANSI_BACKSLASH)] = CTRL(BACKSPACE),
#endif

#if ESC_IS_BACKTICK
    [KEY(ESC)] = KEY(ESC),
#else
    [KEY(ESC)] = KEY(BACKTICK),
#endif

    [KEY(LEFT_SHIFT)] = KEY(LEFT_SHIFT),
    [KEY(LEFT_CTRL)] = KEY(LEFT_CTRL),

    [KEY(LEFT_WIN)] = KEY(LEFT_ALT),
    [KEY(LEFT_ALT)] = KEY(LEFT_CMD),

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
    [KEY(OPEN_BRACKET)] = KEY(F11),
    [KEY(CLOSE_BRACKET)] = KEY(F12),

    [KEY(SLASH)] = KEY(RIGHT_SHIFT),

    // Convenience shortcuts
#if SPLIT_BACKSPACE
    [KEY(BACKSPACE)] = KEY(BACKSPACE),
    [KEY(BACKTICK)] = KEY(DELETE),
#else
    [KEY(BACKSPACE)] = KEY(DELETE),
#endif

#if !(ENABLE_APPLE_FN_KEY && RIGHT_MODIFIERS_ARE_ARROWS)
    // If we don't have an actual Apple Fn key and arrow keys available
    // let's create the arrows behind Fn

    [KEY(Q)] = KEY(HOME),
    [KEY(W)] = KEY(UP_ARROW),
    [KEY(E)] = KEY(END),
    [KEY(R)] = KEY(PAGE_UP),

    [KEY(A)] = KEY(LEFT_ARROW),
    [KEY(S)] = KEY(DOWN_ARROW),
    [KEY(D)] = KEY(RIGHT_ARROW),
    [KEY(F)] = KEY(PAGE_DOWN),
#endif // ^ !(ENABLE_APPLE_FN_KEY && RIGHT_MODIFIERS_ARE_ARROWS)

#if !ENABLE_APPLE_FN_KEY
#if DVORAK_MAPPINGS
    [KEY(X)] = CMD(DVORAK_X),
    [KEY(C)] = CMD(DVORAK_C),
    [KEY(V)] = CMD(DVORAK_V),
#else
    [KEY(X)] = CMD(X),
    [KEY(C)] = CMD(C),
    [KEY(V)] = CMD(V),
#endif
#endif

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
#else // ^ RIGHT_MODIFIERS_ARE_ARROWS
    [KEY(NUM_LOCK)] = KEY(NUM_LOCK),
#if ENABLE_APPLE_FN_KEY
    [KEY(RIGHT_SHIFT)] = KEY_APPLE_FN,
#endif
    [KEY(RETURN)] = KEY(KP_ENTER),
#endif

    [KEY_APPLE_FN] = LAYER_TOGGLE(APPLE_FN_LAYER),
};
#endif

#if LAYER_COUNT >= WINDOWS_FN_LAYER
DEFINE_LAYER(WINDOWS_FN_LAYER) {
    // Only the differences to APPLE_FN_LAYER - the other layer is combined
    // in macros.c
    [KEY(LEFT_WIN)] = KEY(LEFT_WIN),
    [KEY(LEFT_ALT)] = KEY(LEFT_ALT),

#if SPLIT_BACKSPACE
    [KEY(BACKSPACE)] = KEY(INSERT),
#endif

    [KEY(SLASH)] = KEY(RIGHT_SHIFT),

    [KEY(Q)] = KEY(HOME),
    [KEY(W)] = KEY(UP_ARROW),
    [KEY(E)] = KEY(END),
    [KEY(R)] = KEY(PAGE_UP),
#if DVORAK_MAPPINGS
    [KEY(T)] = CTRL(DVORAK_OPEN_BRACKET),
    [KEY(Y)] = CTRL(DVORAK_CLOSE_BRACKET),
#else
    [KEY(T)] = CTRL(OPEN_BRACKET),
    [KEY(Y)] = CTRL(CLOSE_BRACKET),
#endif
    [KEY(O)] = KEY(PRINT_SCREEN),
    [KEY(P)] = KEY(SCROLL_LOCK),

    [KEY(A)] = KEY(LEFT_ARROW),
    [KEY(S)] = KEY(DOWN_ARROW),
    [KEY(D)] = KEY(RIGHT_ARROW),
    [KEY(F)] = KEY(PAGE_DOWN),
    [KEY(G)] = KEY(INSERT),
    [KEY(H)] = KEY(DELETE),
    [KEY(L)] = KEY(NUM_LOCK),
    [KEY(SEMICOLON)] = KEY(PAUSE_BREAK),

#if DVORAK_MAPPINGS
    [KEY(X)] = CTRL(DVORAK_X),
    [KEY(C)] = CTRL(DVORAK_C),
    [KEY(V)] = CTRL(DVORAK_V),
#else
    [KEY(X)] = CTRL(X),
    [KEY(C)] = CTRL(C),
    [KEY(V)] = CTRL(V),
#endif

#if !RIGHT_MODIFIERS_ARE_ARROWS
    [KEY(RIGHT_SHIFT)] = KEY(RIGHT_WIN),
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
    [KEY(1)] = EXT(PRINT_DEBUG_INFO),
    [KEY(2)] = MACRO(MACRO_DEBUG_CALIBRATION),
#endif

    [KEY(DVORAK_H)] = MACRO(MACRO_TOGGLE_SOLENOID),
    [KEY(H)] = MACRO(MACRO_TOGGLE_SOLENOID),

    [KEY(DVORAK_U)] = MACRO(MACRO_UNSAVE_CALIBRATION),
    [KEY(U)] = MACRO(MACRO_UNSAVE_CALIBRATION),

    [KEY(DVORAK_S)] = MACRO(MACRO_SAVE_CALIBRATION),
    [KEY(S)] = MACRO(MACRO_SAVE_CALIBRATION),

    [KEY(DVORAK_C)] = MACRO(MACRO_CALIBRATE),

    [KEY(DVORAK_V)] = LAYER_TOGGLE(DVORAK_LAYER),

    [KEY(DVORAK_I)] = LAYER_TOGGLE(IPAD_LAYER),

    [KEY(DVORAK_B)] = EXT(TOGGLE_BOOT_PROTOCOL),

    [KEY(SPACE)] = LAYER_TOGGLE(FN_SPACE_LAYER),
};
#endif
