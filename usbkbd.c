/*
 * usbkbd.c: USB HID keyboard implementation for ATMEGA32U4.
 *
 * Copyright (c) 2021-2022 Kimmo Kulovesi, https://arkku.dev/
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
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>

#define INCLUDE_USB_HARDWARE_ACCESS
#define INCLUDE_USB_KEYBOARD_ACCESS
#include "usbkbd.h"
#include "usbkbd_descriptors.h"

#include "main.h"
#include "avrusb.h"
#include "usb.h"
#include "usb_keys.h"
#include "generic_hid.h"

// MARK: - Configuration

/// Frame divider for the idle counter. Must be a power of 2.
#define IDLE_COUNT_FRAME_DIVIDER    4

// Not an actual USB status, but we'll recycle the variable.
#define USB_STATUS_JUMP_TO_BOOTLOADER   (1 << 7)

#define ENDPOINT_0_FLAGS            EP_SINGLE_BUFFER

#define KEYBOARD_ENDPOINT_FLAGS     EP_DOUBLE_BUFFER
#define KEYBOARD_ENDPOINT_TYPE      EP_TYPE_INTERRUPT_IN

#define GENERIC_ENDPOINT_FLAGS      EP_SINGLE_BUFFER
#define GENERIC_ENDPOINT_TYPE       EP_TYPE_INTERRUPT_IN

// MARK: - USB Variables

/// How long has the keyboard has been idle, frames / `IDLE_COUNT_FRAME_DIVIDER`.
static volatile uint8_t keyboard_idle_count = 0;

/// The value of `keyboard_idle_count` to send an update on.
static volatile uint8_t keyboard_update_on_idle_count = KEYBOARD_UPDATE_IDLE_MS / IDLE_COUNT_FRAME_DIVIDER;

/// The active USB configuration. This is set by a request from the host.
static volatile uint8_t usb_configuration = 0;

/// USB status flags.
static volatile uint8_t usb_status = 0;

/// Is USB susended?
static volatile bool usb_suspended = 0;

/// Zero or a an ASCII character to identify an error.
static volatile uint8_t usb_error = 0;

#if ENABLE_GENERIC_HID_ENDPOINT
static volatile uint8_t generic_update_on_idle_count = GENERIC_HID_UPDATE_IDLE_MS / IDLE_COUNT_FRAME_DIVIDER;
static volatile uint8_t generic_idle_count = 0;
static volatile uint8_t generic_report_pending = 0;

#if GENERIC_HID_HANDLE_SYNCHRONOUSLY
static volatile uint8_t generic_request_pending = 0;
static volatile uint8_t generic_request_pending_id = 0;
#endif

#if GENERIC_HID_REPORT_SIZE != 0
static uint8_t generic_report[GENERIC_HID_REPORT_SIZE] = { 0 };
#else
static uint8_t generic_report[1] = { 0 };
#endif

#if GENERIC_HID_FEATURE_SIZE != 0
static uint8_t generic_request[GENERIC_HID_FEATURE_SIZE];
#else
static uint8_t generic_request[1] = { 0 };
#endif
#endif // ^ ENABLE_GENERIC_HID_ENDPOINT

#if ENABLE_DFU_INTERFACE
static volatile uint8_t usb_request_detach = 0;

#define dfu_app_state   (usb_request_detach ? DFU_APP_STATE_DETACH : DFU_APP_STATE_IDLE)
#endif

#if ENABLE_APPLE_FN_KEY
#define APPLE_VIRTUAL_START     USB_KEY_VIRTUAL_APPLE_FN
#define APPLE_VIRTUAL_END       USB_KEY_VIRTUAL_APPLE_EXPOSE_DESKTOP
#define APPLE_VIRTUAL_MASK      (0xFFU)

#define IS_APPLE_VIRTUAL(key)   ((key) >= APPLE_VIRTUAL_START && (key) <= APPLE_VIRTUAL_END)

#define APPLE_VIRTUAL_BIT(key)  (1 << ((key) - APPLE_VIRTUAL_START))
#endif

// MARK: - Keyboard Variables

/// Desired state of the keyboard LEDs as set over USB.
static volatile uint8_t keyboard_leds = 0;

/// The buffer for keys currently pressed. Terminated by a zero, hence one
/// element more than required.
static uint8_t keys_buffer[MAX_KEY_ROLLOVER + 1] = { 0 };

/// Flags indicating which modifier keys are currently pressed. Note that if
/// multiple keys are mapped to the same modifier key, releasing either of
/// them causes the modifier to be released.
static uint8_t keys_modifier_flags = 0;

/// Are there currently no keys pressed?
#define are_no_keys_pressed (keys_modifier_flags == 0 && keys_buffer[0] == 0)

/// The selected keyboard protocol.
static uint8_t keyboard_protocol = HID_PROTOCOL_REPORT;

/// Are there changes to the pressed keys that we haven't sent?
static volatile bool have_unsent_changes = false;

#define KEY_ERROR_NEEDS_REPORTING_FLAG  (1)

/// Error status of the keyboard (e.g., overflow). The least significant bit
/// is 1 if the error has not yet been sent to the host in a report.
static volatile uint8_t key_error = 0;

static uint8_t extended_keys_mask = 0;

/// Is the boot protocol active?
#define is_boot_protocol    (keyboard_protocol == HID_PROTOCOL_BOOT)

/// Effective rollover.
#define rollover            (is_boot_protocol ? USB_BOOT_PROTOCOL_ROLLOVER : USB_MAX_KEY_ROLLOVER)

// MARK: - USB

static INLINE bool usb_keyboard_wait_to_send(uint8_t * const sregptr, const int_fast8_t endpoint);

static void
usb_keyboard_reset (void) {
    keyboard_idle_count = 0;
    keyboard_leds = 0;
    keyboard_protocol = HID_PROTOCOL_REPORT;
    keyboard_update_on_idle_count = DIV_ROUND_BYTE(IDLE_COUNT_FRAME_DIVIDER, KEYBOARD_UPDATE_IDLE_MS);
#if ENABLE_GENERIC_HID_ENDPOINT
    generic_update_on_idle_count = DIV_ROUND_BYTE(IDLE_COUNT_FRAME_DIVIDER, GENERIC_HID_POLL_INTERVAL_MS);
    generic_report_pending = 0;
#if GENERIC_HID_HANDLE_SYNCHRONOUSLY
    generic_request_pending = 0;
    generic_request_pending_id = 0;
#endif
#endif
    usb_release_all_keys();
    have_unsent_changes = true;
}

static void
usb_reset (void) {
    usb_freeze();
    pll_enable();
    while (!is_pll_locked)
        ;
    usb_start_clock();

    usb_attach();
}

void
usb_init (void) {
    usb_descriptors_init();
    usb_hardware_init();
    usb_reset();
    usb_error = 0;
    usb_configuration = 0;
    usb_status = 0;
    usb_suspended = false;
    usb_keyboard_reset();
    usb_clear_interrupts(INT_SUSPEND_FLAG | INT_WAKE_UP_FLAG);
#if IS_SUSPEND_SUPPORTED
    usb_set_enabled_interrupts(INT_START_OF_FRAME_FLAG | INT_END_OF_RESET_FLAG | INT_SUSPEND_FLAG);
#else
    usb_set_enabled_interrupts(INT_START_OF_FRAME_FLAG | INT_END_OF_RESET_FLAG);
#endif
#if ENABLE_DFU_INTERFACE
    usb_request_detach = 0;
#endif
}

static INLINE void
usb_init_endpoints (void) {
    _Static_assert(IS_ENDPOINT_SIZE_VALID(KEYBOARD_ENDPOINT_SIZE), "Invalid keyboard endpoint size");

    for (int_fast8_t i = 1; i < USB_MAX_ENDPOINT; ++i) {
        usb_set_endpoint(i);
        switch (i) {
#if ENABLE_KEYBOARD_ENDPOINT
        case KEYBOARD_ENDPOINT_NUM:
            usb_setup_endpoint(
                KEYBOARD_ENDPOINT_NUM,
                KEYBOARD_ENDPOINT_TYPE,
                KEYBOARD_ENDPOINT_SIZE,
                KEYBOARD_ENDPOINT_FLAGS
            );
            break;
#endif
#if ENABLE_GENERIC_HID_ENDPOINT
        case GENERIC_HID_ENDPOINT_NUM:
            usb_setup_endpoint(
                GENERIC_HID_ENDPOINT_NUM,
                GENERIC_ENDPOINT_TYPE,
                GENERIC_ENDPOINT_SIZE,
                GENERIC_ENDPOINT_FLAGS
            );
            break;
#endif
        default:
            usb_set_endpoint(i);
            usb_disable_endpoint();
            break;
        }
    }
    usb_reset_endpoints_1to(USB_MAX_ENDPOINT);
}

bool
usb_is_ok (void) {
    return (usb_configuration != 0) && (usb_error == 0);
}

bool
usb_is_configured (void) {
    return (usb_configuration != 0);
}

bool
usb_is_in_boot_protocol (void) {
    return (is_boot_protocol != 0);
}

uint8_t
usb_address (void) {
    return usb_get_address();
}

uint8_t
usb_last_error (void) {
    return usb_error;
}

bool
usb_is_suspended (void) {
    return usb_suspended;
}

uint8_t
usb_key_error (void) {
    return key_error;
}

uint8_t
usb_detach_requested (void) {
#if ENABLE_DFU_INTERFACE
    return usb_request_detach;
#else
    return 0;
#endif
}

static INLINE void
usb_wake_up_if_suspended (void) {
#if IS_SUSPEND_SUPPORTED
    if (usb_suspended) {
        usb_set_remote_wakeup();
    }
#endif
}

bool
usb_wake_up_host (void) {
    usb_clear_remote_wakeup();

    if (is_usb_remote_wakeup_set || usb_suspended || !(usb_status & USB_STATUS_REMOTE_WAKEUP_ENABLED)) {
        usb_error = 'w';
        return false;
    }

    usb_init();
    usb_set_remote_wakeup();

    return true;
}

// MARK: - Generic HID

#if ENABLE_GENERIC_HID_ENDPOINT
bool
send_generic_hid_report (uint8_t report_id, uint8_t count, const uint8_t report[static count]) {
    if (!usb_configuration) {
        return false;
    }

    uint8_t old_sreg;
    if (!usb_keyboard_wait_to_send(&old_sreg, GENERIC_HID_ENDPOINT_NUM)) {
        return false;
    }

    generic_report_pending = false;

    usb_set_endpoint(GENERIC_HID_ENDPOINT_NUM);
    usb_wake_up_if_suspended();

    if (report_id) {
        usb_tx(report_id);
    }
    for (int_fast8_t i = 0; i < count; ++i) {
        const uint8_t byte = report[i];
        usb_tx(byte);
        if (i < GENERIC_HID_REPORT_SIZE) {
            generic_report[i] = byte;
        }
    }

    usb_release_rx();
    SREG = old_sreg;

    generic_idle_count = 0;

    return true;
}

bool
make_and_send_generic_hid_report (void) {
    if (make_generic_hid_report(0, GENERIC_HID_REPORT_SIZE, generic_report)) {
        return send_generic_hid_report(0, GENERIC_HID_REPORT_SIZE, generic_report);
    } else {
        return false;
    }
}

static INLINE bool
generic_request_call_handler (const uint8_t report_id, const uint8_t length) {
    uint8_t response_length = GENERIC_HID_REPORT_SIZE;
    const uint8_t response = handle_generic_hid_report(report_id, length, generic_request, &response_length, generic_report);

    switch (response) {
    case RESPONSE_OK:
        break;
    case RESPONSE_SEND_REPLY:
        generic_report_pending = response_length;
        break;
    case RESPONSE_JUMP_TO_BOOTLOADER:
        usb_configuration = 0;
        usb_status |= USB_STATUS_JUMP_TO_BOOTLOADER;
        break;
    case RESPONSE_ERROR:
        return false;
    default:
        break;
    }
    return true;
}
#endif // ^ ENABLE_GENERIC_HID_ENDPOINT

void
usb_tick (void) {
    if (usb_configuration == 0) {
        if (usb_status & USB_STATUS_JUMP_TO_BOOTLOADER) {
            jump_to_bootloader();
        }
        if (keyboard_idle_count || !have_unsent_changes) {
            usb_keyboard_reset();
        }
    } else {
#if ENABLE_GENERIC_HID_ENDPOINT
#if GENERIC_HID_HANDLE_SYNCHRONOUSLY
        if (generic_request_pending_id) {
            const uint8_t request_id = generic_request_pending_id;
            generic_request_pending_id = 0;
            generic_request_call_handler(request_id, generic_request_pending);
            generic_request_pending = 0;
        }
#endif
        if (generic_report_pending) {
            send_generic_hid_report(0, generic_report_pending, generic_report);
        }
#endif
    }
}

// MARK: - Keyboard

void
usb_keyboard_press (const uint8_t key) {
    if (key <= KEY_MAX_ERROR_CODE) {
        if (!key_error) {
            key_error = key;
        }
        return;
    }
    if (key < MODIFIERS_START) {
        int_fast8_t i = 0;
        while (keys_buffer[i] && keys_buffer[i] != key) {
            // Find the first empty spot or duplicate entry to overwrite
            ++i;
        }
        if (i >= rollover && !key_error) {
            key_error = KEY_ERROR_OVERFLOW;
        }
        if (i == MAX_KEY_ROLLOVER) {
            // Don't overwrite the zero terminator
            return;
        }
        keys_buffer[i] = key;
        have_unsent_changes = true;
    } else if (IS_MODIFIER(key)) {
        usb_keyboard_add_modifiers(MODIFIER_BIT(key));
    }
#if ENABLE_APPLE_FN_KEY
    else {
        press_apple_virtual(key);
    }
#endif
}

void
usb_keyboard_release (const uint8_t key) {
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
            if (are_no_keys_pressed) {
                // All keys are released, we can clear the error state
                key_error = 0;
            }
        }
        have_unsent_changes = true;
    } else if (IS_MODIFIER(key)) {
        usb_keyboard_remove_modifiers(MODIFIER_BIT(key));
    }
#if ENABLE_APPLE_FN_KEY
    else {
        release_apple_virtual(key);
    }
#endif
}

#if ENABLE_APPLE_FN_KEY
void
press_apple_virtual (const uint8_t key) {
    if (IS_APPLE_VIRTUAL(key)) {
        extended_keys_mask |= APPLE_VIRTUAL_BIT(key);
        have_unsent_changes = true;
    }
}

void
release_apple_virtual (const uint8_t key) {
    if (IS_APPLE_VIRTUAL(key)) {
        extended_keys_mask &= ~APPLE_VIRTUAL_BIT(key);
        have_unsent_changes = true;
    }
}

bool
is_apple_virtual_pressed (const uint8_t key) {
    if (IS_APPLE_VIRTUAL(key)) {
        return (extended_keys_mask & APPLE_VIRTUAL_BIT(key)) != 0;
    } else {
        return false;
    }
}
#endif

void
usb_release_all_keys (void) {
    for (int_fast8_t i = 0; i < MAX_KEY_ROLLOVER; ++i) {
        keys_buffer[i] = 0;
    }
    keys_modifier_flags = 0;
    extended_keys_mask = 0;
    key_error = 0;
    have_unsent_changes = true;
}

bool
usb_keyboard_simulate_keypress (const uint8_t key, const uint8_t mods) {
    const uint8_t old_mods = keys_modifier_flags;
    const uint8_t old_extended = extended_keys_mask;

    keys_modifier_flags = mods;
    usb_keyboard_press(key);
    (void) usb_keyboard_send_report();
    _delay_ms(SIMULATED_KEYPRESS_TIME_MS);
    usb_keyboard_release(key);

    keys_modifier_flags = old_mods;
    extended_keys_mask = old_extended;
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

static int
debug_kbd_putchar (const char c, FILE *f) {
    if (usb_keyboard_type_char(c)) {
        wdt_reset();
        return 1;
    } else {
        return 0;
    }
}

/// The serial port UART device.
static FILE debug_kbd_fdev = FDEV_SETUP_STREAM((debug_kbd_putchar), NULL, _FDEV_SETUP_WRITE);

FILE *usb_kbd_type = &debug_kbd_fdev;

void
usb_keyboard_type_debug_report (void) {
    uint8_t old_mods = keys_modifier_flags;
    int_fast8_t key_count = 0;
    while (key_count < MAX_KEY_ROLLOVER && keys_buffer[key_count]) {
        ++key_count;
    }

    extern int __heap_start, *__brkval;
    int free_bytes = ((int) &free_bytes) - (__brkval ? (int) __brkval : (int) &__heap_start);

    usb_release_all_keys();

    (void) fprintf_P(
        usb_kbd_type,
        PSTR("M %d A%d %d@%d $%d ^%d *%c%c%c %c\n"),
        free_bytes,
        usb_get_address(),
        usb_configuration,
        keyboard_protocol,
        key_count,
        old_mods,
        (keyboard_leds & 1) ? '1' : '0',
        (keyboard_leds & 2) ? '1' : '0',
        (keyboard_leds & 4) ? '1' : '0',
        usb_suspended ? '!' : '@'
    );

    usb_release_all_keys();
    keys_modifier_flags = old_mods;
}
#endif

uint8_t
usb_keyboard_modifiers (void) {
    return keys_modifier_flags;
}

void
usb_keyboard_set_modifiers (const uint8_t modifier_flags) {
    if (keys_modifier_flags != modifier_flags) {
        if (modifier_flags == (SHIFT_BIT | RIGHT_SHIFT_BIT) && !(keys_modifier_flags & RIGHT_SHIFT_BIT)) {
#if ENABLE_BOOTLOADER_SHORTCUT
            if (keys_buffer[0] == USB_KEY_SCROLL_LOCK) {
                jump_to_bootloader();
            }
#endif
#if ENABLE_RESET_SHORTCUT
            if (keys_buffer[0] == USB_KEY_ESC || keys_buffer[1] == USB_KEY_ESC) {
                keyboard_reset();
            }
#endif
#if ENABLE_DEBUG_SHORTCUT
            if (keys_buffer[0] == USB_KEY_F1) {
                usb_keyboard_type_debug_report();
            }
#endif
        }

        keys_modifier_flags = modifier_flags;
        have_unsent_changes = true;
    }
}

void
usb_keyboard_add_modifiers (const uint8_t modifier_flags) {
    usb_keyboard_set_modifiers(keys_modifier_flags | modifier_flags);
}

void
usb_keyboard_remove_modifiers (const uint8_t modifier_flags) {
    usb_keyboard_set_modifiers(keys_modifier_flags & ~modifier_flags);
}

static INLINE void
usb_tx_error_report (const uint8_t byte) {
    int_fast8_t i = rollover;
    keyboard_idle_count = 0;
#if USE_MULTIPLE_REPORTS
    if (!is_boot_protocol) {
        usb_tx(KEYBOARD_REPORT_ID);
    }
#endif
    usb_tx(keys_modifier_flags);
    if (RESERVE_BOOT_PROTOCOL_RESERVED_BYTE || is_boot_protocol) {
        usb_tx(0);
    }
    while (i--) {
        usb_tx(byte);
    }
    if (!is_boot_protocol || USB_MAX_KEY_ROLLOVER < USB_BOOT_PROTOCOL_ROLLOVER) {
#if ENABLE_APPLE_FN_KEY
        usb_tx(extended_keys_mask & APPLE_VIRTUAL_MASK);
#endif
    }
}

static INLINE void
usb_tx_keys_state (void) {
    int_fast8_t count = rollover;
    keyboard_idle_count = 0;
    have_unsent_changes = false;

#if USE_MULTIPLE_REPORTS
    if (!is_boot_protocol) {
        usb_tx(KEYBOARD_REPORT_ID);
    }
#endif

    usb_tx(keys_modifier_flags);
    if (RESERVE_BOOT_PROTOCOL_RESERVED_BYTE || is_boot_protocol) {
        usb_tx(0);
    } else {
        // Send the last key here so the end result is same as boot protocol
        // until we exceed 6 non-modifier keys down (which is probably never
        // unless specifically testing rollover). According to HID 1.11 spec
        // the order of keys in the array doesn't matter, so conforming hosts
        // should not have a problem with a leading zero byte.
        usb_tx(keys_buffer[--count]);
    }
    // ...although the order of keys in the array doesn't matter, somehow it
    // feels nicer to send them in chronological order.
    for (int_fast8_t i = 0; i < count; ++i) {
        usb_tx(keys_buffer[i]);
    }
    if (!is_boot_protocol || USB_MAX_KEY_ROLLOVER < USB_BOOT_PROTOCOL_ROLLOVER) {
#if ENABLE_APPLE_FN_KEY
        usb_tx(extended_keys_mask & APPLE_VIRTUAL_MASK);
#endif
    }
}

static INLINE bool
usb_keyboard_wait_to_send (uint8_t * const sregptr, const int_fast8_t endpoint) {
    const uint8_t timeout = usb_frame_count + 50U;

    uint8_t old_sreg = SREG;
    cli();

    for (;;) {
        if (is_usb_rw_allowed) {
            break;
        }

        SREG = old_sreg;

        if (!usb_configuration || (usb_frame_count == timeout)) {
            *sregptr = old_sreg;
            return false;
        }

        old_sreg = SREG;
        cli();
        usb_set_endpoint(endpoint);
    }

    *sregptr = old_sreg;
    return true;
}

bool
usb_keyboard_send_report (void) {
#if ENABLE_KEYBOARD_ENDPOINT
    if (!usb_configuration) {
        usb_error = 'c';
        return false;
    }

    uint8_t old_sreg;

    if (!usb_keyboard_wait_to_send(&old_sreg, KEYBOARD_ENDPOINT_NUM)) {
        usb_error = 'T';
        return false;
    }

    usb_set_endpoint(KEYBOARD_ENDPOINT_NUM);
    usb_wake_up_if_suspended();

    usb_tx_keys_state();
    usb_release_tx();

    keyboard_idle_count = 0;
    usb_error = 0;
    SREG = old_sreg;
#endif
    return true;
}

bool
usb_keyboard_send_if_needed (void) {
    bool did_send = false;
    if (have_unsent_changes) {
        did_send = usb_keyboard_send_report();
    }
    return did_send;
}

uint8_t
usb_keyboard_led_state (void) {
    return keyboard_leds;
}

void
usb_keyboard_toggle_boot_protocol (void) {
    keyboard_protocol = is_boot_protocol ? HID_PROTOCOL_REPORT : HID_PROTOCOL_BOOT;
    have_unsent_changes = true;
}

// MARK: - Interrupt handlers (this is most of the USB stuff happens)

ISR(USB_GEN_vect) {
    static volatile uint8_t frame_count = 0;
    const uint8_t intflags = usb_interrupt_flags_reg;
    usb_clear_interrupts(INT_END_OF_RESET_FLAG | INT_START_OF_FRAME_FLAG);

    if (intflags & INT_END_OF_RESET_FLAG) {
        _Static_assert(IS_ENDPOINT_SIZE_VALID(ENDPOINT_0_SIZE), "Invalid endpoint 0 size");

        usb_setup_endpoint(
            0,
            EP_TYPE_CONTROL,
            ENDPOINT_0_SIZE,
            ENDPOINT_0_FLAGS
        );
        usb_configuration = 0;
#if ENABLE_DFU_INTERFACE
        if (usb_request_detach) {
            usb_status |= USB_STATUS_JUMP_TO_BOOTLOADER;
        }
#endif
        usb_enable_endpoint_interrupts();
    }

    if ((intflags & INT_START_OF_FRAME_FLAG)) {
#if ENABLE_DFU_INTERFACE
        if (usb_request_detach) {
            --usb_request_detach;
        }
#endif
        if ((++frame_count % IDLE_COUNT_FRAME_DIVIDER) == 0 && !usb_suspended) {
#if ENABLE_KEYBOARD_ENDPOINT
            if (keyboard_update_on_idle_count) {
                usb_set_endpoint(KEYBOARD_ENDPOINT_NUM);
                if (is_usb_rw_allowed) {
                    if (++keyboard_idle_count == keyboard_update_on_idle_count) {
                        // Been idle for a while, send the state
                        const uint8_t error = key_error;
                        if (error & KEY_ERROR_NEEDS_REPORTING_FLAG) {
                            key_error = error + 1; // clears the flag, which is 1
                            usb_tx_error_report(error);
                        } else {
                            usb_tx_keys_state();
                        }
                    }
                    usb_release_tx();
                }
            }
#endif
#if ENABLE_GENERIC_HID_ENDPOINT
            if (generic_update_on_idle_count) {
                usb_set_endpoint(GENERIC_HID_ENDPOINT_NUM);
                if (is_usb_rw_allowed) {
                    if (++generic_idle_count == generic_update_on_idle_count) {
                        if (generic_report_pending) {
                            // Send the pending report now
                            const uint8_t count = generic_report_pending;
                            generic_report_pending = 0;
                            for (int_fast8_t i = 0; i < count; ++i) {
                                usb_tx(generic_report[i]);
                            }
                        } else if (make_generic_hid_report(0, GENERIC_HID_REPORT_SIZE, generic_report)) {
                            for (int_fast8_t i = 0; i < GENERIC_HID_REPORT_SIZE; ++i) {
                                usb_tx(generic_report[i]);
                            }
                            generic_idle_count = 0;
                        }
                    }
                    usb_release_tx();
                }
            }
#endif // ^ ENABLE_GENERIC_HID_ENDPOINT
        }
    }

    if (intflags & INT_WAKE_UP_FLAG) {
        usb_disable_interrupts(INT_WAKE_UP_FLAG);
        usb_enable_interrupts(INT_SUSPEND_FLAG);
        usb_suspended = false;
        usb_clear_interrupts(INT_WAKE_UP_FLAG);
    } else if (intflags & INT_SUSPEND_FLAG) {
        usb_disable_interrupts(INT_SUSPEND_FLAG);
        usb_enable_interrupts(INT_WAKE_UP_FLAG);
        usb_suspended = true;
        usb_clear_interrupts(INT_SUSPEND_FLAG | INT_WAKE_UP_FLAG);
    }
}

ISR(USB_COM_vect) {
    usb_set_endpoint(0);

    if (!is_usb_rx_int_setup) {
        usb_stall();
        return;
    }

    bool success = true;
    uint8_t i;
    uint8_t type = usb_rx();
    uint8_t request = usb_rx();
    uint_fast16_t value = usb_rx();
    value |= usb_rx() << 8;
    uint_fast16_t index = usb_rx();
    index |= usb_rx() << 8;
    uint_fast16_t length = usb_rx();
    length |= usb_rx() << 8;

    usb_clear_setup_int();

    if (type & USB_REQUEST_DIRECTION_TO_HOST) {
        usb_wait_tx_in();
    } else {
        usb_flush_tx_in();
    }

    if ((type & USB_REQUEST_TYPE_MASK) == USB_REQUEST_TYPE_STANDARD) {
        if (request == USB_REQUEST_GET_DESCRIPTOR) {
            const char *data = 0;
            uint8_t desc_length;

            for (i = 0; i < descriptor_count; ++i) {
                if (pgm_read_word(&(descriptor_list[i].value)) != value) {
                    continue;
                }
                if (pgm_read_word(&(descriptor_list[i].index)) != index) {
                    continue;
                }
                data = pgm_read_ptr(&(descriptor_list[i].data));
                desc_length = pgm_read_byte(&(descriptor_list[i].length));
                break;
            }
            if (!data) {
                // Descriptor not found
                usb_stall();
                usb_error = 'D';
                return;
            }

            // 1 = string, 0 = data
            type = (index != 0 && MSB(value) == DESCRIPTOR_TYPE_STRING);

            if (desc_length < length) {
                length = desc_length;
            }
            request = (desc_length % ENDPOINT_0_SIZE);
            while (length) {
                // Split into packets of at most endpoint size
                i = (length < ENDPOINT_0_SIZE) ? length : ENDPOINT_0_SIZE;
                length -= i;

                if (type) {
                    // String - convert ASCII to UTF-16 on the fly

                    i /= 2; // The length of string descriptors is always even
                    if (desc_length) {
                        // The header, only send in the first packet
                        if (i) {
                            --i;
                            usb_tx(desc_length);
                            desc_length = 0;
                            usb_tx(DESCRIPTOR_TYPE_STRING);
                        }
                    }
                    while (i--) {
                        usb_tx(pgm_read_byte(data++));
                        usb_tx(0); // Upper byte of the UTF-16 string
                    }
                } else {
                    // Normal data, already in the correct format
                    while (i--) {
                        usb_tx(pgm_read_byte(data++));
                    }
                }

                usb_flush_tx_in();

                usb_wait_in_or_out();
                if (is_usb_rx_out_ready) {
                    // Abort
                    return;
                }
            }
            if (request) {
                return; // Return since we already flushed the last packet
            }
            // The data structure was an exact multiple of the endpoint
            // size, we need to tell that it is over by sending an empty
            // packet. (The flush as the end of this function.)
        } else if (request == USB_REQUEST_SET_ADDRESS) {
            // Set address
            usb_wait_tx_in();
            usb_set_address(value);
        } else if (request == USB_REQUEST_GET_STATUS) {
            // Get status
            if (type == USB_REQUEST_DEVICE_TO_HOST_STANDARD_DEVICE) {
                i = usb_status;
            } else if (type == USB_REQUEST_DEVICE_TO_HOST_STANDARD_ENDPOINT) {
                i = index & 0xFF;
                usb_set_endpoint(i);
                i = is_usb_stall_requested ? 1 : 0;
                usb_set_endpoint(0);
            } else {
                i = 0;
            }
            usb_tx(i);
            usb_tx(0);
        } else if (request == USB_REQUEST_GET_CONFIGURATION) {
            // Get configuration
            if (type == USB_REQUEST_DEVICE_TO_HOST_STANDARD_DEVICE) {
                usb_tx(usb_configuration);
            } else {
                usb_tx(1);
            }
        } else if (request == USB_REQUEST_SET_CONFIGURATION) {
            // Set configuration
            if ((type & USB_REQUEST_RECIPIENT_MASK) == USB_REQUEST_RECIPIENT_DEVICE) {
                usb_init_endpoints();
                if (value <= CONFIGURATIONS_COUNT) {
                    usb_configuration = value;
                } else {
                    success = false;
                }
            } else {
                success = false;
            }
        } else if (request == USB_REQUEST_CLEAR_FEATURE || request == USB_REQUEST_SET_FEATURE) {
            // Set or clear a feature
            if (type == USB_REQUEST_HOST_TO_DEVICE_STANDARD_DEVICE) {
                if (value == USB_FEATURE_DEVICE_REMOTE_WAKEUP) {
                    // Remote wakeup
                    if (request == USB_REQUEST_CLEAR_FEATURE) {
                        usb_status &= ~USB_STATUS_REMOTE_WAKEUP_ENABLED;
                    } else {
                        usb_status |= USB_STATUS_REMOTE_WAKEUP_ENABLED;
                    }
                }
            } else if (type == USB_REQUEST_HOST_TO_DEVICE_STANDARD_ENDPOINT) {
                if (value == USB_FEATURE_HALT_ENDPOINT) {
                    // Halt
                    i = index & 0x7FU;
                    if (i != 0 && i <= USB_MAX_ENDPOINT) {
                        usb_set_endpoint(i);
                        if (request == USB_REQUEST_CLEAR_FEATURE) {
                            usb_clear_stall();
                            usb_reset_endpoint(i);
                        } else {
                            usb_stall();
                        }
                        usb_set_endpoint(0);
                    }
                }
            }
        } else if (request == USB_REQUEST_SET_DESCRIPTOR) {
            success = false;
        } else if (request == USB_REQUEST_GET_INTERFACE) {
            if (usb_configuration && length == 1) {
                // index = interface number
                usb_tx(0); // alternate setting
            } else {
                success = false;
            }
        } else if (request == USB_REQUEST_SET_INTERFACE) {
            if (usb_configuration && index < INTERFACES_COUNT && value == 0) {
                // index = interface number
                // value = alternate setting
            } else {
                success = false;
            }
        }
    } else if (index == KEYBOARD_INTERFACE_INDEX) {
        if (type == USB_REQUEST_DEVICE_TO_HOST_CLASS_INTERFACE) {
            if (request == HID_REQUEST_GET_REPORT) {
                usb_wait_tx_in();
                usb_tx_keys_state();
            } else if (request == HID_REQUEST_GET_IDLE) {
#if IDLE_COUNT_FRAME_DIVIDER == 4
                usb_tx(keyboard_update_on_idle_count);
#else
                value = keyboard_update_on_idle_count;
                i = (value * IDLE_COUNT_FRAME_DIVIDER) / 4;
                usb_tx(i);
#endif
            } else if (request == HID_REQUEST_GET_PROTOCOL) {
                usb_tx(keyboard_protocol);
            }
        } else if (type == USB_REQUEST_HOST_TO_DEVICE_CLASS_INTERFACE) {
            if (request == HID_REQUEST_SET_REPORT) {
                usb_wait_rx_out();
                keyboard_leds = usb_rx();
                usb_ack_rx_out();
            } else if (request == HID_REQUEST_SET_IDLE) {
                keyboard_idle_count = 0;
                keyboard_update_on_idle_count = (value >> 6) / IDLE_COUNT_FRAME_DIVIDER;
            } else if (request == HID_REQUEST_SET_PROTOCOL) {
                keyboard_protocol = value;
            }
        } else {
            success = false;
        }
    }
#if ENABLE_GENERIC_HID_ENDPOINT
    else if (index == GENERIC_INTERFACE_INDEX) {
        if (type == USB_REQUEST_DEVICE_TO_HOST_CLASS_INTERFACE) {
            if (request == HID_REQUEST_GET_REPORT) {
                if (length > GENERIC_HID_REPORT_SIZE) {
                    length = GENERIC_HID_REPORT_SIZE;
                }
                if (make_generic_hid_report(value & 0xFFU, length, generic_report)) {
                    for (i = 0; i < length; ++i) {
                        usb_tx(generic_report[i]);
                    }
                } else {
                    success = false;
                }
            } else if (request == HID_REQUEST_GET_IDLE) {
#if IDLE_COUNT_FRAME_DIVIDER == 4
                usb_tx(generic_update_on_idle_count);
#else
                value = generic_update_on_idle_count;
                i = (value * IDLE_COUNT_FRAME_DIVIDER) / 4;
                usb_tx(i);
#endif
            } else if (request == HID_REQUEST_GET_PROTOCOL) {
                usb_tx(INTERFACE_NO_SPECIFIC_PROTOCOL);
            }
        } else if (type == USB_REQUEST_HOST_TO_DEVICE_CLASS_INTERFACE) {
            if (request == HID_REQUEST_SET_REPORT) {
                if (length > GENERIC_HID_FEATURE_SIZE) {
                    length = GENERIC_HID_FEATURE_SIZE;
                }

#if GENERIC_HID_HANDLE_SYNCHRONOUSLY
                if (generic_request_pending) {
                    success = false;
                } else {
                    generic_request_pending_id = value & 0xFFU;
#else
                {
#endif
                    usb_wait_rx_out();
                    for (i = 0; i < length; ++i) {
                        generic_request[i] = usb_rx();
                    }
                    usb_ack_rx_out();
#if GENERIC_HID_HANDLE_SYNCHRONOUSLY
                    generic_request_pending = length;
#else
                    success = generic_request_call_handler(value & 0xFFU, length);
#endif
                }
            } else if (request == HID_REQUEST_SET_IDLE) {
                generic_idle_count = 0;
#if GENERIC_HID_UPDATE_IDLE_MS
                generic_update_on_idle_count = (value >> 6) / IDLE_COUNT_FRAME_DIVIDER;
#endif
            }
        } else {
            success = false;
        }
    }
#endif // ^ ENABLE_GENERIC_HID_ENDPOINT
#if ENABLE_DFU_INTERFACE
    else if (index == DFU_INTERFACE_INDEX) {
        if (type == USB_REQUEST_HOST_TO_DEVICE_CLASS_INTERFACE) {
            if (request == DFU_REQUEST_DETACH) {
                if ((value >> 8)) {
                    usb_request_detach = 0xFFU;
                } else {
                    usb_request_detach = value ? value : 1;
                }
            } else {
                success = false;
            }
        } else if (type == USB_REQUEST_DEVICE_TO_HOST_CLASS_INTERFACE) {
            if (request == DFU_REQUEST_GET_STATE) {
                usb_tx(dfu_app_state);
            } else if (request == DFU_REQUEST_GET_STATUS) {
                usb_tx(DFU_STATUS_OK);              // bStatus
                usb_tx(0); usb_tx(0); usb_tx(0);    // bwPollTimeout
                usb_tx(dfu_app_state);              // bState
                usb_tx(STRING_INDEX_PRODUCT);       // iString
            } else {
                success = false;
            }
        } else {
            success = false;
        }
    }
#endif // ^ ENABLE_DFU_INTERFACE
    else {
        success = false;
    }

    if (success) {
        usb_flush_tx_in();
        usb_error = 0;
    } else {
        usb_stall();
        usb_error = 'R';
    }
}

void
usb_deinit (void) {
    usb_release_all_keys();
    keyboard_leds = 5;
    usb_keyboard_send_report();
    _delay_ms(8);

    cli();
    usb_detach();
    usb_freeze();
    _delay_ms(8);
    usb_disable();
    sei();
}

#if (USB_MAX_KEY_ROLLOVER + ENABLE_APPLE_FN_KEY) < USB_BOOT_PROTOCOL_ROLLOVER
#error "USB_MAX_KEY_ROLLOVER must be at least 6 (or 5 with ENABLE_APPLE_FN_KEY)"
#endif
#if MAX_KEY_ROLLOVER < USB_MAX_KEY_ROLLOVER
#error "MAX_KEY_ROLLOVER must be at least equal to USB_MAX_KEY_ROLLOVER"
#endif
#if ENABLE_APPLE_FN_KEY && USB_VENDOR_ID != USB_VENDOR_ID_APPLE
#error "USB_VENDOR_ID must be USB_VENDOR_ID_APPLE for ENABLE_APPLE_FN_KEY"
#endif
