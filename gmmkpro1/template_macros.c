/**
 * macros.c: Macro definitions for GMMK Pro.
 */

#include <macros.h>
#include <stdint.h>
#include "encoder.h"
#include "rgb_matrix.h"

#include "config.h"
#include "led_map.h"
#include "timer.h"
#include "matrix.h"

#ifndef KK_LAYERS_H
#include "layers.c"
#endif

#define NUM_LOCK_LIGHT_KEY USB_KEY_ESC
#define CAPS_LOCK_LIGHT_KEY USB_KEY_CAPS_LOCK
#define SCROLL_LOCK_LIGHT_KEY USB_KEY_PRINT_SCREEN

static inline void keyboard_host_leds_changed(uint8_t leds) {
    const bool is_error = (leds & LED_VIRTUAL_USB_ERROR_BIT) != 0;
    const bool is_active = (leds & LED_VIRTUAL_USB_ACTIVE_BIT) != 0;
    const bool is_boot = (leds & LED_VIRTUAL_BOOT_PROTOCOL_BIT) != 0;

    if (leds & LED_NUM_LOCK_BIT) {
        rgb_led_set_by_keycode(USB_KEY_ESC, is_error ? 16 : 8, is_boot ? 128 : 32, is_active ? 64 : 32);
    } else {
        rgb_led_set_by_keycode(USB_KEY_ESC, is_error ? 64 : 0, is_boot ? 128 :  0, is_active ? 32 : 0);
    }
    if (leds & LED_CAPS_LOCK_BIT) {
        rgb_led_set_by_keycode(USB_KEY_CAPS_LOCK, 16, 128, 64);
    } else {
        rgb_led_set_by_keycode(USB_KEY_CAPS_LOCK, 0, 0, 0);
    }
    if (leds & LED_SCROLL_LOCK_BIT) {
        rgb_led_set_by_keycode(USB_KEY_PRINT_SCREEN, 0, 32, 128);
    } else {
        rgb_led_set_by_keycode(USB_KEY_PRINT_SCREEN, 0, 0, 0);
    }
}

#if ENABLE_APPLE_FN_KEY
static bool is_weak_apple_fn_pressed = false;

static inline void press_weak_apple_fn(void) {
    if (!is_virtual_pressed(USB_KEY_VIRTUAL_APPLE_FN)) {
        press_virtual(USB_KEY_VIRTUAL_APPLE_FN);
        is_weak_apple_fn_pressed = true;
    }
}

static inline bool release_weak_apple_fn(void) {
    if (is_weak_apple_fn_pressed) {
        release_virtual(USB_KEY_VIRTUAL_APPLE_FN);
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

static inline keycode_t preprocess_press(keycode_t keycode, uint8_t physical_key, uint8_t * restrict data) {
    rgb_led_set_by_keycode(physical_key, 128, 192, 224);

#if ENABLE_APPLE_FN_KEY
    if (keycode == PASS && is_layer_active(APPLE_FN_LAYER) && !is_layer_active(WINDOWS_FN_LAYER)) {
        press_weak_apple_fn();
    } else if (keycode == KEY_APPLE_FN) {
        is_weak_apple_fn_pressed = false;
    } else if (release_weak_apple_fn()) {
        usb_keyboard_send_if_needed();
    }
#endif
    return keycode;
}

static inline void postprocess_release(keycode_t keycode, uint8_t physical_key, uint8_t data) {
    if (physical_key != NUM_LOCK_LIGHT_KEY && physical_key != CAPS_LOCK_LIGHT_KEY && physical_key != SCROLL_LOCK_LIGHT_KEY) {
        rgb_led_set_by_keycode(physical_key, 0, 0, 0);
    } else {
        keyboard_host_leds_changed(usb_keyboard_led_state());
    }
}

static void execute_macro(uint8_t macro_number, bool is_release, uint8_t physical_key, uint8_t * restrict data) {
    const enum macro macro = macro_number;

    switch (macro) {
    case MACRO_NOP:
        break;

    case MACRO_FALLTHROUGH:
        register_key(physical_key, is_release);
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
            } else {
                *data = 0;
            }
        }
        break;

    case MACRO_IPAD_AE_OE:
        if (!is_release) {
            const uint8_t mods = strong_modifiers_mask();
            if (!(mods & ~(SHIFT_BIT | RIGHT_SHIFT_BIT))) {
                usb_keyboard_simulate_keypress(KEY(U), ALT_BIT);
                usb_keyboard_simulate_keypress(
                    (mods & (SHIFT_BIT | RIGHT_SHIFT_BIT)) ? KEY(O) : KEY(A), 0);
            } else {
                *data = KEY(INT_NEXT_TO_LEFT_SHIFT);
            }
        }
        if (*data) {
            register_key(*data, is_release);
        }
        break;

    case MACRO_IPAD_A_O:
        if (!is_release) {
            const uint8_t mods = strong_modifiers_mask();
            if ((mods & (ALT_BIT | ALTGR_BIT)) && !(mods & (CMD_BIT | RIGHT_CMD_BIT | CTRL_BIT | RIGHT_CTRL_BIT))) {
                usb_keyboard_simulate_keypress(KEY(U), ALT_BIT);
                clear_strong_modifiers();
                add_weak_modifiers(SHIFT_BIT);
            }
            *data = physical_key;
        }
        register_key(*data, is_release);
        break;

    case MACRO_DEBUG_MATRIX:
        if (!is_release) {
            uint16_t scan_time = timer_read();
            for (int_fast8_t i = 100; i; --i) {
                (void) matrix_scan();
            }
            scan_time = timer_elapsed(scan_time);
            fprintf_P(usb_kbd_type, PSTR("Scan time %u.%02u ms\n"), scan_time / 100, scan_time % 100);
        }
        break;

    default:
        break;
    }
}

enum gradient {
    GRADIENT_RED_BLUE,
    GRADIENT_GREEN_BLUE,
    GRADIENT_BLUE_WHITE,
};

static void led_set_gradient(bool is_reversed, enum gradient scheme) {
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

static inline void handle_reset(void) {
#if ENABLE_APPLE_FN_KEY
    is_weak_apple_fn_pressed = false;
#endif
    led_set_gradient(false, ENABLE_HOST_FINGERPRINT ? GRADIENT_BLUE_WHITE : GRADIENT_RED_BLUE);

    // Safeguard (probably unnecessary) against momentary layers being left
    // on (e.g., when preserving active layers after wake from suspend)
    disable_layer(APPLE_FN_LAYER);
    disable_layer(WINDOWS_FN_LAYER);
    disable_layer(FN_SPACE_LAYER);
}

static inline void layer_state_changed(uint8_t layer, bool is_enabled) {
    if (is_enabled) {
        switch (layer) {
        case LINUX_LAYER:
            led_set_gradient(false, GRADIENT_GREEN_BLUE);
            break;
        case WINDOWS_LAYER:
            led_set_gradient(true, GRADIENT_RED_BLUE);
            break;
        case APPLE_FN_LAYER:
            break;
        case WINDOWS_FN_LAYER:
            enable_layer(APPLE_FN_LAYER);
            break;
        default:
            break;
        }
    } else {
        switch (layer) {
        case LINUX_LAYER:
            led_set_gradient(is_layer_active(WINDOWS_LAYER), GRADIENT_RED_BLUE);
            break;
        case WINDOWS_LAYER:
            led_set_gradient(false, is_layer_active(LINUX_LAYER) ? GRADIENT_GREEN_BLUE : GRADIENT_RED_BLUE);
            break;
        case APPLE_FN_LAYER:
            (void) release_weak_apple_fn();
            break;
        case WINDOWS_FN_LAYER:
            disable_layer(APPLE_FN_LAYER);
            break;
        default:
            break;
        }
    }
}

static inline void handle_tick(uint8_t tick_10ms_count) {
}

void os_fingerprint_updated(uint8_t evidence) {
    (void) evidence;
}

void handle_encoder_rotation(bool is_clockwise) {
    if (is_clockwise) {
        if (ENABLE_MEDIA_KEYS && !is_layer_active(WINDOWS_LAYER)) {
            usb_keyboard_simulate_keypress(KEY(VOLUME_UP), 0);
        } else {
            usb_keyboard_simulate_keypress(KEY(F21), 0);
        }
    } else {
        if (ENABLE_MEDIA_KEYS && !is_layer_active(WINDOWS_LAYER)) {
            usb_keyboard_simulate_keypress(KEY(VOLUME_DOWN), 0);
        } else {
            usb_keyboard_simulate_keypress(KEY(F20), 0);
        }
    }
}

#if ENABLE_HOST_FINGERPRINT
#include "host_fingerprint.h"

void host_os_fingerprint_updated(uint8_t fingerprint) {
    switch (host_fingerprint_os_guess()) {
        case HOST_OS_LINUX:
            disable_layer(WINDOWS_LAYER);
            enable_layer(LINUX_LAYER);
            host_fingerprint_stop_notifications();
            break;
        case HOST_OS_WINDOWS:
            disable_layer(LINUX_LAYER);
            enable_layer(WINDOWS_LAYER);
            host_fingerprint_stop_notifications();
            break;
        case HOST_OS_MACOS:
            disable_layer(LINUX_LAYER);
            disable_layer(WINDOWS_LAYER);
            led_set_gradient(false, GRADIENT_RED_BLUE);
            host_fingerprint_stop_notifications();
            break;
        default:
            break;
    }
}
#endif
