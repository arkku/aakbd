/**
 * macros_vial.c: Macro definitions.
 *
 * https://github.com/arkku/aakbd
 *
 * The local file macros_vial.c is ignored by Git so customisation can be
 * done there without being overwritten. The file template_macros_vial.c
 * contains an example set of macros.
 *
 * See the file "macros.h" for some of the functions available. You can also
 * call functions from "usbkbd.h" for really low level access. And of course
 * do anything at all; macros are arbitrary programs...
 */

#include <macros.h>

#include "config.h"
#include "led_map.h"
#include "rgb_matrix.h"
#include "timer.h"
#include "qmk_translate.h"
#include "qmk_keycodes.h"

#if ENABLE_PS2_DEVICE
#include "ps2_output.h"
#endif

// The "Esc" key coordinates are actually row 1 col 3, but col 6 is the
// backtick key that we have mapped to "Esc Grave" on the static FN_LAYER.
// This key mapping in EEPROM determines whether it is backtick, if yes,
// Esc will be produced, otherwise backtick will be produced on Fn layer.
#define ESC_ROW 1
#define ESC_COL 6

#define NUM_LOCK_LIGHT_KEY USB_KEY_ESC
#define CAPS_LOCK_LIGHT_KEY USB_KEY_CAPS_LOCK
#define SCROLL_LOCK_LIGHT_KEY USB_KEY_PRINT_SCREEN

#ifndef KK_LAYERS_H
// Note that the the `enum macro` is defined in `layers_vial.c`. This is to
// ensure that the two files agree on macro names.
#include "layers_vial.c"
#endif

enum gradient {
    GRADIENT_BLUE_WHITE,
    GRADIENT_RED_BLUE,
    GRADIENT_GREEN_BLUE,
};

static bool rgb_enabled = true;
static bool gradient_reversed = false;
static enum gradient gradient_scheme = GRADIENT_BLUE_WHITE;

#if ENABLE_APPLE_FN_KEY
static bool is_weak_apple_fn_pressed = false;

static void press_weak_apple_fn(void) {
    if (!is_virtual_pressed(USB_KEY_VIRTUAL_APPLE_FN)) {
        press_virtual(USB_KEY_VIRTUAL_APPLE_FN);
        is_weak_apple_fn_pressed = true;
    }
}

static bool release_weak_apple_fn(void) {
    if (is_weak_apple_fn_pressed) {
        release_virtual(USB_KEY_VIRTUAL_APPLE_FN);
        is_weak_apple_fn_pressed = false;
        return true;
    } else {
        return false;
    }
}
#endif

static inline keycode_t preprocess_press(keycode_t keycode, uint8_t physical_key, uint8_t layer, uint8_t *  restrict data) {
    if (rgb_enabled) {
        rgb_led_set_by_keycode(physical_key, 128, 192, 224);
    }

#if ENABLE_APPLE_FN_KEY
    if (keycode == KEY_APPLE_FN) {
        // Real Apple Fn makes weak strong
        is_weak_apple_fn_pressed = false;
    } else if (layer < FN_LAYER && is_layer_enabled(FN_LAYER)) {
        // A passthrough key on Fn layer will be used for Fn-combos
        press_weak_apple_fn();
    } else if (release_weak_apple_fn()) {
        // Release the weak Apple Fn when pressing any other key
        usb_keyboard_send_if_needed();
    }
#endif

    if (physical_key && keycode >= MACRO(MACRO_READ_ONLY_LAYER) && keycode < MACRO(COUNT_OF_MACROS)) {
        // The marker macros just fall through to the key's default function
        return physical_key;
    }

    return keycode;
}

/// This function is called after all handlers of a key release have been
/// called. This is the counterpart to `preprocess_press`, and can be used to
/// clean up any state. The single byte of data is the same as was written by
/// `preprocess_press` and/or any macro handlers.
static inline void postprocess_release(keycode_t keycode, uint8_t physical_key, uint8_t data) {
    if (physical_key == NUM_LOCK_LIGHT_KEY || physical_key == CAPS_LOCK_LIGHT_KEY || physical_key == SCROLL_LOCK_LIGHT_KEY) {
        keyboard_host_leds_changed(usb_keyboard_led_state());
    } else if (rgb_enabled) {
        rgb_led_set_by_keycode(physical_key, 0, 0, 0);
    }
}

/// This function is called to execute macro keycodes. Macros are implemented
/// as actual code, so you can do pretty much anything with them.
static void execute_macro(uint8_t macro_number, bool is_release, uint8_t physical_key, uint8_t layer, uint8_t * restrict data) {
    const enum macro macro = macro_number;

    switch (macro) {
    case MACRO_ESC_GRAVE:
        // If the Key at the Esc position is backtick, then this key is Esc,
        // otherwise this key is backtick.
        if (is_release) {
            usb_keyboard_release(*data);
        } else {
            const uint16_t normal_esc_keycode = vial_get_keycode_at(ESC_ROW, ESC_COL, 1);
            *data = (normal_esc_keycode == KEY(BACKTICK)) ? KEY(ESC) : KEY(BACKTICK);
            usb_keyboard_press(*data);
        }
        break;

    case MACRO_DEBUG:
        if (is_release) {
            usb_keyboard_type_debug_report();
        }
        break;

    case MACRO_LAYER_FN:
        if (is_release) {
#if ENABLE_APPLE_FN_KEY
            release_weak_apple_fn();
#endif
            if (*data) {
                disable_layer(*data);
            }
        } else {
#if ENABLE_APPLE_FN_KEY
            press_weak_apple_fn();
#endif
            layer = FN_LAYER;
            if (!is_layer_enabled(layer)) {
                enable_layer(layer);
                *data = layer;
            } else {
                *data = 0;
            }
        }
        break;

    case MACRO_UNSWAP_ALL:
        if (is_release) {
            vial_magic_save(0);
        }
        break;

    default:
        if (is_release) {
            usb_keyboard_release(physical_key ? physical_key : *data);
        } else {
            usb_keyboard_press(physical_key);
            *data = physical_key;
        }
        break;
    }
}

#if ENABLE_HOST_FINGERPRINT || ENABLE_PS2_DEVICE
static void enable_layers_with_macro(uint8_t macro_num) {
    const uint16_t target = MACRO(macro_num);
    for (uint8_t layer = 2; layer <= VIAL_LAYER_COUNT; ++layer) {
        for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
            for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
                if (qmk_to_aakbd(dynamic_keymap_get_qmk_keycode(layer - 1, row, col)) == target) {
                    enable_layer(layer);
                    goto next_vial_layer;
                }
            }
        }
        next_vial_layer:;
    }
}
#endif

static void led_set_gradient(bool is_reversed, enum gradient scheme) {
    gradient_reversed = is_reversed;
    gradient_scheme = scheme;

    if (!rgb_enabled) {
        return;
    }

    static const uint8_t left[8] = {
        LED_EDGE_LEFT_1, LED_EDGE_LEFT_2, LED_EDGE_LEFT_3, LED_EDGE_LEFT_4,
        LED_EDGE_LEFT_5, LED_EDGE_LEFT_6, LED_EDGE_LEFT_7, LED_EDGE_LEFT_8,
    };
    static const uint8_t right[8] = {
        LED_EDGE_RIGHT_1, LED_EDGE_RIGHT_2, LED_EDGE_RIGHT_3, LED_EDGE_RIGHT_4,
        LED_EDGE_RIGHT_5, LED_EDGE_RIGHT_6, LED_EDGE_RIGHT_7, LED_EDGE_RIGHT_8,
    };

    switch (scheme) {
        default:
            // fallthrough
        case GRADIENT_RED_BLUE:
            rgb_led_set(left[is_reversed  ? 7 : 0], 255,  0,  16);
            rgb_led_set(right[is_reversed ? 7 : 0], 255,  0,  16);
            rgb_led_set(left[is_reversed  ? 6 : 1], 192,  8,  64);
            rgb_led_set(right[is_reversed ? 6 : 1], 192,  8,  64);
            rgb_led_set(left[is_reversed  ? 5 : 2], 160, 16,  96);
            rgb_led_set(right[is_reversed ? 5 : 2], 160, 16,  96);
            rgb_led_set(left[is_reversed  ? 4 : 3], 128, 24, 128);
            rgb_led_set(right[is_reversed ? 4 : 3], 128, 24, 128);
            rgb_led_set(left[is_reversed  ? 3 : 4],  96, 32, 158);
            rgb_led_set(right[is_reversed ? 3 : 4],  96, 32, 158);
            rgb_led_set(left[is_reversed  ? 2 : 5],  64, 40, 190);
            rgb_led_set(right[is_reversed ? 2 : 5],  64, 40, 190);
            rgb_led_set(left[is_reversed  ? 1 : 6],  32, 48, 222);
            rgb_led_set(right[is_reversed ? 1 : 6],  32, 48, 222);
            rgb_led_set(left[is_reversed  ? 0 : 7],   0, 56, 255);
            rgb_led_set(right[is_reversed ? 0 : 7],   0, 56, 255);
            break;
        case GRADIENT_GREEN_BLUE:
            rgb_led_set(left[is_reversed  ? 7 : 0],  0, 255,  16);
            rgb_led_set(right[is_reversed ? 7 : 0],  0, 255,  16);
            rgb_led_set(left[is_reversed  ? 6 : 1],  0, 192,  64);
            rgb_led_set(right[is_reversed ? 6 : 1],  0, 192,  64);
            rgb_led_set(left[is_reversed  ? 5 : 2],  0, 160,  96);
            rgb_led_set(right[is_reversed ? 5 : 2],  0, 160,  96);
            rgb_led_set(left[is_reversed  ? 4 : 3],  0, 128, 128);
            rgb_led_set(right[is_reversed ? 4 : 3],  0, 128, 128);
            rgb_led_set(left[is_reversed  ? 3 : 4],  0,  96, 158);
            rgb_led_set(right[is_reversed ? 3 : 4],  0,  96, 158);
            rgb_led_set(left[is_reversed  ? 2 : 5],  0,  64, 190);
            rgb_led_set(right[is_reversed ? 2 : 5],  0,  64, 190);
            rgb_led_set(left[is_reversed  ? 1 : 6],  0,  32, 222);
            rgb_led_set(right[is_reversed ? 1 : 6],  0,  32, 222);
            rgb_led_set(left[is_reversed  ? 0 : 7],  0,  16, 255);
            rgb_led_set(right[is_reversed ? 0 : 7],  0,  16, 255);
            break;
        case GRADIENT_BLUE_WHITE:
            rgb_led_set(left[is_reversed  ? 7 : 0],  32,  32, 222);
            rgb_led_set(right[is_reversed ? 7 : 0],  32,  32, 222);
            rgb_led_set(left[is_reversed  ? 6 : 1],  64,  64, 190);
            rgb_led_set(right[is_reversed ? 6 : 1],  64,  64, 190);
            rgb_led_set(left[is_reversed  ? 5 : 2],  96,  96, 158);
            rgb_led_set(right[is_reversed ? 5 : 2],  96,  96, 158);
            rgb_led_set(left[is_reversed  ? 4 : 3], 128, 128, 128);
            rgb_led_set(right[is_reversed ? 4 : 3], 128, 128, 128);
            rgb_led_set(left[is_reversed  ? 3 : 4],  96,  96, 158);
            rgb_led_set(right[is_reversed ? 3 : 4],  96,  96, 158);
            rgb_led_set(left[is_reversed  ? 2 : 5],  64,  64, 190);
            rgb_led_set(right[is_reversed ? 2 : 5],  64,  64, 190);
            rgb_led_set(left[is_reversed  ? 1 : 6],  16,  32, 222);
            rgb_led_set(right[is_reversed ? 1 : 6],  16,  32, 222);
            rgb_led_set(left[is_reversed  ? 0 : 7],   0,  16, 255);
            rgb_led_set(right[is_reversed ? 0 : 7],   0,  16, 255);
            break;
    }

}

/// Called after enabling or disabling a layer.
/// This can be used to do things like add/remove modifiers based on the state
/// of a layer, or override LEDSs.
static void layer_state_changed(uint8_t layer, bool is_enabled) {
    if (layer == FN_LAYER) {
        if (is_enabled) {
            add_override_leds_on(LED_SCROLL_LOCK_BIT);
        } else {
            remove_override_leds_on(LED_SCROLL_LOCK_BIT);
        }
    }
    if (layer > VIAL_LAYER_COUNT) {
        return;
    }
    for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
        for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
            const keycode_t key = qmk_to_aakbd(dynamic_keymap_get_qmk_keycode(layer - 1, row, col));
            if (key < MACRO(MACRO_NUM_LED_ON) || key > MACRO(MACRO_SCROLL_LED_ON)) {
                continue;
            }
            switch (key - MACRO(MACRO_NUM_LED_ON)) {
            case MACRO(MACRO_NUM_LED_ON) - MACRO(MACRO_NUM_LED_ON):
                if (is_enabled) {
                    add_override_leds_on(LED_NUM_LOCK_BIT);
                } else {
                    remove_override_leds_on(LED_NUM_LOCK_BIT);
                }
                break;
            case MACRO(MACRO_NUM_LED_OFF) - MACRO(MACRO_NUM_LED_ON):
                if (is_enabled) {
                    add_override_leds_off(LED_NUM_LOCK_BIT);
                } else {
                    remove_override_leds_off(LED_NUM_LOCK_BIT);
                }
                break;
            case MACRO(MACRO_CAPS_LED_ON) - MACRO(MACRO_NUM_LED_ON):
                if (is_enabled) {
                    add_override_leds_on(LED_CAPS_LOCK_BIT);
                } else {
                    remove_override_leds_on(LED_CAPS_LOCK_BIT);
                }
                break;
            case MACRO(MACRO_SCROLL_LED_ON) - MACRO(MACRO_NUM_LED_ON):
                if (is_enabled) {
                    add_override_leds_on(LED_SCROLL_LOCK_BIT);
                } else {
                    remove_override_leds_on(LED_SCROLL_LOCK_BIT);
                }
                break;
            }
        }
    }
}

/// Called after the keyboard has been reset. This can be used to override the
/// default initial state, e.g., set custom layers mask, load configuration
/// from EEPROM, etc.
static inline void handle_reset(void) {
    rgb_enabled = true;
    clear_override_leds();
    layer_state_changed(current_base_layer(), true);
    led_set_gradient(gradient_reversed, gradient_scheme);
    keyboard_host_leds_changed(usb_keyboard_led_state());

#if ENABLE_PS2_DEVICE
    if (ps2_output_is_initialized()) {
        enable_layers_with_macro(MACRO_PS2_LAYER);
    }
#endif
}

/// Called approximately once every 10 milliseconds with an 8-bit time value.
/// This function can be used to implement time-based behaviour, e.g., long
/// press effects. Long macros and simulated typing can cause this to be
/// called less frequently than every 10 ms, since this is not an interrupt;
/// do not assume the count sees every value.
static inline void handle_tick(uint8_t tick_10ms_count) {
}

/// Called when USB host LED state changes.
static inline void keyboard_host_leds_changed(uint8_t leds) {
    const bool is_error = (leds & LED_VIRTUAL_USB_ERROR_BIT) != 0;
    const bool is_active = (leds & LED_VIRTUAL_USB_ACTIVE_BIT) != 0;
    const bool is_boot = (leds & LED_VIRTUAL_BOOT_PROTOCOL_BIT) != 0;
    const bool is_fn = is_layer_enabled(FN_LAYER);

    if (leds & LED_NUM_LOCK_BIT) {
        rgb_led_set_by_keycode(USB_KEY_ESC, is_error ? 16 : 8, is_boot ? 128 : 32, is_active ? 64 : 32);
    } else {
        rgb_led_set_by_keycode(USB_KEY_ESC, is_error ? 64 : 0, is_boot ? 128 :  0, is_active ? 32 : 0);
    }
    if (leds & LED_CAPS_LOCK_BIT) {
        rgb_led_set_by_keycode(USB_KEY_CAPS_LOCK, 16, 128, is_fn ? 196 : 64);
    } else if (is_fn) {
        rgb_led_set_by_keycode(USB_KEY_CAPS_LOCK, 32, 64, 128);
    } else {
        rgb_led_set_by_keycode(USB_KEY_CAPS_LOCK, 0, 0, 0);
    }
    if (leds & LED_SCROLL_LOCK_BIT) {
        rgb_led_set_by_keycode(USB_KEY_PRINT_SCREEN, 0, 32, 128);
    } else {
        rgb_led_set_by_keycode(USB_KEY_PRINT_SCREEN, 0, 0, 0);
    }
}

/// Handle QMK RGB keycodes from EEPROM (RGBM On/Off/Toggle).
void process_qmk_keycode (uint16_t qmk_key, bool is_release) {
    if (is_release) {
        return;
    }
    if (qmk_key == QK_RGB_MATRIX_TOGGLE) {
        rgb_enabled = !rgb_enabled;
    } else {
        const bool should_enable = (qmk_key == QK_RGB_MATRIX_ON);
        if (rgb_enabled == should_enable) {
            return;
        }
        rgb_enabled = should_enable;
    }

    if (rgb_enabled) {
        led_set_gradient(gradient_reversed, gradient_scheme);
    } else {
        for (uint8_t i = 0; i < AW20216S_LED_COUNT; ++i) {
            rgb_led_set(i, 0, 0, 0);
        }
    }
    keyboard_host_leds_changed(usb_keyboard_led_state());
}

#if ENABLE_HOST_FINGERPRINT
#include "host_fingerprint.h"

/// If OS fingerprinting is enabled, this function is called when the
/// fingerprint is updated. Scans Vial layers for OS-specific macro markers
/// and enables any layer that contains a match.
void host_os_fingerprint_updated(uint8_t fingerprint) {
    switch (host_fingerprint_os_guess()) {
        case HOST_OS_LINUX:
            enable_layers_with_macro(MACRO_LINUX_LAYER);
            led_set_gradient(false, GRADIENT_RED_BLUE);
            break;
        case HOST_OS_WINDOWS:
            enable_layers_with_macro(MACRO_WIN_LAYER);
            led_set_gradient(true, GRADIENT_RED_BLUE);
            break;
        case HOST_OS_MACOS:
            enable_layers_with_macro(MACRO_MAC_LAYER);
            led_set_gradient(true, GRADIENT_GREEN_BLUE);
            break;
        default:
            return;
    }
    host_fingerprint_stop_notifications();
}
#endif
