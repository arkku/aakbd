#include <layers.h>
#include "keymap.h"

#define DEFAULT_BASE_LAYER 1
#define APPLE_LAYER 2
#define LINUX_LAYER 3
#define NUM_LOCK_LAYER 4
#define FN_LAYER 5

#define LAYER_COUNT FN_LAYER

enum macro {
    MACRO_NOP,
    MACRO_FALLTHROUGH,
    MACRO_CYCLE_OS_LAYERS,
    MACRO_SAVE_CALIBRATION,
    MACRO_UNSAVE_CALIBRATION,
    MACRO_DEBUG_CALIBRATION,
    MACRO_TYPE_00,
    MACRO_TOGGLE_SOLENOID,
};

// MARK: - Layer 1

#if LAYER_COUNT >= 1
/// Layer 1 is the default base layer. Only the differences to the default
/// mapping need to be defined here.
DEFINE_LAYER(DEFAULT_BASE_LAYER) {
    [KEY(F1)] = KEY(INSERT),
    [KEY(F2)] = KEY(DELETE),
    [KEY(F5)] = KEY(HOME),
    [KEY(F6)] = KEY(END),
    [KEY(F9)] = KEY(PAGE_UP),
    [KEY(F10)] = KEY(PAGE_DOWN),

    [KEY(KP_D)] = KEY(PRINT_SCREEN),
    [KEY(KP_E)] = KEY(SCROLL_LOCK),
    [KEY(KP_F)] = LAYER_OR_PLAIN_KEY(FN_LAYER, KEY(PAUSE)),

    [KEY(F3)] = KEY(F16),
    [KEY(F7)] = KEY(F17),
    [KEY(F11)] = KEY(F18),

    [KEY(F4)] = KEY(F19),
    [KEY(F8)] = KEY(F20),
    [KEY(F12)] = KEY(F21),

    [KEY(KP_A)] = KEY(F22),
    [KEY(KP_B)] = KEY(F23),
    [KEY(KP_C)] = KEY(F24),

    // The F-keys arranged in groups of 3 with F1-F9 same as the keypad 1-9
    [KEY(F13)] = KEY(F10),
    [KEY(F17)] = KEY(F11),
    [KEY(F21)] = KEY(F12),

    [KEY(F14)] = KEY(F7),
    [KEY(F18)] = KEY(F8),
    [KEY(F22)] = KEY(F9),

    [KEY(F15)] = KEY(F4),
    [KEY(F19)] = KEY(F5),
    [KEY(F23)] = KEY(F6),

    [KEY(F16)] = KEY(F1),
    [KEY(F20)] = KEY(F2),
    [KEY(F24)] = KEY(F3),

#if SPLIT_PAD_PLUS
    // If the plus is split, use the Mac layout (but change = to backspace)
    [KEY(KP_DIVIDE)] = KEY(BACKSPACE),
    [KEY(KP_MULTIPLY)] = KEY(KP_DIVIDE),
    [KEY(KP_MINUS)] = KEY(KP_MULTIPLY),
    [KEY(KP_PLUS)] = KEY(KP_MINUS),
    [KEY(BACKSPACE)] = KEY(KP_PLUS),
#endif

#if SPLIT_PAD_ZERO
    [KEY(KP_00)] = MACRO(MACRO_TYPE_00),
#endif
};
#endif

#if LAYER_COUNT >= LINUX_LAYER
DEFINE_LAYER(LINUX_LAYER) {
#if ENABLE_MEDIA_KEYS
    [KEY(F4)] = KEY(VOLUME_MUTE),
    [KEY(F8)] = KEY(VOLUME_DOWN),
    [KEY(F12)] = KEY(VOLUME_UP),
    [KEY(KP_A)] = KEY(PREVIOUS_TRACK),
    [KEY(KP_B)] = KEY(PLAY_PAUSE),
    [KEY(KP_C)] = KEY(NEXT_TRACK),
#endif
};
#endif

#if LAYER_COUNT >= APPLE_LAYER
DEFINE_LAYER(APPLE_LAYER) {
#if ENABLE_APPLE_FN_KEY
    // Insert does nothing useful on Mac
    [KEY(F1)] = KEY_APPLE_FN,
#else
    [KEY(F1)] = KEY(DELETE),
#endif

    // On Mac the Print Screen, Scroll Lock, and Pause/Break don't really
    // exist so let's just put F13-F15 here for more comfortable bindings
    [KEY(KP_D)] = KEY(F13),
    [KEY(KP_E)] = KEY(F14),
    [KEY(KP_F)] = LAYER_OR_PLAIN_KEY(FN_LAYER, KEY(F15)),

#if ENABLE_MEDIA_KEYS
    [KEY(F4)] = KEY(VOLUME_MUTE),
    [KEY(F8)] = KEY(VOLUME_DOWN),
    [KEY(F12)] = KEY(VOLUME_UP),
    [KEY(KP_A)] = KEY(PREVIOUS_TRACK),
    [KEY(KP_B)] = KEY(PLAY_PAUSE),
    [KEY(KP_C)] = KEY(NEXT_TRACK),
#else
    [KEY(F4)] = KEY(F10),
    [KEY(F8)] = KEY(F11),
    [KEY(F12)] = KEY(F12),
    [KEY(KP_A)] = KEY(F7),
    [KEY(KP_B)] = KEY(F8),
    [KEY(KP_C)] = KEY(F9),
#endif
};
#endif

#if LAYER_COUNT >= NUM_LOCK_LAYER
DEFINE_LAYER(NUM_LOCK_LAYER) {
    [KEY(KP_7_HOME)] = KEY(HOME),
    [KEY(KP_8_UP)] = KEY(UP_ARROW),
    [KEY(KP_9_PAGE_UP)] = KEY(PAGE_UP),

    [KEY(KP_4_LEFT)] = KEY(LEFT_ARROW),
    [KEY(KP_5)] = KEY(DOWN_ARROW),
    [KEY(KP_6_RIGHT)] = KEY(RIGHT_ARROW),

    [KEY(KP_1_END)] = KEY(END),
    [KEY(KP_2_DOWN)] = KEY(DOWN_ARROW),
    [KEY(KP_3_PAGE_DOWN)] = KEY(PAGE_DOWN),

    [KEY(KP_0_INSERT)] = KEY(INSERT),
    [KEY(KP_COMMA_DEL)] = KEY(DELETE),

#if SPLIT_PAD_PLUS
    [KEY(KP_MULTIPLY)] = KEY(DELETE),
    [KEY(KP_MINUS)] = KEY(ALT_GR),
    [KEY(KP_PLUS)] = KEY(RIGHT_SHIFT),
    [KEY(BACKSPACE)] = KEY(SPACE),
#else
    [KEY(KP_DIVIDE)] = KEY(BACKSPACE),
    [KEY(KP_MULTIPLY)] = KEY(ALT_GR),
    [KEY(KP_MINUS)] = KEY(RIGHT_SHIFT),
    [KEY(KP_PLUS)] = KEY(SPACE),
#endif
};
#endif

#if LAYER_COUNT >= FN_LAYER
DEFINE_LAYER(FN_LAYER) {
    [KEY(F1)] = MACRO(MACRO_CYCLE_OS_LAYERS),
    [KEY(NUM_LOCK)] = LAYER_TOGGLE(NUM_LOCK_LAYER),

    [KEY(KP_ENTER)] = EXT(ENTER_BOOTLOADER),

#if ENABLE_SIMULATED_TYPING
    [KEY(F5)] = EXT(PRINT_DEBUG_INFO),
    [KEY(F9)] = MACRO(MACRO_DEBUG_CALIBRATION),
#endif

    [KEY(KP_A)] = MACRO(MACRO_SAVE_CALIBRATION),
    [KEY(KP_B)] = MACRO(MACRO_UNSAVE_CALIBRATION),
};
#endif
