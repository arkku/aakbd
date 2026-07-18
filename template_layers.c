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
#define LAYER_COUNT 3

/// The default base layer. Layers with a number lower than base layer are
/// ignored.
#define DEFAULT_BASE_LAYER 1

/// You can define helper macros to name layers.
#define FN_LAYER 2
#define FN_SPACE_LAYER 3

/// Recognised macro names, see `macros.c`. To define a macro, add the name
/// here, e.g., `MACRO_MY_MACRO`, and then map `MACRO(MACRO_MY_MACRO)` to a
/// key. Remember to use the `MACRO()` wrapper, do not use the macro name
/// directly as a keycode! You can have up to 127 macros (or 63 with Vial).
/// Note that every macro also knows which physical key was used to activate
/// it, so the same macro can actually do different things based on the key.
enum macro {
    FN_LAYER_ON_HOLD,
};

// MARK: - Layer 1

#if LAYER_COUNT >= 1
/// Layer 1 is the default base layer. Only the differences to the default
/// mapping need to be defined here.
DEFINE_LAYER(1) {
    // Caps Lock works as a Cmd or Ctrl key when held down, or sends Esc when
    // clicked
    //[KEY(CAPS_LOCK)] = CMD_OR(ESC),
    //[KEY(CAPS_LOCK)] = CTRL_OR(ESC),

    [KEY(RIGHT_CTRL)] = MACRO(FN_LAYER_ON_HOLD),
};
#endif

#if LAYER_COUNT >= FN_LAYER
DEFINE_LAYER(FN_LAYER) {
#if !ENABLE_APPLE_FN_KEY
    // The default action of a layer is to pass through to layers below, and
    // ultimately to the key's default action. However, if you wish to disable
    // keys that you don't explicitly define, use the following line at the
    // beginning of the layer:
    DISABLE_ALL_KEYS_NOT_DEFINED_BELOW,
#endif

    // This layer simulates some of the Mac Fn key shortcuts. It does not work
    // as a true Fn key, though.
    [KEY(BACKTICK)] = KEY(ESC),
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
    [KEY(RETURN)] = KEY(KP_ENTER),

    [KEY(UP_ARROW)] = KEY(PAGE_UP),
    [KEY(DOWN_ARROW)] = KEY(PAGE_DOWN),
    [KEY(LEFT_ARROW)] = KEY(HOME),
    [KEY(RIGHT_ARROW)] = KEY(END),

#if LAYER_COUNT >= FN_SPACE_LAYER
    [KEY(SPACE)] = LAYER_ON_HOLD(FN_SPACE_LAYER),
#endif
};
#endif

#if LAYER_COUNT >= FN_SPACE_LAYER
DEFINE_LAYER(FN_SPACE_LAYER) {
    [KEY(ESC)] = EXT(ENTER_BOOTLOADER),
    [KEY(R)] = EXT(ENTER_BOOTLOADER),
    [KEY(X)] = EXT(RESET_KEYBOARD),
    [KEY(DVORAK_B)] = EXT(TOGGLE_BOOT_PROTOCOL),

    [KEY(SPACE)] = LAYER_TOGGLE(FN_SPACE_LAYER),
};
#endif
