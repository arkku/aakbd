#include <layers.h>
#include "keymap.h"

#define DEFAULT_BASE_LAYER 1

#define DVORAK_LAYER 2
#define IPAD_LAYER 3
#define LINUX_LAYER 4
#define WINDOWS_LAYER 5
#define APPLE_FN_LAYER 6
#define WINDOWS_FN_LAYER 7
#define FN_SPACE_LAYER 8

#define LAYER_COUNT FN_SPACE_LAYER

enum macro {
    MACRO_NOP,
    MACRO_FALLTHROUGH,
    MACRO_WEAK_APPLE_FN,
    MACRO_IPAD_A_O,
    MACRO_IPAD_AE_OE,
    MACRO_DEBUG_MATRIX,
};

// MARK: - Layer 1

#if LAYER_COUNT >= 1
DEFINE_LAYER(1) {
    [KEY(CAPS_LOCK)] = CMD_OR(ESC),
    [KEY(LEFT_WIN)] = KEY(ALT),
    [KEY(LEFT_ALT)] = KEY(LEFT_CMD),
    [KEY(RIGHT_CTRL)] = KEY_APPLE_FN,

#if ENABLE_MEDIA_KEYS
    [KEY(SCROLL_LOCK)] = KEY(VOLUME_MUTE),
#else
    [KEY(SCROLL_LOCK)] = KEY(F19),
#endif

    [KEY_APPLE_FN] = MACRO(MACRO_WEAK_APPLE_FN),
};
#endif

#if LAYER_COUNT >= DVORAK_LAYER
DEFINE_LAYER(DVORAK_LAYER) {
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
};
#endif

#if LAYER_COUNT >= LINUX_LAYER
DEFINE_LAYER(LINUX_LAYER) {
    [KEY(CAPS_LOCK)] = CTRL_OR(ESC),

    [KEY(INT_NEXT_TO_LEFT_SHIFT)] = KEY(INT_NEXT_TO_LEFT_SHIFT),

    [KEY(LEFT_WIN)] = KEY(ALT_GR),
    [KEY(LEFT_ALT)] = KEY(LEFT_ALT),
    [KEY(RIGHT_CTRL)] = KEY(LEFT_WIN),
};
#endif

#if LAYER_COUNT >= WINDOWS_LAYER
DEFINE_LAYER(WINDOWS_LAYER) {
    [KEY(CAPS_LOCK)] = CTRL_OR(ESC),

    [KEY(INT_NEXT_TO_LEFT_SHIFT)] = KEY(INT_NEXT_TO_LEFT_SHIFT),

    [KEY(LEFT_WIN)] = KEY(ALT_GR),
    [KEY(LEFT_ALT)] = KEY(LEFT_ALT),
    [KEY(RIGHT_CTRL)] = KEY(LEFT_WIN),

    [KEY_APPLE_FN] = LAYER_ON_HOLD(WINDOWS_FN_LAYER),
};
#endif

#if LAYER_COUNT >= APPLE_FN_LAYER
DEFINE_LAYER(APPLE_FN_LAYER) {
    [KEY(TAB)] = KEY(CAPS_LOCK),

    [KEY(CAPS_LOCK)] = LAYER_TOGGLE(WINDOWS_LAYER),
    [KEY(SPACE)] = LAYER_ON_HOLD(FN_SPACE_LAYER),

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

    [KEY(BACKSPACE)] = KEY(DELETE),

    [KEY(Q)] = KEY(HOME),
    [KEY(W)] = KEY(UP_ARROW),
    [KEY(E)] = KEY(END),
    [KEY(R)] = KEY(PAGE_UP),

    [KEY(A)] = KEY(LEFT_ARROW),
    [KEY(S)] = KEY(DOWN_ARROW),
    [KEY(D)] = KEY(RIGHT_ARROW),
    [KEY(F)] = KEY(PAGE_DOWN),

    [KEY(RETURN)] = KEY(KP_ENTER),

#if ENABLE_MEDIA_KEYS
    [KEY(COMMA)] = KEY(PREVIOUS_TRACK),
    [KEY(PERIOD)] = KEY(PLAY_PAUSE),
    [KEY(SLASH)] = KEY(NEXT_TRACK),

    [KEY(SEMICOLON)] = KEY(VOLUME_DOWN),
    [KEY(QUOTE)] = KEY(VOLUME_UP),
#else
    [KEY(COMMA)] = KEY(F22),
    [KEY(PERIOD)] = KEY(F23),
    [KEY(SLASH)] = KEY(F24),

    [KEY(SEMICOLON)] = KEY(F20),
    [KEY(QUOTE)] = KEY(F21),
#endif

    [KEY_APPLE_FN] = LAYER_TOGGLE(APPLE_FN_LAYER),
};
#endif

#if LAYER_COUNT >= WINDOWS_FN_LAYER
DEFINE_LAYER(WINDOWS_FN_LAYER) {
    [KEY(LEFT_WIN)] = KEY(LEFT_WIN),
    [KEY(LEFT_ALT)] = KEY(LEFT_ALT),

    [KEY(SLASH)] = KEY(RIGHT_SHIFT),

    [KEY(Q)] = KEY(HOME),
    [KEY(W)] = KEY(UP_ARROW),
    [KEY(E)] = KEY(END),
    [KEY(R)] = KEY(PAGE_UP),

    [KEY(A)] = KEY(LEFT_ARROW),
    [KEY(S)] = KEY(DOWN_ARROW),
    [KEY(D)] = KEY(RIGHT_ARROW),
    [KEY(F)] = KEY(PAGE_DOWN),
    [KEY(G)] = KEY(INSERT),
    [KEY(H)] = KEY(DELETE),
    [KEY(L)] = KEY(NUM_LOCK),
    [KEY(SEMICOLON)] = KEY(PAUSE_BREAK),

    [KEY(O)] = KEY(SCROLL_LOCK),
    [KEY(P)] = KEY(PRINT_SCREEN),

    // Our current media key implementation does not work in Windows
    // because Windows doesn't correctly parse the descriptor. Remap these:
    [KEY(SCROLL_LOCK)] = KEY(F19),  // mute
    [KEY(SEMICOLON)] = KEY(F20),    // volume down
    [KEY(QUOTE)] = KEY(F21),        // volume up
    [KEY(COMMA)] = KEY(F22),        // prev track
    [KEY(PERIOD)] = KEY(F23),       // play/pause
    [KEY(SLASH)] = KEY(F24),        // next track

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
    [KEY(PRINT_SCREEN)] = EXT(PRINT_DEBUG_INFO),
    [KEY(1)] = EXT(PRINT_DEBUG_INFO),
    [KEY(2)] = MACRO(MACRO_DEBUG_MATRIX),
#endif

    [KEY(V)] = LAYER_TOGGLE(DVORAK_LAYER),
    [KEY(DVORAK_V)] = LAYER_TOGGLE(DVORAK_LAYER),

    [KEY(I)] = LAYER_TOGGLE(IPAD_LAYER),
    [KEY(DVORAK_I)] = LAYER_TOGGLE(IPAD_LAYER),

    [KEY(B)] = EXT(TOGGLE_BOOT_PROTOCOL),
    [KEY(DVORAK_B)] = EXT(TOGGLE_BOOT_PROTOCOL),

    [KEY(SPACE)] = LAYER_TOGGLE(FN_SPACE_LAYER),
};
#endif
