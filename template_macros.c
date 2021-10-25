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

    case MACRO_SHIFT_REPLACE_ALT_WITH_CMD_IF_NOT_ALREADY:
        // Shift, but if Cmd was not already pressed replace Alt with it.
        if (is_release) {
            remove_strong_modifiers(*data);
        } else {
            if (strong_modifiers_mask() & CMD_BIT) {
                // There's already a Cmd, just work as a Shift
                *data = SHIFT_BIT;
            } else {
                // No Cmd, remove any Alt and work as Shift + Cmd
                remove_strong_modifiers(ALT_BIT);
                *data = SHIFT_BIT | CMD_BIT;
            }
            add_strong_modifiers(*data);
        }
        break;

    case MACRO_CMD_OR_ALT_IF_ALREADY_CMD:
        // Works as Cmd if that modifier isn't already set, otherwise as Alt.
        if (is_release) {
            remove_strong_modifiers(*data);
        } else {
            // Use `data` to store the bit that we added.
            if (strong_modifiers_mask() & CMD_BIT) {
                *data = ALT_BIT;
            } else {
                *data = CMD_BIT;
            }
            add_strong_modifiers(*data);
        }
        break;

    case MACRO_PRINT_SCREEN_BOOTLOADER:
        // Works as a print screen key, but if both shifts are down when the
        // key is released, enters the bootloader for firmware update. This
        // basically duplicates the `ENABLE_RESET_SHORTCUT` option in a macro,
        // and moves it to print screen.
        if (is_release) {
            usb_keyboard_release(KEY(PRINT_SCREEN));
            if (strong_modifiers_mask() == (SHIFT_BIT | RIGHT_SHIFT_BIT)) {
                jump_to_bootloader();
            }
        } else {
            usb_keyboard_press(KEY(PRINT_SCREEN));
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
    if (layer == DVORAK_LAYER) {
        if (is_enabled) {
            add_override_leds_on(LED_SCROLL_LOCK);
        } else {
            remove_override_leds_on(LED_SCROLL_LOCK);
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
