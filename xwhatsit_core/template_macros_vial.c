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
#include <qmk_core/matrix.h>
#include <qmk_core/platforms/timer.h>
#include <xwhatsit_core/matrix_manipulate.h>
#include "qmk_translate.h"
#include "stdio.h"

#if ENABLE_PS2_DEVICE
#include "ps2_output.h"
#endif

#define ESC_ROW 4
#define ESC_COL 0

#ifndef KK_LAYERS_H
// Note that the the `enum macro` is defined in `layers_vial.c`. This is to
// ensure that the two files agree on macro names.
#include "layers_vial.c"
#endif

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

static void matrix_print_calibration_stats(void) {
#if CAPSENSE_CAL_DEBUG
    fprintf_P(usb_kbd_type, PSTR("Calibration %u ms\n"), cal_time);
#endif
    fprintf_P(usb_kbd_type, PSTR("Done=%d Load=%d Save=%d Skip=%d Doubt=%d flags=%02X\n"), calibration_done, calibration_loaded, calibration_saved, calibration_skipped, calibration_unreliable, cal_flags);
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

/// This function is called after resolving the keycode of a pressed key from
/// the currently active layers. It can change the keycode and/or have any
/// side effects wanted. A single byte of data is available to store state
/// information for this specific keypress. The same byte is used for macros
/// in `execute_macro` and for the release `postprocess_release`.
static inline keycode_t preprocess_press(keycode_t keycode, uint8_t physical_key, uint8_t layer, uint8_t *  restrict data) {
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
            matrix_print_calibration_stats();
        }
        break;

    case MACRO_CAL_SAVE:
        if (is_release) {
            save_matrix_calibration();
        }
        break;

    case MACRO_CAL_DELETE:
        if (is_release) {
            clear_saved_matrix_calibration();
            usb_keyboard_simulate_keypress(USB_KEY_SPACE, 0);
        }
        break;

    case MACRO_CAL_RECAL:
        if (is_release) {
#if CAPSENSE_CAL_DEBUG
            cal_time = timer_read();
#endif
            calibrate_matrix();
#if CAPSENSE_CAL_DEBUG
            cal_time = timer_elapsed(cal_time);
#endif
#if ENABLE_SIMULATED_TYPING
            matrix_print_calibration_stats();
#endif
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
static void handle_reset(void) {
    clear_override_leds();

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
            break;
        case HOST_OS_WINDOWS:
            enable_layers_with_macro(MACRO_WIN_LAYER);
            break;
        case HOST_OS_MACOS:
            enable_layers_with_macro(MACRO_MAC_LAYER);
            break;
        default:
            return;
    }
    host_fingerprint_stop_notifications();
}
#endif
