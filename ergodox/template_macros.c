/**
 * macros.c: Macro definitions.
 *
 * https://github.com/arkku/aakbd
 *
 * The local file macros.c is ignored by Git so customisation can be done there
 * without being overwritten. The file template_macros.c contains an example
 * set of macros.
 *
 * See the file "macros.h" for some of the functions available. You can also
 * call functions from "usbkbd.h" for really low level access. And of course
 * do anything at all; macros are arbitrary programs...
 */
#include <macros.h>

#include "ergodox_ez.h"

#ifndef KK_LAYERS_H
// Note that the the `enum macro` is defined in `layers.c`. This is to ensure
// that `layers.c` and `macros.c` agree on the macro names/numbers.
#include "layers.c"
#endif

/// This function is called after resolving the keycode of a pressed key from
/// the currently active layers. It can change the keycode and/or have any
/// side effects wanted. A single byte of data is available to store state
/// information for this specific keypress. The same byte is used for macros
/// in `execute_macro` and for the release `postprocess_release`.
static inline keycode_t preprocess_press(keycode_t keycode, uint8_t physical_key, uint8_t * restrict data) {
    return keycode;
}

/// This function is called after all handlers of a key release have been
/// called. This is the counterpart to `preprocess_press`, and can be used to
/// clean up any state. The single byte of data is the same as was written by
/// `preprocess_press` and/or any macro handlers.
static inline void postprocess_release(keycode_t keycode, uint8_t physical_key, uint8_t data) {
}

#define LED_LEVEL_MAX 4

/// This function is called to execute macro keycodes. Macros are implemented
/// as actual code, so you can do pretty much anything with them.
static void execute_macro(uint8_t macro_number, bool is_release, uint8_t physical_key, uint8_t * restrict data) {
    const enum macro macro = macro_number;

    switch (macro) {
    case MACRO_NOP:
        break;

    case MACRO_FALLTHROUGH:
        register_key(physical_key, is_release);
        break;

    case MACRO_ERGODOX_LED_LEVEL_UP:
        // fallthrough
    case MACRO_ERGODOX_LED_LEVEL_DOWN:
        if (!is_release) {
            uint8_t level = keyboard_config.led_level;
            if (macro == MACRO_ERGODOX_LED_LEVEL_UP) {
                level = (level >= LED_LEVEL_MAX) ? 0 : (level + 1);
            } else {
                level = (level == 0) ? LED_LEVEL_MAX : (level - 1);
            }
            ergodox_led_all_set((level * 255) / LED_LEVEL_MAX);
            keyboard_config.led_level = level;
            eeconfig_update_kb(keyboard_config.raw);
        }
        break;

    default:
        break;
    }
}

static const uint8_t layer_leds[LAYER_COUNT + 1] = {
    [SYMBOL_LAYER] = LED_SCROLL_LOCK_BIT,
};

/// Called after enabling or disabling a layer.
/// This can be used to do things like add/remove modifiers based on the state
/// of a layer, or override LEDSs.
static inline void layer_state_changed(uint8_t layer, bool is_enabled) {
    uint8_t led = layer_leds[layer];

    if (led) {
        if (is_enabled) {
            add_override_leds_on(led);
        } else {
            remove_override_leds_on(led);
        }
    }
}

/// Called after the keyboard has been reset. This can be used to override the
/// default initial state, e.g., set custom layers mask, load configuration
/// from EEPROM, etc.
static inline void handle_reset(void) {
}

/// Called approximately once every 10 milliseconds with an 8-bit time value.
/// Long macros and simulated typing can cause this to be called less
/// frequently, since this is not an interrupt.
static inline void handle_tick(uint8_t tick_10ms_count) {
}

/// Called when USB host LED state changes.
static inline void keyboard_host_leds_changed(uint8_t leds) {
}
