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

/// The number of layers to make active. The layer numbering starts from 1,
/// so this is also the number of the highest layer. Any layer with a number
/// higher than this will be unused, i.e., setting `LAYER_COUNT 0` will
/// ignore all layers defined below. The maximum layer count is 31.
#define LAYER_COUNT 0

/// The default base layer. Layers with a number lower than base layer are
/// ignored.
#define DEFAULT_BASE_LAYER 1

/// You can define helper macros to name layers.
#define DVORAK_LAYER 2
#define SHIFT_LAYER 3
#define ALT_LAYER 4
#define PAUSE_LAYER 5
#define FN_LAYER 6

/// Recognised macro names, see `macros.c`. To define a macro, add the name
/// here, e.g., `MACRO_MY_MACRO`, and then map `MACRO(MACRO_MY_MACRO)` to a
/// key. Remember to use the `MACRO()` wrapper, do not use the macro name
/// directly as a keycode! You can have up to 127 macros.
enum macro {
    MACRO_NOP,
    MACRO_FALLTHROUGH,
    MACRO_SHIFT_REPLACE_ALT_WITH_CMD_IF_NOT_ALREADY,
    MACRO_CMD_OR_ALT_IF_ALREADY_CMD,
    MACRO_PRINT_SCREEN_BOOTLOADER,
};

// MARK: - Layer 1

#if LAYER_COUNT >= 1
/// Layer 1 is the default base layer. Only the differences to the default
/// mapping need to be defined here.
DEFINE_LAYER(1) {
    [KEY(PAUSE_BREAK)] = LAYER_TOGGLE_STICKY(PAUSE_LAYER),
    [KEY(SCROLL_LOCK)] = LAYER_TOGGLE_STICKY(DVORAK_LAYER),

    // Hyper = Shift + Ctrl + Alt + Cmd
    //[KEY(CAPS_LOCK)] = EXT(HYPER),

    // Caps Lock works as a Cmd key when held down, or sends Esc when clicked
    [KEY(CAPS_LOCK)] = CMD_OR(ESC),
    [KEY(LEFT_SHIFT)] = SHIFT_AND_LAYER(SHIFT_LAYER),
    [KEY(LEFT_ALT)] = ALT_AND_LAYER(ALT_LAYER),

#if ENABLE_APPLE_FN_KEY
    [KEY(RIGHT_SHIFT)] = KEY(VIRTUAL_APPLE_FN),

    // Apple reverses these two keycodes on its keyboards
    [KEY(BACKTICK)] = KEY(INT_NEXT_TO_LEFT_SHIFT),
    [KEY(INT_NEXT_TO_LEFT_SHIFT)] = KEY(BACKTICK),
#endif
};
#endif

// MARK: - Layer 2

#if LAYER_COUNT >= 2
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

// MARK: - Layer 3

#if LAYER_COUNT >= 3
DEFINE_LAYER(SHIFT_LAYER) {
    // Caps Lock + Shift + S (O in Dvorak) doesn't work on IBM Model M. This
    // layer maps Left Shift + Left Alt to Left Shift + Left Cmd instead so
    // that it's possible to use those shortcuts.
    [KEY(LEFT_ALT)] = KEY(LEFT_CMD),

    [KEY(CAPS_LOCK)] = MACRO(MACRO_CMD_OR_ALT_IF_ALREADY_CMD),

    // Restore Scroll Lock to make the Left Shift + Scroll Lock + Right Shift
    // bootloader shortcut easy to access.
    [KEY(SCROLL_LOCK)] = KEY(SCROLL_LOCK),
    [KEY(PAUSE_BREAK)] = KEY(PAUSE_BREAK),

#if ENABLE_APPLE_FN_KEY
    [KEY(RIGHT_SHIFT)] = RIGHT_SHIFT(VIRTUAL_APPLE_FN),
#endif
};
#endif

// MARK: - Layer 4

#if LAYER_COUNT >= 4
DEFINE_LAYER(ALT_LAYER) {
    // Map Left Alt + Left Shift to Left Cmd + Left Shift. This requires a
    // macro to get rid of the Alt modifier, since normal keycodes don't
    // support removing existing modifiers while they key is held. However,
    // this serves as an example of how the order of layers matters (see also
    // `SHIFT_LAYER` and its `LEFT_ALT`, which handles the same thing much
    // easier when Shift is pressed before Alt).
    [KEY(LEFT_SHIFT)] = MACRO(MACRO_SHIFT_REPLACE_ALT_WITH_CMD_IF_NOT_ALREADY),
    [KEY(CAPS_LOCK)] = MACRO(MACRO_CMD_OR_ALT_IF_ALREADY_CMD),
};
#endif

// MARK: - Layer 5

#if LAYER_COUNT >= 5
DEFINE_LAYER(PAUSE_LAYER) {
    // This layer is toggled via the Pause/Break key, and it disables special
    // mappings on modifiers.
    [KEY(LEFT_SHIFT)] = KEY(LEFT_SHIFT),
    [KEY(RIGHT_CTRL)] = KEY(RIGHT_CTRL),
    [KEY(RIGHT_SHIFT)] = KEY(RIGHT_SHIFT),
    [KEY(CAPS_LOCK)] = KEY(CAPS_LOCK),

    // Jump to bootloader if both left and right shift modifiers are active.
    [KEY(PRINT_SCREEN)] = MACRO(MACRO_PRINT_SCREEN_BOOTLOADER),

#if ENABLE_APPLE_FN_KEY
    [KEY(BACKTICK)] = KEY(BACKTICK),
    [KEY(INT_NEXT_TO_LEFT_SHIFT)] = KEY(INT_NEXT_TO_LEFT_SHIFT),
#endif
};
#endif

// MARK: - Layer 6

#if LAYER_COUNT >= 6
DEFINE_LAYER(FN_LAYER) {
    // The default action of a layer is to pass through to layers below, and
    // ultimately to the key's default action. However, if you wish to disable
    // keys that you don't explicitly define, use the following line at the
    // beginning of the layer:
    DISABLE_ALL_KEYS_NOT_DEFINED_BELOW,

    // This layer simulates some of the Mac Fn key shortcuts. It does not work
    // as a true Fn key, though.
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
    [KEY(BACKSPACE)] = KEY(DELETE),

    [KEY(UP_ARROW)] = KEY(PAGE_UP),
    [KEY(DOWN_ARROW)] = KEY(PAGE_DOWN),
    [KEY(LEFT_ARROW)] = KEY(HOME),
    [KEY(RIGHT_ARROW)] = KEY(END),

    [KEY(RETURN)] = KEY(KP_ENTER),
    [KEY(BACKTICK)] = KEY(ESC),
};
#endif

// MARK: - Layer 7

#if LAYER_COUNT >= 7
DEFINE_LAYER(7) {
    NONE
};
#endif

// MARK: - Layer 8

#if LAYER_COUNT >= 8
DEFINE_LAYER(8) {
    NONE
};
#endif

// MARK: - Layer 9

#if LAYER_COUNT >= 9
DEFINE_LAYER(9) {
    NONE
};
#endif

// MARK: - Layer 10

#if LAYER_COUNT >= 10
DEFINE_LAYER(10) {
    NONE
};
#endif

// MARK: - Layer 11

#if LAYER_COUNT >= 11
DEFINE_LAYER(11) {
    NONE
};
#endif

// MARK: - Layer 12

#if LAYER_COUNT >= 12
DEFINE_LAYER(12) {
    NONE
};
#endif

// MARK: - Layer 13

#if LAYER_COUNT >= 13
DEFINE_LAYER(13) {
    NONE
};
#endif

// MARK: - Layer 14

#if LAYER_COUNT >= 14
DEFINE_LAYER(14) {
    NONE
};
#endif

// MARK: - Layer 15

#if LAYER_COUNT >= 15
DEFINE_LAYER(15) {
    NONE
};
#endif

// MARK: - Layer 16

#if LAYER_COUNT >= 16
DEFINE_LAYER(16) {
    NONE
};
#endif
