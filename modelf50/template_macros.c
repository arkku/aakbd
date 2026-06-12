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
#include <qmk_core/platforms/timer.h>
#include <xwhatsit_core/matrix_manipulate.h>

#if HAPTIC_ENABLE
#include <qmk_core/haptic.h>
#endif

#ifndef KK_LAYERS_H
// Note that the the `enum macro` is defined in `layers.c`. This is to ensure
// that `layers.c` and `macros.c` agree on the macro names/numbers.
#include "layers.c"
#endif

#if ENABLE_APPLE_FN_KEY
#define APPLE_FN_WEAK 1
#define APPLE_FN_STRONG 2

static uint8_t apple_fn_pressed = 0;

#define is_weak_apple_fn_pressed()      (apple_fn_pressed == APPLE_FN_WEAK)
#define is_strong_apple_fn_pressed()    (apple_fn_pressed == APPLE_FN_STRONG)

static inline void press_weak_apple_fn(void) {
    if (!is_virtual_pressed(USB_KEY_VIRTUAL_APPLE_FN)) {
        press_virtual(USB_KEY_VIRTUAL_APPLE_FN);
        apple_fn_pressed = APPLE_FN_WEAK;
    }
}

static inline bool release_weak_apple_fn(void) {
    if (is_weak_apple_fn_pressed()) {
        release_virtual(USB_KEY_VIRTUAL_APPLE_FN);
        if (apple_fn_pressed == APPLE_FN_WEAK) {
            apple_fn_pressed = 0;
        }
        return true;
    } else {
        return false;
    }
}

static inline void press_strong_apple_fn() {
    if (!is_virtual_pressed(USB_KEY_VIRTUAL_APPLE_FN)) {
        press_virtual(USB_KEY_VIRTUAL_APPLE_FN);
    }
    apple_fn_pressed = APPLE_FN_STRONG;
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
    if (physical_key >= KEY(F13) && physical_key <= KEY(F24)
        && !(is_layer_active(WINDOWS_LAYER) || is_layer_active(FN_LAYER))
    ) {
        // Use as F-keys (F1-F12, which have been moved to the second block)
        if (is_strong_apple_fn_pressed()) {
            *data = APPLE_FN_STRONG;
            release_virtual(USB_KEY_VIRTUAL_APPLE_FN);
            usb_keyboard_send_if_needed();
        } else {
            *data = APPLE_FN_WEAK;
            press_weak_apple_fn();
        }
        return keycode;
    } else if (keycode == KEY_APPLE_FN) {
        apple_fn_pressed = APPLE_FN_STRONG; // Real Apple Fn makes weak strong
    } else if (release_weak_apple_fn()) {
        // Release the weak Apple Fn when pressing any other key
        usb_keyboard_send_if_needed();
    }
    *data = 0;
#endif
    return keycode;
}

/// This function is called after all handlers of a key release have been
/// called. This is the counterpart to `preprocess_press`, and can be used to
/// clean up any state. The single byte of data is the same as was written by
/// `preprocess_press` and/or any macro handlers.
static inline void postprocess_release(keycode_t keycode, uint8_t physical_key, uint8_t data) {
#if ENABLE_APPLE_FN_KEY
    if (data && physical_key >= KEY(F13) && physical_key <= KEY(F24)) {
        if (data == APPLE_FN_WEAK && release_weak_apple_fn()) {
            // Release the weak Apple Fn after releasing the F-key
            usb_keyboard_send_if_needed();
        } else if (data == APPLE_FN_STRONG && is_strong_apple_fn_pressed() && !is_virtual_pressed(USB_KEY_VIRTUAL_APPLE_FN)) {
            // Press the strong Apple Fn again after releasing the F-key
            press_virtual(USB_KEY_VIRTUAL_APPLE_FN);
            usb_keyboard_send_if_needed();
        }
    } else if (keycode == KEY_APPLE_FN) {
        apple_fn_pressed = 0;
    }
#endif
}


#if ENABLE_SIMULATED_TYPING
static void matrix_print_calibration_stats(void) {
#if CAPSENSE_CAL_DEBUG
    fprintf_P(usb_kbd_type, PSTR("Calibration %u ms\n"), cal_time);
#endif
    fprintf_P(usb_kbd_type, PSTR("Cal=%d Load=%d Save=%d Skip=%d Doubt=%d flags=%02X\n"), calibration_done, calibration_loaded, calibration_saved, calibration_skipped, calibration_unreliable, cal_flags);
    fprintf_P(usb_kbd_type, PSTR("Min = %u, Max = %u, Offset = %u\n"), cal_threshold_min, cal_threshold_max, cal_threshold_offset);

    uint16_t scan_time = timer_read();
    for (int_fast8_t i = 100; i; --i) {
        (void) matrix_scan();
    }
    scan_time = timer_elapsed(scan_time);
    fprintf_P(usb_kbd_type, PSTR("Scan time %u.%02u ms\n"), scan_time / 100, scan_time % 100);

    for (int_fast8_t bin = 0; bin < CAPSENSE_CAL_BINS; ++bin) {
        fprintf_P(usb_kbd_type, PSTR("Bin %u, threshold=%u keys=%u\n"), bin, cal_thresholds[bin], cal_bin_key_count[bin]);
        for (int_fast8_t row = 0; row < MATRIX_CAPSENSE_ROWS; ++row) {
            fprintf_P(usb_kbd_type, PSTR("Row %u 0x%04X\n"), row, assigned_to_threshold[bin][row]);
        }
    }
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

    case MACRO_TYPE_00:
        if (!is_release) {
            *data = KEY(KP_0_INSERT);
        }
        register_key(*data, is_release);
        usb_keyboard_simulate_keypress(*data, 0);
        break;

    case MACRO_TOGGLE_SOLENOID:
#if HAPTIC_ENABLE
        haptic_toggle();
#endif
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
            add_override_leds_off(LED_NUM_LOCK_BIT);
            break;
#endif
        default:
            break;
        }
    } else {
        switch (layer) {
#ifdef NUM_LOCK_LAYER
        case NUM_LOCK_LAYER:
            remove_override_leds_off(LED_NUM_LOCK_BIT);
            break;
#endif
        default:
            break;
        }
    }
}

/// Called after the keyboard has been reset. This can be used to override the
/// default initial state, e.g., set custom layers mask, load configuration
/// from EEPROM, etc.
static inline void handle_reset(void) {
    clear_override_leds();
#if ENABLE_APPLE_FN_KEY
    apple_fn_pressed = 0;
#endif
}

/// Called approximately once every 10 milliseconds with an 8-bit time value.
/// Long macros and simulated typing can cause this to be called less
/// frequently, since this is not an interrupt.
static inline void handle_tick(uint8_t tick_10ms_count) {
}

/// Called when USB host LED state changes.
static inline void keyboard_host_leds_changed(uint8_t leds) {
}
