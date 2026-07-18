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

#define FN_LAYER        STATIC_LAYER_1
#define SYSTEM_LAYER    STATIC_LAYER_2
#define LAYER_COUNT     SYSTEM_LAYER

enum macro {
    MACRO_CAL_SAVE,
    MACRO_CAL_DELETE,
    MACRO_CAL_RECAL,
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

// This is actually a "Num Lock off" -layer, but named "FN_LAYER" so that on
// non-Apple builds if someone maps the Fn key (which is Apple-exclusive), it
// will at least activate this layer and do something. (This is harmless
// because otherwise the key would do nothing and with Apple builds it will
// act as the Apple Fn key without activating this layer.)
DEFINE_LAYER(FN_LAYER) {
    [KEY(KP_7_HOME)] = KEY(HOME),
    [KEY(KP_8_UP)] = KEY(UP_ARROW),
    [KEY(KP_9_PAGE_UP)] = KEY(PAGE_UP),

    [KEY(KP_4_LEFT)] = KEY(LEFT_ARROW),
    [KEY(KP_5)] = MACRO(MACRO_READ_ONLY_LAYER),
    [KEY(KP_6_RIGHT)] = KEY(RIGHT_ARROW),

    [KEY(KP_1_END)] = KEY(END),
    [KEY(KP_2_DOWN)] = KEY(DOWN_ARROW),
    [KEY(KP_3_PAGE_DOWN)] = KEY(PAGE_DOWN),

    [KEY(KP_0_INSERT)] = KEY(INSERT),
    [KEY(0)] = KEY(INSERT),
    [KEY(KP_COMMA_DEL)] = KEY(DELETE),
};

DEFINE_LAYER(SYSTEM_LAYER) {
    [KC_R3C1] = EXT(ENTER_BOOTLOADER),
    [KC_R3C2] = EXT(RESET_KEYBOARD),
    [KC_R3C3] = EXT(TOGGLE_BOOT_PROTOCOL),

    [KC_R4C1] = MACRO(MACRO_CAL_RECAL),
    [KC_R4C2] = MACRO(MACRO_CAL_SAVE),
    [KC_R4C3] = MACRO(MACRO_CAL_DELETE),

    [KC_R5C1] = MACRO(MACRO_DEBUG),
    [KC_R5C2] = EXT(EEPROM_RESET),
    [KC_R5C3] = LAYER_SET_BASE(0),

    [KEY(KP_5)] = MACRO(MACRO_READ_ONLY_LAYER),
};
