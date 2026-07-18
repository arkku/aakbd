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

#ifndef KK_LAYERS_H
// Note that the the `enum macro` is defined in `layers.c`. This is to ensure
// that `layers.c` and `macros.c` agree on the macro names/numbers.
#include "layers.c"
#endif

#if ENABLE_APPLE_FN_KEY
/// The "weak Apple fn" acts like Apple Fn alone (e.g., for things like
/// double tap Apple Fn), but the other effects (like changing backspace to
/// delete) can be done with FN_LAYER, so we can release the Apple Fn there
/// (e.g., on keyboards without function keys pressing Fn + 1 is supposed to
/// be the same as pressing the plain F1 key, but without the _weak_ Apple Fn
/// it would always be like pressing Fn + F1).

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

/// This function is called after resolving the keycode of a pressed key from
/// the currently active layers. It can change the keycode and/or have any
/// side effects wanted. A single byte of data is available to store state
/// information for this specific keypress. The same byte is used for macros
/// in `execute_macro` and for the release `postprocess_release`.
static inline keycode_t preprocess_press(keycode_t keycode, uint8_t physical_key, uint8_t layer, uint8_t * restrict data) {
#if ENABLE_APPLE_FN_KEY
    if (keycode == KEY_APPLE_FN) {
        // Real Apple Fn makes weak strong
        is_weak_apple_fn_pressed = false;
    } else if (layer < FN_LAYER && is_layer_enabled(FN_LAYER)) {
        // A passthrough key on Apple Fn layer will be used for Fn-combos
        press_weak_apple_fn();
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

/// This function is called to execute macro keycodes. Macros are implemented
/// as actual code, so you can do pretty much anything with them.
static void execute_macro(uint8_t macro_number, bool is_release, uint8_t physical_key, uint8_t layer, uint8_t * restrict data) {
    const enum macro macro = macro_number;

    switch (macro) {
        case FN_LAYER_ON_HOLD:
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

        default:
            break;
    }
}

/// Called after enabling or disabling a layer.
/// This can be used to do things like add/remove modifiers based on the state
/// of a layer, or override LEDSs.
static void layer_state_changed(uint8_t layer, bool is_enabled) {
    // if (layer == FN_LAYER) {
    //     if (is_enabled) {
    //         add_override_leds_on(LED_SCROLL_LOCK);
    //     } else {
    //         remove_override_leds_on(LED_SCROLL_LOCK);
    //     }
    // }
}

/// Called after the keyboard has been reset. This can be used to override the
/// default initial state, e.g., set custom layers mask, load configuration
/// from EEPROM, etc.
static inline void handle_reset(void) {
    clear_override_leds();
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
/// fingerprint is updated. It can be used to configure the keyboard (e.g.,
/// active layers) based on the operating system it is plugged into.
void host_os_fingerprint_updated(uint8_t fingerprint) {
    switch (host_fingerprint_os_guess()) {
        case HOST_OS_LINUX:
            //disable_layer(WINDOWS_LAYER);
            //enable_layer(LINUX_LAYER);
            break;
        case HOST_OS_WINDOWS:
            //disable_layer(LINUX_LAYER);
            //enable_layer(WINDOWS_LAYER);
            break;
        case HOST_OS_MACOS:
            //disable_layer(LINUX_LAYER);
            //disable_layer(WINDOWS_LAYER);
            break;
        default:
            return;
    }
    host_fingerprint_stop_notifications();
}
#endif
