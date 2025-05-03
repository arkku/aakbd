#include <layers.h>
#include "keymap.h"

#define DEFAULT_BASE_LAYER 1
#define WINDOWS_LAYER 2
#define NUM_LOCK_LAYER 3
#define FN_LAYER 4

#define LAYER_COUNT FN_LAYER

enum macro {
    MACRO_NOP,
    MACRO_FALLTHROUGH,
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
    [KEY(F1)] = KEY(BACKSPACE),
    [KEY(F2)] = KEY(DELETE),

    [KEY(F5)] = KEY(HOME),
    [KEY(F6)] = KEY(END),

    [KEY(F9)] = KEY(PAGE_UP),
    [KEY(F10)] = KEY(PAGE_DOWN),

    [KEY(F3)] = KEY(F13),
    [KEY(F7)] = KEY(F14),
    [KEY(F11)] = KEY(F15),

#if ENABLE_MEDIA_KEYS
    [KEY(F4)] = KEY(VOLUME_MUTE),
    [KEY(F8)] = KEY(VOLUME_DOWN),
    [KEY(F12)] = KEY(VOLUME_UP),
#else
    [KEY(F4)] = KEY(F10),
    [KEY(F8)] = KEY(F11),
    [KEY(F12)] = KEY(F12),
#endif

#if ENABLE_MEDIA_KEYS
    [KEY(KP_A)] = KEY(PREVIOUS_TRACK),
    [KEY(KP_B)] = KEY(PLAY_PAUSE),
    [KEY(KP_C)] = KEY(NEXT_TRACK),
#else
    [KEY(KP_A)] = KEY(F7),
    [KEY(KP_B)] = KEY(F8),
    [KEY(KP_C)] = KEY(F9),
#endif

    [KEY(F13)] = KEY(F1),
    [KEY(F14)] = KEY(F2),
    [KEY(F15)] = KEY(F3),
    [KEY(F16)] = KEY(F4),
    
    [KEY(F17)] = KEY(F5),
    [KEY(F18)] = KEY(F6),
    [KEY(F19)] = KEY(F7),
    [KEY(F20)] = KEY(F8),

    [KEY(F21)] = KEY(F9),
    [KEY(F22)] = KEY(F10),
    [KEY(F23)] = KEY(F11),
    [KEY(F24)] = KEY(F12),

    [KEY(KP_D)] = KEY(F1),
    [KEY(KP_E)] = KEY(F2),
    [KEY(KP_F)] = LAYER_OR_PLAIN_KEY(FN_LAYER, KEY(PAUSE)),

#if SPLIT_PAD_PLUS
    // If the plus is split, add the equals sign
    [KEY(KP_DIVIDE)] = KEY(KP_EQUALS),
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

#if LAYER_COUNT >= WINDOWS_LAYER
DEFINE_LAYER(WINDOWS_LAYER) {
    [KEY(F1)] = KEY(INSERT),

    [KEY(F4)] = KEY(F19),
    [KEY(F8)] = KEY(F20),
    [KEY(F12)] = KEY(F21),

    [KEY(KP_A)] = KEY(F22),
    [KEY(KP_B)] = KEY(F23),
    [KEY(KP_C)] = KEY(F24),

    [KEY(KP_D)] = KEY(PRINT_SCREEN),
    [KEY(KP_E)] = KEY(SCROLL_LOCK),
    [KEY(KP_F)] = LAYER_OR_PLAIN_KEY(FN_LAYER, KEY(PAUSE)),

#if SPLIT_PAD_PLUS
    // The KP_EQUALS key doesn't seem to work on Windows
    [KEY(KP_DIVIDE)] = KEY(BACKSPACE),
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
};
#endif

#if LAYER_COUNT >= FN_LAYER
DEFINE_LAYER(FN_LAYER) {
    [KEY(F1)] = LAYER_TOGGLE(WINDOWS_LAYER),
    [KEY(NUM_LOCK)] = LAYER_TOGGLE(NUM_LOCK_LAYER),

    [KEY(KP_ENTER)] = EXT(ENTER_BOOTLOADER),

#if ENABLE_SIMULATED_TYPING
    [KEY(F13)] = EXT(PRINT_DEBUG_INFO),
    [KEY(F14)] = MACRO(MACRO_DEBUG_CALIBRATION),
#endif

    [KEY(F5)] = MACRO(MACRO_SAVE_CALIBRATION),
    [KEY(F8)] = MACRO(MACRO_UNSAVE_CALIBRATION),

    [KEY(F17)] = MACRO(MACRO_SAVE_CALIBRATION),
    [KEY(F20)] = MACRO(MACRO_UNSAVE_CALIBRATION),
};
#endif
