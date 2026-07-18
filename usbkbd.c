/*
 * usbkbd.c: USB HID keyboard implementation.
 *
 * Copyright (c) 2021-2026 Kimmo Kulovesi, https://arkku.dev/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define USB_KEYBOARD_ACCESS_STATE 1
#include "usbkbd.h"

#if ENABLE_PS2_DEVICE
#include "ps2_output.h"
static uint8_t ps2_modifier_flags = 0;
#endif

#include "aakbd.h"
#include "usb.h"
#include "usb_keys.h"
#include "usb_hardware.h"
#include "generic_hid.h"
#include "progmem.h"
#if ENABLE_HOST_FINGERPRINT
#include "host_fingerprint.h"
#endif
#if DEBOUNCE_DEBUG
#include "debounce/debounce_debug.h"
#endif

// MARK: - Keyboard Variables

/// Desired state of the keyboard LEDs as set over USB.
volatile uint8_t usb_keyboard_leds = 0;

#if ENABLE_VIRTUAL_LEDS
volatile uint8_t usb_virtual_leds = 0;
#endif

/// The buffer for keys currently pressed. Terminated by a zero, hence one
/// element more than required.
uint8_t usb_keys_buffer[MAX_KEY_ROLLOVER + 1] = { 0 };

/// Flags indicating which modifier keys are currently pressed. Note that if
/// multiple keys are mapped to the same modifier key, releasing either of
/// them causes the modifier to be released.
uint8_t usb_keys_modifier_flags = 0;

/// Flags of extended keys (e.g., media keys and Apple Fn).
usb_keys_extended_flags_t usb_keys_extended_flags = 0;

#if MEDIA_KEYS_ENDPOINT
/// Consumer endpoint: 16-bit usage (0 = none / release all).
uint16_t usb_consumer_usage = 0;

/// Consumer report needs to be sent.
bool usb_consumer_updated = false;
#endif

/// The selected keyboard protocol.
uint8_t usb_keyboard_protocol = HID_PROTOCOL_REPORT;

/// Are there changes to the pressed keys that we haven't sent?
volatile bool usb_keyboard_updated = false;

/// Error status of the keyboard (e.g., overflow). The least significant bit
/// is 1 if the error has not yet been sent to the host in a report.
volatile uint8_t key_error = 0;

#define keys_buffer usb_keys_buffer

// MARK: - Keyboard

#if defined(__arm__) && ENABLE_SIMULATED_TYPING
static int debug_kbd_write(void *cookie, const char *buf, int len);
#endif

void
usb_keyboard_reset (void) {
#if defined(__arm__) && ENABLE_SIMULATED_TYPING
    if (!usb_kbd_type) {
        usb_kbd_type = funopen(NULL, NULL, debug_kbd_write, NULL, NULL);
        setvbuf(usb_kbd_type, NULL, _IOLBF, 0);
    }
#endif
    usb_keyboard_leds = 0;
    usb_keyboard_protocol = HID_PROTOCOL_REPORT;
    usb_keyboard_release_all_keys();
    usb_keyboard_updated = true;
}

static inline int_fast8_t
next_free_buffer_index (const uint8_t key) {
    int_fast8_t i = 0;
    while (keys_buffer[i] && keys_buffer[i] != key) {
        // Find the first empty spot or duplicate entry to overwrite
        ++i;
    }
    return i;
}

void
usb_keyboard_press (const uint8_t key) {
    if (key <= KEY_MAX_ERROR_CODE) {
        if (!key_error) {
            key_error = key;
        }
        return;
    }

#if ENABLE_PS2_DEVICE
    if (ps2_output_is_scanning()) {
#if !ENABLE_PS2_NKRO
        if (key >= MODIFIERS_START || next_free_buffer_index(key) != MAX_KEY_ROLLOVER)
#endif
        {
            ps2_press_key(key);
            if (IS_MODIFIER(key)) {
                ps2_modifier_flags |= MODIFIER_BIT(key);
            }
        }
    }
#endif

    if (key < MODIFIERS_START) {
        const int_fast8_t i = next_free_buffer_index(key);
        if (i >= usb_keyboard_rollover && !key_error) {
            key_error = KEY_ERROR_OVERFLOW;
        }
        if (i == MAX_KEY_ROLLOVER) {
            // Don't overwrite the zero terminator
            return;
        }
        keys_buffer[i] = key;
        usb_keyboard_updated = true;
    } else if (IS_MODIFIER(key)) {
        usb_keyboard_add_modifiers(MODIFIER_BIT(key));
    }
#if ENABLE_VIRTUAL_KEYS
    else {
        press_virtual(key);
    }
#endif
}

void
usb_keyboard_release (const uint8_t key) {
#if ENABLE_PS2_DEVICE
    if (ps2_output_is_scanning()) {
        ps2_release_key(key);
        if (IS_MODIFIER(key)) {
            ps2_modifier_flags &= ~MODIFIER_BIT(key);
        }
    }
#endif

    if (key < MODIFIERS_START) {
        bool found = false;
        uint8_t *w = keys_buffer;
        const uint8_t *r = w;
        do {
            if (*r != key) {
                *w++ = *r;
            } else {
                found = true;
            }
            ++r;
        } while (*w);

        if (key_error) {
            if (!found) {
                // We are out of sync with the physical keyboard, e.g.,
                // overflow could have masked key releases and left a key
                // stuck.
                keyboard_reset();
            }
            if (usb_keyboard_is_idle) {
                // All keys are released, we can clear the error state
                key_error = 0;
            }
        }
        usb_keyboard_updated = true;
    } else if (IS_MODIFIER(key)) {
        usb_keyboard_remove_modifiers(MODIFIER_BIT(key));
    }
#if ENABLE_VIRTUAL_KEYS
    else {
        release_virtual(key);
    }
#endif
}

#if ENABLE_VIRTUAL_KEYS
#if MEDIA_KEYS_COUNT > 8
/// Low bytes of 22 consumer usage IDs in USB_KEY enum order (matches CONSUMER_KEY_ENUM).
static const uint8_t PROGMEM consumer_usage_low[MEDIA_KEYS_COUNT] = {
    CC_KEY_MEDIA_VOLUME_MUTE,     // USB_KEY_VOLUME_MUTE
    CC_KEY_MEDIA_VOLUME_UP,       // USB_KEY_VOLUME_UP
    CC_KEY_MEDIA_VOLUME_DOWN,     // USB_KEY_VOLUME_DOWN
    CC_KEY_MEDIA_NEXT_TRACK,      // USB_KEY_NEXT_TRACK
    CC_KEY_MEDIA_PREVIOUS_TRACK,  // USB_KEY_PREVIOUS_TRACK
    CC_KEY_MEDIA_STOP_PLAYING,    // USB_KEY_MEDIA_STOP
    CC_KEY_MEDIA_PLAY_PAUSE,      // USB_KEY_PLAY_PAUSE
    CC_KEY_MEDIA_STOP_EJECT,      // USB_KEY_MEDIA_EJECT
    CC_KEY_LAUNCH_MAIL,           // USB_KEY_LAUNCH_MAIL
    CC_KEY_LAUNCH_CALCULATOR,     // USB_KEY_LAUNCH_CALCULATOR
    CC_KEY_LAUNCH_MY_PC,          // USB_KEY_LAUNCH_MY_PC
    (uint8_t) CC_KEY_BROWSE_SEARCH,     // USB_KEY_BROWSE_SEARCH
    (uint8_t) CC_KEY_BROWSE_HOME,       // USB_KEY_BROWSE_HOME
    (uint8_t) CC_KEY_BROWSE_BACK,       // USB_KEY_BROWSE_BACK
    (uint8_t) CC_KEY_BROWSE_FORWARD,    // USB_KEY_BROWSE_FORWARD
    (uint8_t) CC_KEY_BROWSE_STOP,       // USB_KEY_BROWSE_STOP
    (uint8_t) CC_KEY_BROWSE_REFRESH,    // USB_KEY_BROWSE_REFRESH
    (uint8_t) CC_KEY_BROWSE_FAVORITES,  // USB_KEY_BROWSE_FAVORITES
    CC_KEY_MEDIA_FAST_FORWARD,    // USB_KEY_FAST_FORWARD
    CC_KEY_MEDIA_REWIND,          // USB_KEY_REWIND
    CC_KEY_BRIGHTNESS_UP,         // USB_KEY_BRIGHTNESS_UP
    CC_KEY_BRIGHTNESS_DOWN,       // USB_KEY_BRIGHTNESS_DOWN
};
#elif MEDIA_KEYS_ENDPOINT
/// Low bytes of media key usage IDs.
static const uint8_t PROGMEM consumer_usage_low[MEDIA_KEYS_COUNT] = {
    CC_MEDIA_KEY_1,
    CC_MEDIA_KEY_2,
    CC_MEDIA_KEY_3,
    CC_MEDIA_KEY_4,
    CC_MEDIA_KEY_5,
    CC_MEDIA_KEY_6,
    CC_MEDIA_KEY_7,
#if MEDIA_KEYS_COUNT >= 8
    CC_MEDIA_KEY_8,
#endif
};
#endif

void
press_virtual (const uint8_t key) {
#if MEDIA_KEYS_ENDPOINT
    if (key >= USB_KEY_VIRTUAL_MEDIA_1 && key < USB_KEY_VIRTUAL_MEDIA_1 + MEDIA_KEYS_COUNT) {
        uint8_t low = pgm_read_byte(&consumer_usage_low[key - USB_KEY_VIRTUAL_MEDIA_1]);
        uint16_t usage = low;
#if MEDIA_KEYS_COUNT > 8
        if (key >= USB_KEY_BROWSE_SEARCH && key <= USB_KEY_BROWSE_FAVORITES) {
            // The BROWSE_* keys have a 16-bit keycode
            usage = 0x0200U | low;
        }
#endif
        usb_consumer_usage = usage;
        usb_consumer_updated = true;
        return;
    }
#endif
    if (IS_VIRTUAL_KEY(key)) {
#if APPLE_FN_IS_MODIFIER
        if (key == USB_KEY_VIRTUAL_APPLE_FN) {
            usb_keyboard_add_modifiers(APPLE_FN_BIT);
        }
#endif
        usb_keyboard_updated = true;
        usb_keys_extended_flags |= VIRTUAL_KEY_BIT(key);
    }
}

void
release_virtual (const uint8_t key) {
#if MEDIA_KEYS_ENDPOINT
    if (key >= USB_KEY_VIRTUAL_MEDIA_1 && key < USB_KEY_VIRTUAL_MEDIA_1 + MEDIA_KEYS_COUNT) {
        usb_consumer_usage = 0;
        usb_consumer_updated = true;
        return;
    }
#endif
    if (IS_VIRTUAL_KEY(key)) {
#if APPLE_FN_IS_MODIFIER
        if (key == USB_KEY_VIRTUAL_APPLE_FN) {
            usb_keyboard_remove_modifiers(APPLE_FN_BIT);
        }
#endif
        usb_keyboard_updated = true;
        usb_keys_extended_flags &= ~VIRTUAL_KEY_BIT(key);
    }
}

bool
is_virtual_pressed (const uint8_t key) {
    if (IS_VIRTUAL_KEY(key)) {
#if APPLE_FN_IS_MODIFIER
        if (key == USB_KEY_VIRTUAL_APPLE_FN) {
            return (usb_keys_modifier_flags & APPLE_FN_BIT) != 0;
        }
#endif
        return (usb_keys_extended_flags & VIRTUAL_KEY_BIT(key)) != 0;
    }
    return false;
}
#endif // ENABLE_VIRTUAL_KEYS

void
usb_keyboard_release_all_keys (void) {
    for (int_fast8_t i = 0; i < MAX_KEY_ROLLOVER; ++i) {
#if ENABLE_PS2_DEVICE
        if (keys_buffer[i]) {
            ps2_release_key(keys_buffer[i]);
            ps2_output_task();
        }
#endif
        keys_buffer[i] = 0;
    }
    usb_keys_modifier_flags = 0;
    usb_keys_extended_flags = 0;
    key_error = 0;
    usb_keyboard_updated = true;

#if MEDIA_KEYS_ENDPOINT
    usb_consumer_updated = true;
    usb_consumer_usage = 0;
#endif

#if ENABLE_PS2_DEVICE
    {
        uint8_t mods = ps2_modifier_flags;
        uint8_t key = MODIFIERS_START;
        uint8_t bit = 1;
        while (mods) {
            if (mods & bit) {
                ps2_release_key(key);
                mods &= ~bit;
                ps2_output_task();
            }
            ++key;
            bit <<= 1;
        }
        ps2_modifier_flags = 0;

        ps2_output_clear_keys(false);
    }
#endif

}

void
usb_keyboard_keypress_delay (void) {
    for (int_fast8_t i = SIMULATED_KEYPRESS_TIME_MS; i; --i) {
        delay_milliseconds(1);
#if ENABLE_PS2_DEVICE
        ps2_output_task();
#endif
    }
    (void) usb_keyboard_send_if_needed();
}

bool
usb_keyboard_simulate_keypress (const uint8_t key, const uint8_t mods) {
    const uint8_t old_mods = usb_keys_modifier_flags;
    const uint8_t old_extended = usb_keys_extended_flags;

#if ENABLE_PS2_DEVICE
    if (ps2_output_is_scanning()) {
        usb_keyboard_set_modifiers(mods);
        usb_keyboard_press(key);
        usb_keyboard_keypress_delay();
        usb_keyboard_release(key);
        ps2_output_task();
        usb_keyboard_set_modifiers(old_mods);
        usb_keys_extended_flags = old_extended;
        return ps2_output_is_scanning();
    }
#endif

    usb_keys_modifier_flags = mods;
    usb_keyboard_press(key);
    (void) usb_keyboard_send_report();
    usb_keyboard_keypress_delay();
    usb_keyboard_release(key);

    usb_keys_modifier_flags = old_mods;
    usb_keys_extended_flags = old_extended;
    return usb_keyboard_send_report();
}

#if ENABLE_SIMULATED_TYPING
bool
usb_keyboard_type_char (const char c) {
    uint8_t key = 0;
    bool shift = false;
    if (c >= '1' && c <= '9') {
        key = USB_KEY_1 + (c - '1');
#if DVORAK_MAPPINGS
    } else {
        switch (c) {
        case 'A': shift = true; // fallthrough
        case 'a': key = USB_KEY_DVORAK_A; break;
        case 'B': shift = true; // fallthrough
        case 'b': key = USB_KEY_DVORAK_B; break;
        case 'C': shift = true; // fallthrough
        case 'c': key = USB_KEY_DVORAK_C; break;
        case 'D': shift = true; // fallthrough
        case 'd': key = USB_KEY_DVORAK_D; break;
        case 'E': shift = true; // fallthrough
        case 'e': key = USB_KEY_DVORAK_E; break;
        case 'F': shift = true; // fallthrough
        case 'f': key = USB_KEY_DVORAK_F; break;
        case 'G': shift = true; // fallthrough
        case 'g': key = USB_KEY_DVORAK_G; break;
        case 'H': shift = true; // fallthrough
        case 'h': key = USB_KEY_DVORAK_H; break;
        case 'I': shift = true; // fallthrough
        case 'i': key = USB_KEY_DVORAK_I; break;
        case 'J': shift = true; // fallthrough
        case 'j': key = USB_KEY_DVORAK_J; break;
        case 'K': shift = true; // fallthrough
        case 'k': key = USB_KEY_DVORAK_K; break;
        case 'L': shift = true; // fallthrough
        case 'l': key = USB_KEY_DVORAK_L; break;
        case 'M': shift = true; // fallthrough
        case 'm': key = USB_KEY_DVORAK_M; break;
        case 'N': shift = true; // fallthrough
        case 'n': key = USB_KEY_DVORAK_N; break;
        case 'O': shift = true; // fallthrough
        case 'o': key = USB_KEY_DVORAK_O; break;
        case 'P': shift = true; // fallthrough
        case 'p': key = USB_KEY_DVORAK_P; break;
        case 'Q': shift = true; // fallthrough
        case 'q': key = USB_KEY_DVORAK_Q; break;
        case 'R': shift = true; // fallthrough
        case 'r': key = USB_KEY_DVORAK_R; break;
        case 'S': shift = true; // fallthrough
        case 's': key = USB_KEY_DVORAK_S; break;
        case 'T': shift = true; // fallthrough
        case 't': key = USB_KEY_DVORAK_T; break;
        case 'U': shift = true; // fallthrough
        case 'u': key = USB_KEY_DVORAK_U; break;
        case 'V': shift = true; // fallthrough
        case 'v': key = USB_KEY_DVORAK_V; break;
        case 'W': shift = true; // fallthrough
        case 'w': key = USB_KEY_DVORAK_W; break;
        case 'X': shift = true; // fallthrough
        case 'x': key = USB_KEY_DVORAK_X; break;
        case 'Y': shift = true; // fallthrough
        case 'y': key = USB_KEY_DVORAK_Y; break;
        case 'Z': shift = true; // fallthrough
        case 'z': key = USB_KEY_DVORAK_Z; break;
        case ':': shift = true; // fallthrough
        case ';': key = USB_KEY_DVORAK_SEMICOLON; break;
        case '>': shift = true; // fallthrough
        case '.': key = USB_KEY_DVORAK_PERIOD; break;
        case '<': shift = true; // fallthrough
        case ',': key = USB_KEY_DVORAK_COMMA; break;
        case '"': shift = true; // fallthrough
        case '\'': key = USB_KEY_DVORAK_QUOTE; break;
        case '+': shift = true; // fallthrough
        case '=': key = USB_KEY_DVORAK_EQUALS; break;
        case '_': shift = true; // fallthrough
        case '-': key = USB_KEY_DVORAK_DASH; break;
        case '?': shift = true; // fallthrough
        case '/': key = USB_KEY_DVORAK_SLASH; break;
        case '{': shift = true; // fallthrough
        case '[': key = USB_KEY_DVORAK_OPEN_BRACKET; break;
        case '}': shift = true; // fallthrough
        case ']': key = USB_KEY_DVORAK_CLOSE_BRACKET; break;
#else
    } else if (c >= 'a' && c <= 'z') {
        key = USB_KEY_A + (c - 'a');
    } else if (c >= 'A' && c <= 'Z') {
        key = USB_KEY_A + (c - 'A');
        shift = true;
    } else {
        switch (c) {
        case ':': shift = true; // fallthrough
        case ';': key = USB_KEY_SEMICOLON; break;
        case '>': shift = true; // fallthrough
        case '.': key = USB_KEY_PERIOD; break;
        case '<': shift = true; // fallthrough
        case ',': key = USB_KEY_COMMA; break;
        case '"': shift = true; // fallthrough
        case '\'': key = USB_KEY_QUOTE; break;
        case '+': shift = true; // fallthrough
        case '=': key = USB_KEY_EQUALS; break;
        case '_': shift = true; // fallthrough
        case '-': key = USB_KEY_DASH; break;
        case '?': shift = true; // fallthrough
        case '/': key = USB_KEY_SLASH; break;
        case '{': shift = true; // fallthrough
        case '[': key = USB_KEY_OPEN_BRACKET; break;
        case '}': shift = true; // fallthrough
        case ']': key = USB_KEY_CLOSE_BRACKET; break;
#endif
        case '~': shift = true; // fallthrough
        case '`': key = USB_KEY_BACKTICK; break;
        case '|': shift = true; // fallthrough
        case ' ': key = USB_KEY_SPACE; break;
        case '\n': key = USB_KEY_RETURN; break;
        case '0': key = USB_KEY_0; break;
        case '\\': key = USB_KEY_ANSI_BACKSLASH; break;
        case '!': key = USB_KEY_1; shift = true; break;
        case '@': key = USB_KEY_2; shift = true; break;
        case '#': key = USB_KEY_3; shift = true; break;
        case '$': key = USB_KEY_4; shift = true; break;
        case '%': key = USB_KEY_5; shift = true; break;
        case '^': key = USB_KEY_6; shift = true; break;
        case '&': key = USB_KEY_7; shift = true; break;
        case '*': key = USB_KEY_8; shift = true; break;
        case '(': key = USB_KEY_9; shift = true; break;
        case ')': key = USB_KEY_0; shift = true; break;
        case '\t': key = USB_KEY_TAB; break;
        case '\b': key = USB_KEY_BACKSPACE; break;
        case '\033': key = USB_KEY_ESC; break;
        default: break;
        }
    }
    if (key == 0) {
        return false;
    }
    return usb_keyboard_simulate_keypress(key, shift ? SHIFT_BIT : 0);
}

void
usb_keyboard_type_bitmask (const uint8_t bitmask) {
    for (uint8_t bit = (1U << 7); bit; bit >>= 1) {
        usb_keyboard_type_char((bitmask & bit) ? '1' : '0');
    }
}

#if defined(__AVR__)
static int
debug_kbd_putchar (const char c, FILE *f) {
    if (usb_keyboard_type_char(c)) {
        reset_watchdog_timer();
        return 1;
    } else {
        return 0;
    }
}

static FILE debug_kbd_fdev = FDEV_SETUP_STREAM(debug_kbd_putchar, NULL, _FDEV_SETUP_WRITE);
FILE *usb_kbd_type = &debug_kbd_fdev;
#elif defined(__arm__)
static int
debug_kbd_write (void *cookie, const char *buf, int len) {
    for (int i = 0; i < len; ++i) {
        if (!usb_keyboard_type_char(buf[i])) {
            return i;
        }
    }
    return len;
    (void) cookie;
}

FILE *usb_kbd_type = NULL;
#endif

void
usb_keyboard_type_debug_report (void) {
    uint8_t old_mods = usb_keys_modifier_flags;
    int_fast8_t key_count = 0;
    while (key_count < MAX_KEY_ROLLOVER && keys_buffer[key_count]) {
        ++key_count;
    }

#if defined(__AVR__)
    extern int __heap_start, *__brkval;
    int free_bytes = ((int) &free_bytes) - (__brkval ? (int) __brkval : (int) &__heap_start);
#elif defined(__arm__)
    extern uint32_t _ebss;
    volatile uint32_t pos_on_stack = 0;
    int free_bytes = (char *)&pos_on_stack - (char *)&_ebss;
#endif

    usb_keyboard_release_all_keys();

    (void) fprintf_P(
        usb_kbd_type,
        PSTR("M %d A%u %u@%u %d$%d ^%u *%c%c%c %c %u\n"),
        free_bytes,
        usb_address(),
        usb_is_configured(),
        usb_keyboard_protocol,
        key_count,
        usb_keyboard_rollover,
        old_mods,
        (usb_keyboard_leds & 1) ? '1' : '0',
        (usb_keyboard_leds & 2) ? '1' : '0',
        (usb_keyboard_leds & 4) ? '1' : '0',
        usb_is_suspended() ? '!' : '@',
        (unsigned int) current_10ms_tick_count()
    );

#if DEBOUNCE_DEBUG
    debounce_debug_print_histogram();
#endif

#if ENABLE_HOST_FINGERPRINT
    if (host_fingerprint_count()) {
        (void) fprintf_P(usb_kbd_type, PSTR("OS #%u"), (unsigned int) host_fingerprint_bits());
        for (uint8_t i = 0; i < host_fingerprint_wlength_count(); ++i) {
            (void) fprintf_P(usb_kbd_type, PSTR(" %u"), (unsigned int) host_fingerprint_get_wlength_at(i));
        }
        (void) fflush(usb_kbd_type);
        (void) usb_keyboard_simulate_keypress(USB_KEY_RETURN, 0);
    }
#endif

    usb_keyboard_release_all_keys();
    usb_keys_modifier_flags = old_mods;
}
#endif

uint8_t
usb_keyboard_modifiers (void) {
    return usb_keys_modifier_flags;
}

void
usb_keyboard_set_modifiers (const uint8_t modifier_flags) {
#if ENABLE_PS2_DEVICE
    if (ps2_output_is_scanning()) {
        uint8_t unsent_mods = modifier_flags ^ ps2_modifier_flags;
        uint8_t key = MODIFIERS_START;
        uint8_t bit = 1;
        while (unsent_mods) {
            if (unsent_mods & bit) {
                unsent_mods ^= bit;

                if (modifier_flags & bit) {
                    ps2_press_key(key);
                } else {
                    ps2_release_key(key);
                }
                if (unsent_mods) {
                    // Try to avoid filling the output buffer for multiple mods
                    ps2_output_task();
                }
            }
            ++key;
            bit <<= 1;
        }
        ps2_modifier_flags = modifier_flags;
    }
#endif

    if (usb_keys_modifier_flags != modifier_flags) {
#if ENABLE_BOOTLOADER_SHORTCUT
        if (modifier_flags == (SHIFT_BIT | RIGHT_SHIFT_BIT) && !(usb_keys_modifier_flags & RIGHT_SHIFT_BIT)) {
            if (keys_buffer[0] == USB_KEY_ESC) {
                jump_to_bootloader();
            }
        }
#endif

        usb_keys_modifier_flags = modifier_flags;
        usb_keyboard_updated = true;
    }
}

void
usb_keyboard_add_modifiers (const uint8_t modifier_flags) {
    usb_keyboard_set_modifiers(usb_keys_modifier_flags | modifier_flags);
}

void
usb_keyboard_remove_modifiers (const uint8_t modifier_flags) {
    usb_keyboard_set_modifiers(usb_keys_modifier_flags & ~modifier_flags);
}

bool
usb_keyboard_send_if_needed (void) {
    bool did_send = false;
    if (usb_keyboard_updated) {
        did_send = usb_keyboard_send_report();
    }
#if MEDIA_KEYS_ENDPOINT
    if (usb_consumer_updated) {
        usb_consumer_updated = false;
        usb_keyboard_send_consumer(usb_consumer_usage);
    }
#endif
    return did_send;
}

uint8_t
usb_key_error (void) {
    return key_error;
}

uint8_t
usb_keyboard_led_state (void) {
#if ENABLE_VIRTUAL_LEDS
    return usb_keyboard_leds | usb_virtual_leds;
#else
    return usb_keyboard_leds;
#endif
}

void
usb_keyboard_toggle_boot_protocol (void) {
    usb_keyboard_protocol = usb_keyboard_is_in_boot_protocol ? HID_PROTOCOL_REPORT : HID_PROTOCOL_BOOT;
    usb_keyboard_updated = true;
}
