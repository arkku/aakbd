#include <layers.h>
#include "keymap.h"

#define DEFAULT_BASE_LAYER 1
#define ESC_LAYER 2

#define LAYER_COUNT ESC_LAYER

enum macro {
    MACRO_NOP,
    MACRO_FALLTHROUGH,
    MACRO_SAVE_CALIBRATION,
    MACRO_UNSAVE_CALIBRATION,
    MACRO_DEBUG_CALIBRATION,
    MACRO_TOGGLE_SOLENOID,
};

// MARK: - Layer 1

#if LAYER_COUNT >= 1
/// Layer 1 is the default base layer. Only the differences to the default
/// mapping need to be defined here.
DEFINE_LAYER(1) {
    // Bottom row left
    [KEY(KP_A)] = LAYER_OR_PLAIN_KEY(ESC_LAYER, KEY(ESC)),
    [KEY(KP_B)] = KEY(PAGE_UP),
    [KEY(KP_C)] = KEY(PAGE_DOWN),

    // Bottom row middle
    [KEY(KP_D)] = KEY(PRINT_SCREEN),
    [KEY(KP_E)] = KEY(SCROLL_LOCK),
    [KEY(KP_F)] = KEY(PAUSE_BREAK),
};
#endif

#if LAYER_COUNT >= ESC_LAYER
DEFINE_LAYER(ESC_LAYER) {
    // The default action of a layer is to pass through to layers below, and
    // ultimately to the key's default action. However, if you wish to disable
    // keys that you don't explicitly define, use the following line at the
    // beginning of the layer:
    DISABLE_ALL_KEYS_NOT_DEFINED_BELOW,

    [KEY(NUM_LOCK)] = EXT(ENTER_BOOTLOADER),

#if ENABLE_SIMULATED_TYPING
    // Bottom row middle
    [KEY(KP_D)] = EXT(PRINT_DEBUG_INFO),
    [KEY(KP_E)] = MACRO(MACRO_DEBUG_CALIBRATION),
#endif
    [KEY(KP_F)] = EXT(RESET_KEYBOARD),

    [KEY(F5)] = MACRO(MACRO_SAVE_CALIBRATION),
    [KEY(F8)] = MACRO(MACRO_UNSAVE_CALIBRATION),
};
#endif
