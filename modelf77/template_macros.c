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

#include "config.h"
#include <qmk_core/matrix.h>
#include <xwhatsit_core/matrix_manipulate.h>

#ifndef KK_LAYERS_H
// Note that the the `enum macro` is defined in `layers.c`. This is to ensure
// that `layers.c` and `macros.c` agree on the macro names/numbers.
#include "layers.c"
#endif

#if ENABLE_APPLE_FN_KEY
static bool is_weak_apple_fn_pressed = false;

static inline void press_weak_apple_fn(void) {
    if (!is_apple_virtual_pressed(USB_KEY_VIRTUAL_APPLE_FN)) {
        press_apple_virtual(USB_KEY_VIRTUAL_APPLE_FN);
        is_weak_apple_fn_pressed = true;
    }
}

static inline bool release_weak_apple_fn(void) {
    if (is_weak_apple_fn_pressed) {
        release_apple_virtual(USB_KEY_VIRTUAL_APPLE_FN);
        is_weak_apple_fn_pressed = false;
        return true;
    } else {
        return false;
    }
}
#else
#define press_weak_apple_fn(x)      do { } while (0)
#define release_weak_apple_fn(x)    false
#endif

/// This function is called after resolving the keycode of a pressed key from
/// the currently active layers. It can change the keycode and/or have any
/// side effects wanted. A single byte of data is available to store state
/// information for this specific keypress. The same byte is used for macros
/// in `execute_macro` and for the release `postprocess_release`.
static inline keycode_t preprocess_press(keycode_t keycode, uint8_t physical_key, uint8_t * restrict data) {
#if ENABLE_APPLE_FN_KEY
    if (is_layer_active(APPLE_FN_LAYER) && !is_layer_active(WINDOWS_FN_LAYER) && keycode >= KEY(F1) && keycode <= KEY(F12) && physical_key >= KEY(F1)) {
        // Combine real F-keys with Apple Fn on the virtual Fn layer
        press_weak_apple_fn();
    } else if (keycode == KEY_APPLE_FN) {
        is_weak_apple_fn_pressed = false; // Real Apple Fn makes weak strong
    } else if (release_weak_apple_fn()) {
        // Release the weak Apple Fn when pressing any other key
        usb_keyboard_send_if_needed();
    }
#endif
    return keycode;
}

/// This function is called after all handlers of a key release have been
/// called. This is the counterpart to `preprocess_press`, and can be used to
/// clean up any state. The single byte of data is the same as was written by
/// `preprocess_press` and/or any macro handlers.
static inline void postprocess_release(keycode_t keycode, uint8_t physical_key, uint8_t data) {
}


#if ENABLE_SIMULATED_TYPING
static void matrix_print_calibration_stats(void) {
#if CAPSENSE_CAL_DEBUG
    fprintf_P(usb_kbd_type, PSTR("Calibration %u ms\n"), cal_time);
#endif
    fprintf_P(usb_kbd_type, PSTR("Load=%d Save=%d Skip=%d\n"), calibration_loaded, calibration_saved, calibration_skipped);
    fprintf_P(usb_kbd_type, PSTR("All 0 = %u, All 1 = %u\n"), cal_tr_all_zero, cal_tr_all_one);

    for (int_fast8_t bin = 0; bin < CAPSENSE_CAL_BINS; ++bin) {
        fprintf_P(usb_kbd_type, PSTR("Bin %u, threshold=%u keys=%u\n"), bin, cal_thresholds[bin], cal_bin_key_count[bin]);
        for (int_fast8_t row = 0; row < MATRIX_CAPSENSE_ROWS; ++row) {
            fprintf_P(usb_kbd_type, PSTR("Row %u 0x%04X\n"), row, assigned_to_threshold[bin][row]);
        }
    }

    uint16_t scan_time = timer_read();
    for (int_fast8_t i = 100; i; --i) {
        (void) matrix_scan();
    }
    scan_time = timer_elapsed(scan_time);
    fprintf_P(usb_kbd_type, PSTR("Scan %u.%02u ms\n"), scan_time / 100, scan_time % 100);
}
#endif

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

    case MACRO_SAVE_CALIBRATION:
        if (is_release) {
            save_matrix_calibration();
        }
        break;

    case MACRO_UNSAVE_CALIBRATION:
        if (is_release) {
            clear_saved_matrix_calibration();
        }
        break;

    case MACRO_DEBUG_CALIBRATION:
        if (is_release) {
            matrix_print_calibration_stats();
        }
        break;

    case MACRO_WEAK_APPLE_FN:
        if (is_release) {
            release_weak_apple_fn();
            if (*data) {
                disable_layer(*data);
            }
        } else {
            press_weak_apple_fn();
            const uint8_t layer = APPLE_FN_LAYER;
            if (!is_layer_active(layer)) {
                enable_layer(layer);
                *data = layer;
            }
        }
        break;

    default:
        break;
    }
}


/// Called after enabling or disabling a layer.
/// This can be used to do things like add/remove modifiers based on the state
/// of a layer, or override LEDSs.
static inline void layer_state_changed(uint8_t layer, bool is_enabled) {
    if (is_enabled) {
        switch (layer) {
#ifdef NUM_LOCK_LAYER
        case NUM_LOCK_LAYER:
            add_override_leds_on(LED_NUM_LOCK_BIT);
            break;
#endif
        case APPLE_FN_LAYER:
            add_override_leds_on(LED_SCROLL_LOCK_BIT);
            break;
        case WINDOWS_FN_LAYER:
            enable_layer(APPLE_FN_LAYER);
            break;
        default:
            break;
        }
    } else {
        switch (layer) {
#ifdef NUM_LOCK_LAYER
        case NUM_LOCK_LAYER:
            remove_override_leds_on(LED_NUM_LOCK_BIT);
            break;
#endif
        case APPLE_FN_LAYER:
            remove_override_leds_on(LED_SCROLL_LOCK_BIT);
            (void) release_weak_apple_fn(); // Make sure it gets released
            break;
        case WINDOWS_FN_LAYER:
            disable_layer(APPLE_FN_LAYER);
            break;
        default:
            break;
        }
    }
}

/// Called after the keyboard has been reset. This can be used to override the
/// default initial state, e.g., set custom layers mask, load configuration
/// from EEPROM, etc.
static inline void handle_reset(void) {
    is_weak_apple_fn_pressed = false;
    clear_override_leds();
    add_override_leds_off(LED_NUM_LOCK_BIT); // Capture Num Lock LED
}

/// Called approximately once every 10 milliseconds with an 8-bit time value.
/// Long macros and simulated typing can cause this to be called less
/// frequently, since this is not an interrupt.
static inline void handle_tick(uint8_t tick_10ms_count) {
}

/// Called when USB host LED state changes.
static inline void keyboard_host_leds_changed(uint8_t leds) {
}
