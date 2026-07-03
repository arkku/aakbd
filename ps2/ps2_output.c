/**
 * ps2_output.c: PS/2 keyboard output device.
 *
 * Copyright (c) 2026 Kimmo Kulovesi, https://arkku.dev/
 *
 * This is the implementation of a PS/2 keyboard device.
 *
 * It turns out to be surprisingly difficult to find any sort of complete
 * specification for the PS/2 keyboard protocol, especially since no-one
 * seems to bother implementing scancode set 3. Yet, that scancode set is
 * the most sensible to use if supported.
 *
 * I used some datasheets for PS/2 device controller chips as the primary
 * source of information: since those were the actual controllers used in
 * keyboards, at least their specifications should match what is out there
 * in the real world. Also looking at open source PS/2 host implementations
 * (including my own) helped, but of course I can't possibly test every
 * combination. Tell me if you find incompatibilites.
 *
 * There is also an AI-generated comprehensive test suite for this, but it is
 * entirely possible some of that is incorrect as I have not read through it.
 * (This file, ps2_output.c, is human-generated.)
 *
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

#ifndef ENABLE_PS2_DEVICE
#define ENABLE_PS2_DEVICE 1
#endif

#include "kk_ps2_device.h"
#include "usb2ps2_keys.h"
#include <qmk_core/platforms/timer.h>
#include <stdint.h>

#define KK_KEYCODES_INCLUDE_DUPLICATES 1
#include "ps2_keys.h"

#define USB_KEYBOARD_ACCESS_STATE 1
#include "usbkbd.h"

#include "kk_ps2.h"
#include "ps2_output.h"
#include "usb_keys.h"
#include "keys.h"

#ifndef PS2_RESET_DURATION_MS
/// The time we wait for the power-up reset to happen. In reality our reset
/// is basically instantaneous, but we can simulate a classic PS/2 keyboard
/// more accurately.
#define PS2_RESET_DURATION_MS 500
#endif

#ifndef PS2_OUTPUT_MAX_KEY_EVENTS
/// The maximum number of _unsent_ key events (press or release) that can be
/// pending at once. This is NOT rollover; PS/2 has no such limit. This is the
/// limit on the number of keys that can be pressed or release at once between
/// getting sent over the wire. e.g., if a key is mapped to press 4 modifiers
/// at once, this would generate 4 events from a single press. On the other
/// hand, even a couple of milliseconds between keypresses allows sending
/// some queued events, making room for new ones.
#define PS2_OUTPUT_MAX_KEY_EVENTS 16
#endif

#ifndef PS2_OUTPUT_MAX_EVENTS_PER_TASK
/// The maximum number of  key events can be output per one call of
/// `ps2_output_task()`. The trade-off is that the task holds off other
/// main loop stuff, like matrix scanning, while sending these. On the
/// other hand, sending too few may make multi-modifier combos mapped
/// to one key seem unresponsive (each modifier is a separate event).
#define PS2_OUTPUT_MAX_EVENTS_PER_TASK 2
#endif

#ifndef PS2_OUTPUT_NUM_LOCK_LED_EVENT
/// Enabling this treats a Num Lock LED toggle from the host as kind of
/// equivalent to having pressed the Num Lock key locally if keys are held
/// down that are affected by this. This makes sense on computers that may
/// toggle Num Lock from another (e.g., USB) keyboard or just from software.
/// However, this does not match what the original IBM Model M PS/2 keyboard
/// does, so it is optional.
#define PS2_OUTPUT_NUM_LOCK_LED_EVENT 1
#endif

// MARK: - Stored State

/// PS/2 output state flags.
static uint8_t ps2_output_flags = 0;

#define FLAG_OUTPUT_INITIALIZED         ((uint8_t) 0x01U)
#define FLAG_SHIFT_SUPPRESSED_LEFT      ((uint8_t) 0x02U)
#define FLAG_SHIFT_VIRTUAL_ON           ((uint8_t) 0x04U)
#define FLAG_SHIFT_VIRTUAL_WAS_ACTIVE   ((uint8_t) 0x08U)
#define FLAG_HOST_ACTIVE                ((uint8_t) 0x10U)
#define FLAG_SHIFT_SUPPRESSED_RIGHT     ((uint8_t) 0x20U)
#define FLAG_WAS_SCANNING_ENABLED       ((uint8_t) 0x40U)
#define FLAG_SCANNING_ENABLED           ((uint8_t) 0x80U)

/// The active scancode set (1, 2 or 3).
static uint8_t ps2_active_scancode_set = PS2_KEYBOARD_DEFAULT_SCANCODE_SET;

/// Timer for the pending reset (`PS2_RESET_DURATION_MS`).
static uint16_t reset_pending_since = 0;

/// The state of the modifiers (Shift, Ctrl, Alt, Cmd) flags. These use the
/// same bit flags as the USB modifiers.
static uint8_t ps2_modifiers = 0;

// MARK: Keypress event queue

#if (256 % PS2_OUTPUT_MAX_KEY_EVENTS) == 0
// The event queue size is a power of two, we can optimize a bit.

#define PS2_OUTPUT_KEY_EVENT_SENTINEL_COUNT 0

#define key_event_queue_count           ((uint8_t) (key_event_queue_head - key_event_queue_tail))
#define is_key_event_queue_empty        (key_event_queue_count == 0)
#define is_key_event_queue_full         (key_event_queue_count == PS2_OUTPUT_MAX_KEY_EVENTS)
#define key_event_head                  modulo_key_event_queue(key_event_queue_head)
#define key_event_tail                  modulo_key_event_queue(key_event_queue_tail)
#define increment_key_event_queue()     do { ++key_event_queue_head; } while (0)
#define decrement_key_event_queue()     do { ++key_event_queue_tail; } while (0)
#define unshift_key_event_queue()       do { --key_event_queue_tail; } while (0)

#else

#define PS2_OUTPUT_KEY_EVENT_SENTINEL_COUNT 1

#define is_key_event_queue_empty        (key_event_queue_head == key_event_queue_tail)
#define is_key_event_queue_full         (modulo_key_event_queue(key_event_queue_head + 1) == key_event_queue_tail)
#define key_event_head                  (key_event_queue_head)
#define key_event_tail                  (key_event_queue_tail)
#define increment_key_event_queue()     do { key_event_queue_head = modulo_key_event_queue(key_event_queue_head + 1); } while (0)
#define decrement_key_event_queue()     do { key_event_queue_tail = modulo_key_event_queue(key_event_queue_tail + 1); } while (0)
#define unshift_key_event_queue()       do { key_event_queue_tail = modulo_key_event_queue(key_event_queue_tail - 1); } while (0)

#endif

static struct {
    uint8_t key;
    uint8_t is_release;
} key_event_queue[PS2_OUTPUT_MAX_KEY_EVENTS + PS2_OUTPUT_KEY_EVENT_SENTINEL_COUNT];

#define modulo_key_event_queue(index)   ((index) % (PS2_OUTPUT_MAX_KEY_EVENTS + PS2_OUTPUT_KEY_EVENT_SENTINEL_COUNT))

/// A special marker in `key_event_queue`, which is not a keypress. The
/// `is_release` field will then define which event it is.
#define KEY_EVENT_SPECIAL ((uint8_t) 0U)

#define SPECIAL_EVENT_ALL_KEYS_RELEASED ((uint8_t) 0U)
#if PS2_OUTPUT_NUM_LOCK_LED_EVENT
#define SPECIAL_EVENT_NUM_LOCK_TOGGLED ((uint8_t) 1U)
#endif

/// Key event ring buffer head.
uint8_t key_event_queue_head = 0;

/// Key event ring buffer tail.
uint8_t key_event_queue_tail = 0;

// MARK: Tenkey Tracking

/// The count of tenkey cluster keys being held down currently.
/// Why does it matter? Because in scancode sets 1 and 2, these keys have
/// virtual modifiers added depending on shift and num lock states,
/// and those need to be undone after the last tenkey cluster key release.
static uint8_t tenkey_count = 0;

// MARK: Repeat Tracking

/// Key repeat state. At most one key can be repeating at once. This saves
/// the scancode (in the current PS/2 scancode set), flags, and timestamp.
/// The timestamp is the last event of that key, i.e., initially the original
/// press, later the previous repetition.
static struct {
    uint8_t scancode;
    uint8_t flags;
    uint16_t timestamp;
} ps2_repeat_key;

#define REPEAT_FLAG_KEY_IS_EXTENDED ((uint8_t) 0x01U)
#define REPEAT_FLAG_IS_REPEATING    ((uint8_t) 0x02U)

/// The configured F3 typematic rate/delay byte (default: ~10.9 cps, 500 ms).
static uint8_t repeat_rate = PS2_KEYBOARD_DEFAULT_REPEAT_RATE;

/// LED state set by the PS/2 host.
static uint8_t host_led_state = 0;

// MARK: Command State

/// The command that is pending arguments, or 0 if none. Part of the command
/// processing state machine, along with `pending_argc`.
uint8_t pending_cmd = 0;

/// How many bytes have been received for `pending_cmd` (including the command
/// itself, so essentially starting from 1, same as `argc` in `main(…)`.
///
/// This keeps the state within the `pending_cmd` itself. As a special case,
/// set to `ALL_ARGS_READ` when the command should no longer receive more
/// arguments, but the output still needs to be flushed before ceasing the
/// command processing.
static uint8_t pending_argc = 0;

/// All arguments have been read for the current pending command.
#define ALL_ARGS_READ 255

// MARK: - Helpers

static inline void
send_byte (const uint8_t data) {
    (void) ps2_device_send(data);
}

static inline void
send_2_bytes (const uint8_t a, const uint8_t b) {
    if (ps2_device_send(a)) {
        (void) ps2_device_send(b);
    }
}

static void
send_3_bytes (const uint8_t a, const uint8_t b, const uint8_t c) {
    if (ps2_device_send(a)) {
        if (ps2_device_send(b)) {
            (void) ps2_device_send(c);
        }
    }
}

/// Converts PS/2 LED flags to USB LED flags.
static uint8_t
ps2_leds_to_usb (const uint8_t ps2_leds) {
    return ((ps2_leds & PS2_LED_SCROLL_LOCK_BIT) ? LED_SCROLL_LOCK_BIT : 0) | ((ps2_leds & PS2_LED_NUM_LOCK_BIT) ? LED_NUM_LOCK_BIT : 0) | ((ps2_leds & PS2_LED_CAPS_LOCK_BIT) ? LED_CAPS_LOCK_BIT : 0);
}

/// Is the PS/2 host active (have we received something from them)?
#define is_ps2_host_active (ps2_output_flags & FLAG_HOST_ACTIVE)

/// Marks the PS/2 host active (they have sent something).
#define ps2_set_host_active() (ps2_output_flags |= FLAG_HOST_ACTIVE)

/// Is keyboard scanning (reacting to keypresses) enabled?
#define is_ps2_scanning_enabled (ps2_output_flags & FLAG_SCANNING_ENABLED)

/// Enables keyboard scanning.
#define ps2_enable_scanning() (ps2_output_flags |= FLAG_SCANNING_ENABLED)

/// Disables keyboard scanning. Save the previous state.
static inline void
ps2_disable_scanning (void) {
    if (ps2_output_flags & FLAG_SCANNING_ENABLED) {
        ps2_output_flags |= FLAG_WAS_SCANNING_ENABLED;
    } else {
        ps2_output_flags &= ~FLAG_WAS_SCANNING_ENABLED;
    }
    ps2_output_flags &= ~FLAG_SCANNING_ENABLED;
}

/// Re-enables keyboard scanning if it was enabled when last disabled.
static inline void
ps2_restore_scanning_state (void) {
    if (ps2_output_flags & FLAG_WAS_SCANNING_ENABLED) {
        ps2_output_flags |= FLAG_SCANNING_ENABLED;
    }
}

/// Stop the key with `scancode` from repeating, if it was repeating.
/// As a special case, pass `0` for `scancode` to stop any active repeat.
static void
clear_repeat_for_key (const uint8_t scancode, const bool is_extended) {
    if (!scancode || (ps2_repeat_key.scancode == scancode && (ps2_repeat_key.flags & REPEAT_FLAG_KEY_IS_EXTENDED) == (is_extended ? REPEAT_FLAG_KEY_IS_EXTENDED : 0))) {
        ps2_repeat_key.scancode = 0;
    }
}

/// Clear any active repeat. (This does not stop repeating of new keypresses,
/// just stops any currently held keys from repeating.)
static inline void
clear_repeat (void) {
    clear_repeat_for_key(0, false);
}

static void set3_init_defaults(void);

/// Set the default state and clear repeats.
static void
ps2_output_set_defaults (void) {
    ps2_output_clear_keys(true);
    repeat_rate = PS2_KEYBOARD_DEFAULT_REPEAT_RATE;
    set3_init_defaults();
}

/// Perform a reset of the keyboard.
/// - Parameter include_power_up_delay: If `true`, delay to simulate a real
///   keyboard restarting (asynchronously). Otherwise reset immediately.
static void
ps2_output_reset (const bool include_power_up_delay) {
    ps2_active_scancode_set = PS2_KEYBOARD_DEFAULT_SCANCODE_SET;
    host_led_state = 0;
    ps2_output_set_defaults();
    if (include_power_up_delay) {
        ps2_disable_scanning();
        usb_keyboard_leds = PS2_LED_SCROLL_LOCK_BIT | PS2_LED_NUM_LOCK_BIT | PS2_LED_CAPS_LOCK_BIT;
        reset_pending_since = timer_read() | 1;
    } else {
        ps2_enable_scanning();
        usb_keyboard_leds = 0;
        reset_pending_since = 0;
        send_byte(PS2_REPLY_TEST_PASSED);
    }
}

void
ps2_output_init (void) {
    pending_cmd = 0;
    ps2_output_reset(true);
    ps2_device_attach();
    ps2_output_flags = FLAG_OUTPUT_INITIALIZED;
}

// MARK: - Alternative Scancode Sets

#if ENABLE_PS2_DEVICE_SET_3
#define SET3_NATIVE_KEY_MAX 0x8F
#define SET3_STATE_BYTES    ((SET3_NATIVE_KEY_MAX / 8) + 1)
#define BITMAP_BYTE(sc)     ((sc) / 8)
#define BITMAP_BIT(sc)      ((sc) % 8)

/// Is break enabled for set 3 keys (1 bit per key)?
static unsigned char ps2_key_break_state[SET3_STATE_BYTES];

/// Is repeat enabled for set 3 keys (1 bit per key)?
static unsigned char ps2_key_repeat_state[SET3_STATE_BYTES];

/// Is this a "native" set 3 scancode? We only maintain per key state for the
/// "native" keys, extended extras can't be configured per key.
#define set3_is_native(sc) ((sc) <= SET3_NATIVE_KEY_MAX)

static inline bool
is_set3_break_enabled_for (const uint8_t scancode) {
    return set3_is_native(scancode) ? (ps2_key_break_state[BITMAP_BYTE(scancode)] >> BITMAP_BIT(scancode)) & 1 : true;
}

static inline void
set3_set_break (const uint8_t scancode, const bool enable) {
    if (set3_is_native(scancode)) {
        if (enable) {
            ps2_key_break_state[BITMAP_BYTE(scancode)] |= 1U << BITMAP_BIT(scancode);
        } else {
            ps2_key_break_state[BITMAP_BYTE(scancode)] &= ~(1U << BITMAP_BIT(scancode));
        }
    }
}

static inline bool
is_set3_repeat_enabled_for (const uint8_t scancode) {
    return set3_is_native(scancode) ? (ps2_key_repeat_state[BITMAP_BYTE(scancode)] >> BITMAP_BIT(scancode)) & 1 : true;
}

static inline void
set3_set_repeat (const uint8_t scancode, const bool enable) {
    if (set3_is_native(scancode)) {
        if (enable) {
            ps2_key_repeat_state[BITMAP_BYTE(scancode)] |= 1U << BITMAP_BIT(scancode);
        } else {
            ps2_key_repeat_state[BITMAP_BYTE(scancode)] &= ~(1U << BITMAP_BIT(scancode));
        }
    }
}

static inline void
set3_set_break_and_repeat (const uint8_t scancode, const bool break_enabled, const bool repeat_enabled) {
    set3_set_break(scancode, break_enabled);
    set3_set_repeat(scancode, repeat_enabled);
}

static void
set3_set_all_break (bool enabled) {
    for (int i = 0; i < SET3_STATE_BYTES; ++i) {
        ps2_key_break_state[i] = enabled ? 0xFFU : 0;
    }
}

static void
set3_set_all_repeat (bool enabled) {
    for (int i = 0; i < SET3_STATE_BYTES; ++i) {
        ps2_key_repeat_state[i] = enabled ? 0xFFU : 0;
    }
}

static void
set3_init_defaults (void) {
    for (int i = 0; i < SET3_STATE_BYTES; ++i) {
        ps2_key_break_state[i] = 0;
        ps2_key_repeat_state[i] = 0xFF;
    }
    set3_set_break_and_repeat(KEY_CAPS_LOCK, 1, 0);
    set3_set_break_and_repeat(KEY_LEFT_SHIFT, 1, 0);
    set3_set_break_and_repeat(KEY_RIGHT_SHIFT, 1, 0);
    set3_set_break_and_repeat(KEY_LEFT_CTRL, 1, 0);
    set3_set_break_and_repeat(KEY_RIGHT_CTRL, 1, 0);
    set3_set_break_and_repeat(KEY_LEFT_ALT, 1, 0);
    set3_set_break_and_repeat(KEY_RIGHT_ALT, 1, 0);
    set3_set_break_and_repeat(KEY_LEFT_WIN, 1, 0);
    set3_set_break_and_repeat(KEY_MENU, 1, 0);
}

#define is_scancode_set_3_active (ps2_active_scancode_set == 3)

#else // ^ENABLE_PS2_DEVICE_SET_3

#define is_set3_break_enabled_for(sc)  (true)
#define is_set3_repeat_enabled_for(sc) (true)
#define is_scancode_set_3_active       (false)

static void
set3_init_defaults (void) {
}

#endif // !ENABLE_PS2_DEVICE_SET_3

#define SET1_MAKE_TO_BREAK(sc) ((sc) ^ 0x80)

#if ENABLE_PS2_DEVICE_SET_1
#define is_scancode_set_1_active (ps2_active_scancode_set == 1)
#else
#define is_scancode_set_1_active (false)
#endif

// MARK: - Keypress Processing

static inline void
send_ext_key_make (const uint8_t keycode) {
    send_2_bytes(PS2_EXT_PREFIX, keycode);
}

static inline void
send_ext_key_set2_break (const uint8_t keycode) {
    send_3_bytes(PS2_EXT_PREFIX, PS2_BREAK_PREFIX, keycode);
}

static inline void
send_ext_key_set1_break (const uint8_t keycode) {
    send_2_bytes(PS2_EXT_PREFIX, SET1_MAKE_TO_BREAK(keycode));
}

static void
virtual_shift_on (void) {
    if (!(ps2_modifiers & BOTH_SHIFT_BITS) && !(ps2_output_flags & FLAG_SHIFT_VIRTUAL_ON)) {
        ps2_output_flags &= ~FLAG_SHIFT_VIRTUAL_WAS_ACTIVE;
        ps2_output_flags |= FLAG_SHIFT_VIRTUAL_ON;
        send_ext_key_make(is_scancode_set_1_active ? KEY_LEFT_SHIFT_SET1 : KEY_LEFT_SHIFT);
    }
}

static void
virtual_shift_off (void) {
    if (ps2_output_flags & FLAG_SHIFT_VIRTUAL_ON) {
        ps2_output_flags &= ~FLAG_SHIFT_VIRTUAL_ON;
        ps2_output_flags |= FLAG_SHIFT_VIRTUAL_WAS_ACTIVE;
        if (is_scancode_set_1_active) {
            send_ext_key_set1_break(KEY_LEFT_SHIFT_SET1);
        } else {
            send_ext_key_set2_break(KEY_LEFT_SHIFT);
        }
    }
}

static void
shift_suppress (void) {
    _Static_assert(FLAG_SHIFT_SUPPRESSED_LEFT == SHIFT_BIT);
    _Static_assert(FLAG_SHIFT_SUPPRESSED_RIGHT == RIGHT_SHIFT_BIT);

    if (ps2_modifiers & SHIFT_BIT) {
        if (is_scancode_set_1_active) {
            send_ext_key_set1_break(KEY_LEFT_SHIFT_SET1);
        } else {
            send_ext_key_set2_break(KEY_LEFT_SHIFT);
        }
    }
    if (ps2_modifiers & RIGHT_SHIFT_BIT) {
        if (is_scancode_set_1_active) {
            send_ext_key_set1_break(KEY_RIGHT_SHIFT_SET1);
        } else {
            send_ext_key_set2_break(KEY_RIGHT_SHIFT);
        }
    }
    ps2_output_flags |= (ps2_modifiers & BOTH_SHIFT_BITS);
}

static void
shift_unsuppress (void) {
    const uint8_t to_restore = (ps2_output_flags & BOTH_SHIFT_BITS) & ps2_modifiers;
    if (to_restore & RIGHT_SHIFT_BIT) {
        send_ext_key_make(is_scancode_set_1_active ? KEY_RIGHT_SHIFT_SET1 : KEY_RIGHT_SHIFT);
    }
    if (to_restore & SHIFT_BIT) {
        send_ext_key_make(is_scancode_set_1_active ? KEY_LEFT_SHIFT_SET1 : KEY_LEFT_SHIFT);
    }
    ps2_output_flags &= ~to_restore;
}

static inline bool
is_tenkey_cluster_key (const uint8_t key) {
    return key >= USB_KEY_INSERT && key <= USB_KEY_UP_ARROW;
}

void
ps2_send_key_press (const uint8_t usb_keycode) {
    if (!is_ps2_scanning_enabled) {
        return;
    }

    uint8_t scancode = ps2_scancode_for_usb_keycode(usb_keycode, ps2_active_scancode_set);

    if (!scancode) {
#if ENABLE_PS2_DEVICE_SET_3 && defined(ENABLE_PS2_DEVICE_SET_3_F24) && ENABLE_PS2_DEVICE_SET_3_F24
        // Set 3 has no native scancodes for all function keys beyond F12,
        // simulate via Shift + F1-F12 if needed
        if (usb_keycode >= USB_KEY_F13 && usb_keycode <= USB_KEY_F24) {
            const uint8_t f_scancode = ps2_scancode_for_usb_keycode(
                USB_KEY_F1 + (usb_keycode - USB_KEY_F13), ps2_active_scancode_set);
            if (f_scancode) {
                const bool shift_held = ps2_modifiers & BOTH_SHIFT_BITS;
                if (!shift_held) {
                    send_byte(KEY_LEFT_SHIFT);
                }
                send_byte(f_scancode);
                if (!shift_held) {
                    send_2_bytes(PS2_BREAK_PREFIX, KEY_LEFT_SHIFT);
                }
            }
        }
#endif
        return;
    }

    const bool is_extended = is_extended_ps2_key(usb_keycode, ps2_active_scancode_set);

    {
        bool can_repeat;
        if (is_scancode_set_3_active) {
            // In set 3 repeat can be toggled per key
            can_repeat = is_extended || is_set3_repeat_enabled_for(scancode);
        } else {
            // In sets 1 and 2, the Pause/Break key is the only non-repeating
            can_repeat = (usb_keycode != USB_KEY_PAUSE_BREAK);
        }
        if (can_repeat) {
            if (scancode) {
                ps2_repeat_key.scancode = scancode;
                ps2_repeat_key.flags = is_extended ? REPEAT_FLAG_KEY_IS_EXTENDED : 0;
            }
            ps2_repeat_key.timestamp = timer_read();
        } else {
            clear_repeat();
        }
    }

    if (!is_scancode_set_3_active) {
        virtual_shift_off();
        shift_unsuppress();

        if (is_tenkey_cluster_key(usb_keycode)) {
            ps2_output_flags &= ~FLAG_SHIFT_VIRTUAL_WAS_ACTIVE;
            if (host_led_state & LED_NUM_LOCK_BIT) {
                virtual_shift_on();
            } else {
                shift_suppress();
            }
            ++tenkey_count;
        }

        switch (usb_keycode) {
        case USB_KEY_PRINT_SCREEN:
            if (ps2_modifiers & BOTH_ALT_BITS) {
                const uint8_t sysrq = is_scancode_set_1_active ? KEY_ALT_SYSRQ_SET1 : KEY_ALT_SYSRQ_SET2;
                send_byte(sysrq);
                ps2_repeat_key.scancode = sysrq;
                ps2_repeat_key.flags = 0;
                return;
            }
            if (!(ps2_modifiers & (CTRL_BIT | RIGHT_CTRL_BIT | SHIFT_BIT | RIGHT_SHIFT_BIT))) {
                virtual_shift_on();
            }
            break;

        case USB_KEY_PAUSE_BREAK:
            if (ps2_modifiers & BOTH_CTRL_BITS) {
                // Pause is a weird special case, so it happens that the
                // scancode resolves to the one with Ctrl active (other parts
                // of the scancode are not unique)
                send_ext_key_make(scancode);
            } else if (is_scancode_set_1_active) {
                send_2_bytes(PS2_PAUSE_PREFIX, KEY_LEFT_CTRL_SET1);
                send_2_bytes(KEY_NUM_LOCK_SET1, PS2_PAUSE_PREFIX);
                send_2_bytes(SET1_MAKE_TO_BREAK(KEY_LEFT_CTRL_SET1), SET1_MAKE_TO_BREAK(KEY_NUM_LOCK_SET1));
            } else {
                send_2_bytes(PS2_PAUSE_PREFIX, EXTENDED_KEY_PAUSE_SET2);
                send_2_bytes(KEY_NUM_LOCK_SET2, PS2_PAUSE_PREFIX);
                send_2_bytes(PS2_BREAK_PREFIX, EXTENDED_KEY_PAUSE_SET2);
                send_2_bytes(PS2_BREAK_PREFIX, KEY_NUM_LOCK_SET2);
            }
            return;

        case USB_KEY_KP_DIVIDE:
            shift_suppress();
            break;

        default:
            break;
        }
    }

    if (is_extended) {
        send_ext_key_make(scancode);
    } else {
        send_byte(scancode);
    }

    if (IS_MODIFIER(usb_keycode)) {
        ps2_modifiers |= MODIFIER_BIT(usb_keycode);
    }
}

static void
clear_key_state (void) {
    ps2_output_flags &= ~(FLAG_SHIFT_VIRTUAL_ON | FLAG_SHIFT_SUPPRESSED_LEFT | FLAG_SHIFT_SUPPRESSED_RIGHT | FLAG_SHIFT_VIRTUAL_WAS_ACTIVE);
    ps2_modifiers = 0;
    tenkey_count = 0;
}

void
ps2_send_key_release (const uint8_t usb_keycode) {
    if (!is_ps2_scanning_enabled) {
        return;
    }

    const uint8_t scancode = ps2_scancode_for_usb_keycode(usb_keycode, ps2_active_scancode_set);

    if (!scancode) {
#if ENABLE_PS2_DEVICE_SET_3 && defined(ENABLE_PS2_DEVICE_SET_3_F24) && ENABLE_PS2_DEVICE_SET_3_F24
        if (usb_keycode >= USB_KEY_F13 && usb_keycode <= USB_KEY_F24) {
            uint8_t f_scancode = ps2_scancode_for_usb_keycode(
                USB_KEY_F1 + (usb_keycode - USB_KEY_F13), ps2_active_scancode_set);
            if (f_scancode) {
                send_2_bytes(PS2_BREAK_PREFIX, f_scancode);
            }
        }
#endif
        return;
    }

    const bool is_extended = is_extended_ps2_key(usb_keycode, ps2_active_scancode_set);

    clear_repeat_for_key(scancode, is_extended);

    if (!is_scancode_set_3_active) {
        switch (usb_keycode) {
        case USB_KEY_PRINT_SCREEN:
            if (ps2_modifiers & BOTH_ALT_BITS) {
                if (is_scancode_set_1_active) {
                    send_byte(SET1_MAKE_TO_BREAK(KEY_ALT_SYSRQ_SET1));
                } else {
                    send_2_bytes(PS2_BREAK_PREFIX, KEY_ALT_SYSRQ_SET2);
                }
                clear_repeat();
                return;
            }
            if (is_scancode_set_1_active) {
                send_ext_key_set1_break(EXTENDED_KEY_PRINT_SCREEN_SET1);
            } else {
                send_ext_key_set2_break(EXTENDED_KEY_PRINT_SCREEN_SET2);
            }
            if (!tenkey_count) {
                virtual_shift_off();
            }
            clear_repeat();
            return;

        case USB_KEY_PAUSE_BREAK:
            if (ps2_modifiers & BOTH_CTRL_BITS) {
                if (is_scancode_set_1_active) {
                    send_ext_key_set1_break(EXTENDED_KEY_CTRL_PAUSE_SET1);
                } else {
                    send_ext_key_set2_break(EXTENDED_KEY_CTRL_PAUSE_SET2);
                }
            }
            return;

        case USB_KEY_KP_DIVIDE:
            if (is_scancode_set_1_active) {
                send_ext_key_set1_break(EXTENDED_KEY_KP_DIVIDE_SET1);
            } else {
                send_ext_key_set2_break(EXTENDED_KEY_KP_DIVIDE_SET2);
            }
            shift_unsuppress();
            return;

        default:
            break;
        }
    }

    switch (ps2_active_scancode_set) {
#if ENABLE_PS2_DEVICE_SET_3
    case 3:
        if (!is_extended && !is_set3_break_enabled_for(scancode)) {
            return;
        }
#endif
        // fallthrough
    case 2:
        if (is_extended) {
            send_ext_key_set2_break(scancode);
        } else {
            send_2_bytes(PS2_BREAK_PREFIX, scancode);
        }
        break;
#if ENABLE_PS2_DEVICE_SET_1
    case 1:
        if (is_extended) {
            send_ext_key_set1_break(scancode);
        } else {
            send_byte(SET1_MAKE_TO_BREAK(scancode));
        }
        break;
#endif
    default:
        break;
    }

    if (!is_scancode_set_3_active && is_tenkey_cluster_key(usb_keycode)) {
        if (tenkey_count) {
            --tenkey_count;
        }
        if (!tenkey_count) {
            virtual_shift_off();
            shift_unsuppress();
            ps2_output_flags &= ~FLAG_SHIFT_VIRTUAL_WAS_ACTIVE;
        }
    }

    if (IS_MODIFIER(usb_keycode)) {
        const uint8_t modifier_bit = MODIFIER_BIT(usb_keycode);
        ps2_modifiers &= ~modifier_bit;
        if (!is_scancode_set_3_active && (modifier_bit & BOTH_SHIFT_BITS)) {
            ps2_output_flags &= ~modifier_bit;
            if (tenkey_count && !(ps2_modifiers & BOTH_SHIFT_BITS)
                && (host_led_state & LED_NUM_LOCK_BIT)
                && !(ps2_output_flags & FLAG_SHIFT_VIRTUAL_WAS_ACTIVE)) {
                virtual_shift_on();
            }
        }
    }
}

static inline uint16_t
decode_repeat_period_ms (const uint8_t rate) {
    const uint8_t a = rate & 0x07;
    const uint8_t b = (rate >> 3) & 0x03;
    return ((uint16_t) (8U + a) * 417U << b) / 100U;
}

static inline uint16_t
decode_repeat_delay_ms (const uint8_t rate) {
    return 250U * ((rate >> 5) + 1);
}

static inline void
process_repeat (void) {
    if (!ps2_repeat_key.scancode) {
        return;
    }

    const uint16_t now = timer_read();
    const uint16_t elapsed = now - ps2_repeat_key.timestamp;

    if (ps2_repeat_key.flags & REPEAT_FLAG_IS_REPEATING) {
        if (elapsed < decode_repeat_period_ms(repeat_rate)) {
            // Repeats after the first: wait repeat period ms between them
            return;
        }
    } else {
        if (elapsed < decode_repeat_delay_ms(repeat_rate)) {
            // First repeat: wait for repeat delay ms
            return;
        }
        ps2_repeat_key.flags |= REPEAT_FLAG_IS_REPEATING;
    }

    ps2_repeat_key.timestamp = now;

    if (ps2_repeat_key.flags & REPEAT_FLAG_KEY_IS_EXTENDED) {
        send_2_bytes(PS2_EXT_PREFIX, ps2_repeat_key.scancode);
    } else {
        send_byte(ps2_repeat_key.scancode);
    }
}

void
ps2_press_key (const uint8_t key) {
    if (!is_key_event_queue_full) {
        key_event_queue[key_event_head].key = key;
        key_event_queue[key_event_head].is_release = false;
        increment_key_event_queue();
    }
}

void
ps2_release_key (const uint8_t key) {
    if (!is_key_event_queue_full) {
        key_event_queue[key_event_head].key = key;
        key_event_queue[key_event_head].is_release = true;
        increment_key_event_queue();
    }
}

// MARK: - Special Key Events

static void
handle_special_key_event (const uint8_t event) {
    switch (event) {
    case SPECIAL_EVENT_ALL_KEYS_RELEASED:
        /// This is a marker that the `ps2_output_clear_keys()`
        /// was done at this point in the queue. This means that
        /// all keys are now released.
        virtual_shift_off();
        clear_key_state();
        break;
#ifdef SPECIAL_EVENT_NUM_LOCK_TOGGLED
    case SPECIAL_EVENT_NUM_LOCK_TOGGLED:
        if (tenkey_count && !is_scancode_set_3_active) {
            // Tenkeys held across Num Lock change
            // Note: Real IBM Model M keyboard does not trigger these changes
            // on Num Lock LED change, instead it assumes that the Num Lock
            // key is the only way to toggle the state (and assumes the Num
            // Lock key always does so, even if the OS decides otherwise). So
            // anything done here does not exactly match Model M, but at the
            // same time I think it _might_ be more correct technically in a
            // post-DOS world.
            if (host_led_state & LED_NUM_LOCK_BIT) {
                shift_unsuppress();
                //virtual_shift_on();
            } else {
                virtual_shift_off();
                //shift_suppress();
            }
        }
        break;
#endif
    default:
        break;
    }
}

// MARK: - Command Processing

/// Respond with ACK and set the `cmd` as pending further processing
/// (pending_argc becomes 1).
static void
ack_and_set_pending_cmd (const uint8_t cmd) {
    send_byte(PS2_REPLY_ACK);
    (void) ps2_device_flush();
    pending_cmd = cmd;
    pending_argc = 1;
}

/// Clear any pending command.
static void
clear_pending_cmd (void) {
    pending_cmd = 0;
}

/// Process `cmd` from the argument state `argc`. The `argc` will be `0` on
/// the first call, then it's up to the command whether it needs further
/// states. If `argc` is `ALL_ARGS_READ`, this will just try to flush any
/// pending output, but the command will not be processed anymore.
static void
ps2_process_cmd (const uint8_t cmd, const uint8_t argc) {
    if (argc == ALL_ARGS_READ) {
        // The command needs no more input, but we may still have output
        if (ps2_device_flush()) {
            clear_pending_cmd();
        }
        return;
    }

    ps2_set_host_active();

    switch (cmd) {
    case PS2_COMMAND_RESET:
        send_byte(PS2_REPLY_ACK);
        ps2_output_reset(false);
        break;

    case PS2_COMMAND_ENABLE:
        ps2_enable_scanning();
        send_byte(PS2_REPLY_ACK);
        break;

    case PS2_COMMAND_DISABLE:
        ps2_disable_scanning();
        ps2_output_set_defaults();
        send_byte(PS2_REPLY_ACK);
        break;

    case PS2_COMMAND_SET_DEFAULTS:
        ps2_output_set_defaults();
        send_byte(PS2_REPLY_ACK);
        break;

    case PS2_COMMAND_SET_LEDS:
        if (argc == 0) {
            ack_and_set_pending_cmd(cmd);
        } else {
            int led_byte = ps2_device_recv();
            if (led_byte == EOF) {
                return;
            }
            if (led_byte >= PS2_COMMAND_RANGE_START) {
                clear_pending_cmd();
                ps2_process_cmd(led_byte, 0);
                return;
            }
#ifdef SPECIAL_EVENT_NUM_LOCK_TOGGLED
            const uint8_t old_leds = host_led_state;
#endif
            host_led_state = ps2_leds_to_usb(led_byte);
            usb_keyboard_leds = host_led_state;
            pending_argc = ALL_ARGS_READ;
            send_byte(PS2_REPLY_ACK);
#ifdef SPECIAL_EVENT_NUM_LOCK_TOGGLED
            if (tenkey_count
                && (old_leds & LED_NUM_LOCK_BIT) != (host_led_state & LED_NUM_LOCK_BIT)
                && !is_key_event_queue_full
            ) {
                unshift_key_event_queue();
                key_event_queue[key_event_tail].key = KEY_EVENT_SPECIAL;
                key_event_queue[key_event_tail].is_release = SPECIAL_EVENT_NUM_LOCK_TOGGLED;
            }
#endif
        }
        return;

    case PS2_COMMAND_ECHO:
        send_byte(PS2_COMMAND_ECHO);
        break;

    case PS2_COMMAND_SET_SCAN_CODES:
        if (argc == 0) {
            ack_and_set_pending_cmd(cmd);
        } else {
            int requested_set = ps2_device_recv();
            if (requested_set == EOF) {
                return;
            }

            if (requested_set >= PS2_COMMAND_RANGE_START) {
                clear_pending_cmd();
                ps2_process_cmd(requested_set, 0);
                return;
            }

            if (requested_set) {
                ps2_output_clear_keys(false);
            }

            switch (requested_set) {
            case 2:
                ps2_active_scancode_set = 2;
                send_byte(PS2_REPLY_ACK);
                break;
            case 1:
#if ENABLE_PS2_DEVICE_SET_1
                ps2_active_scancode_set = 1;
                send_byte(PS2_REPLY_ACK);
#else
                send_byte(PS2_REPLY_RESEND);
#endif
                break;
            case 3:
#if ENABLE_PS2_DEVICE_SET_3
                ps2_active_scancode_set = 3;
                send_byte(PS2_REPLY_ACK);
#else
                send_byte(PS2_REPLY_RESEND);
#endif
                break;
            case 0:
                send_2_bytes(PS2_REPLY_ACK, ps2_active_scancode_set);
                break;
            default:
                send_byte(PS2_REPLY_RESEND);
                break;
            }

            pending_argc = ALL_ARGS_READ;
        }
        return;

    case PS2_COMMAND_SET_RATE:
        if (argc == 0) {
            ack_and_set_pending_cmd(cmd);
        } else {
            int rate_arg = ps2_device_recv();
            if (rate_arg == EOF) {
                return;
            }
            if (rate_arg >= PS2_COMMAND_RANGE_START) {
                clear_pending_cmd();
                ps2_process_cmd(rate_arg, 0);
                return;
            }
            repeat_rate = rate_arg & 0x7FU;
            send_byte(PS2_REPLY_ACK);
            pending_argc = ALL_ARGS_READ;
        }
        return;

    case PS2_COMMAND_ID:
        send_byte(PS2_REPLY_ACK);
#if PS2_DEVICE_ID != 0
        send_byte((PS2_DEVICE_ID >> 8) & 0xFF);
        send_byte(PS2_DEVICE_ID & 0xFF);
#endif
        break;

    case PS2_COMMAND_SET_ALL_KEYS_TYPEMATIC:
        set3_set_all_break(false);
        set3_set_all_repeat(true);
        clear_repeat();
        send_byte(PS2_REPLY_ACK);
        break;

    case PS2_COMMAND_SET_ALL_KEYS_MAKE_BREAK:
        set3_set_all_break(true);
        set3_set_all_repeat(false);
        clear_repeat();
        send_byte(PS2_REPLY_ACK);
        break;

    case PS2_COMMAND_SET_ALL_KEYS_MAKE:
        set3_set_all_break(false);
        set3_set_all_repeat(false);
        clear_repeat();
        send_byte(PS2_REPLY_ACK);
        break;

    case PS2_COMMAND_SET_ALL_KEYS_NORMAL:
        set3_set_all_break(true);
        set3_set_all_repeat(true);
        send_byte(PS2_REPLY_ACK);
        break;

    case PS2_COMMAND_SET_KEY_TYPEMATIC:
        // fallthrough
    case PS2_COMMAND_SET_KEY_MAKE_BREAK:
        // fallthrough
    case PS2_COMMAND_SET_KEY_MAKE:
#if ENABLE_PS2_DEVICE_SET_3
        if (argc == 0) {
            ps2_disable_scanning();
            clear_repeat();
            ack_and_set_pending_cmd(cmd);
        } else {
            int keycode = ps2_device_recv();
            if (keycode == EOF) {
                // The list must be terminated by a command byte, keep state
                return;
            }
            if (keycode >= PS2_COMMAND_RANGE_START) {
                // End of key list
                ps2_restore_scanning_state();
                clear_pending_cmd();
                ps2_process_cmd(keycode, 0);
                return;
            }
            if (cmd == PS2_COMMAND_SET_KEY_TYPEMATIC) {
                set3_set_break(keycode, 0);
                set3_set_repeat(keycode, 1);
            } else if (cmd == PS2_COMMAND_SET_KEY_MAKE_BREAK) {
                set3_set_break(keycode, 1);
                set3_set_repeat(keycode, 0);
            } else {
                set3_set_break(keycode, 0);
                set3_set_repeat(keycode, 0);
            }
            send_byte(PS2_REPLY_ACK);
            // Stay in the state, host terminates the list with a command
        }
        return;
#else
        send_byte(PS2_REPLY_RESEND);
        break;
#endif

    case PS2_COMMAND_RESEND:
        ps2_device_resend();
        break;

    default:
        send_byte(PS2_REPLY_RESEND);
        break;
    }

    if (!pending_cmd && !ps2_device_flush()) {
        // The command needs no further arguments but has unflushed output,
        // we need to keep handling it until all is flushed
        pending_cmd = cmd;
        pending_argc = ALL_ARGS_READ;
    }
}

/// Read input or process commands.
/// - Returns: `true` if input was processed.
static bool
read_and_process_cmd (void) {
    uint8_t cmd = pending_cmd;
    uint8_t argc = pending_argc;

    if (!cmd || argc == ALL_ARGS_READ) {
        // See if we have input from the host (also need to do this when we
        // are in `ALL_ARGS_READ` state, because otherwise we can get stuck
        // looping unable to flush because of host inhibit, and not receiving)
        int input = ps2_device_recv();
        if (input != EOF) {
            // Need to discard pending output because it would come as the
            // response to the new command
            ps2_device_clear_output();

            cmd = (uint8_t) input;
            argc = 0;
        }
    }

    if (cmd) {
        ps2_process_cmd(cmd, argc);
        return true;
    } else {
        return false;
    }
}

// MARK: - Main Loop Task

void
ps2_output_task (void) {
    if (!(ps2_output_flags & FLAG_OUTPUT_INITIALIZED)) {
        return;
    }

    if (!read_and_process_cmd()) {
        if (reset_pending_since && (uint16_t) (timer_read() - reset_pending_since) >= PS2_RESET_DURATION_MS) {
            usb_keyboard_leds = 0;
            send_byte(PS2_REPLY_TEST_PASSED);
            ps2_enable_scanning();
            reset_pending_since = 0;
        }
    }

    if (!pending_cmd && ps2_device_flush()) {
        // No unsent output or in-progress command

        if (is_key_event_queue_empty) {
            // No queued events, check for repeats
            process_repeat();
            (void) ps2_device_flush();
        } else {
            int_fast8_t sent_events = 0;
            do {
                const uint8_t key = key_event_queue[key_event_tail].key;
                if (key == KEY_EVENT_SPECIAL) {
                    handle_special_key_event(key_event_queue[key_event_tail].is_release);
                    goto remove_key_from_queue;
                } else if (key_event_queue[key_event_tail].is_release) {
                    ps2_send_key_release(key);
                } else {
                    ps2_send_key_press(key);
                }

                if (!ps2_device_flush() && read_and_process_cmd()) {
                    // We were interrupted by a command, leave event in queue
                    break;
                }

                ++sent_events;
            remove_key_from_queue:
                decrement_key_event_queue();
            } while (sent_events < PS2_OUTPUT_MAX_EVENTS_PER_TASK && !is_key_event_queue_empty);
        }
    }
}

// MARK: - External Queries

bool
ps2_output_is_scanning (void) {
    return is_ps2_scanning_enabled;
}

uint8_t
ps2_host_leds (void) {
    return host_led_state;
}

bool
ps2_output_is_active (void) {
    return is_ps2_host_active;
}

bool
ps2_output_is_initialized (void) {
    return ps2_output_flags & FLAG_OUTPUT_INITIALIZED;
}

bool
ps2_output_queue_is_clear (void) {
    return is_key_event_queue_empty;
}

void
ps2_output_clear_keys (const bool should_discard_unsent_keys) {
    clear_repeat();
    if (should_discard_unsent_keys) {
        key_event_queue_head = 0;
        key_event_queue_tail = 0;
        clear_key_state();
    } else {
        // Add sentinel after all existing events so cleanup runs
        // after they have all been processed naturally
        if (is_key_event_queue_full) {
            // Overflow, try to flush
            ps2_output_task();
            if (is_key_event_queue_full) {
                // Still full, drop the oldest event
                decrement_key_event_queue();
            }
        }
        key_event_queue[key_event_head].key = KEY_EVENT_SPECIAL;
        key_event_queue[key_event_head].is_release = SPECIAL_EVENT_ALL_KEYS_RELEASED;
        increment_key_event_queue();
    }
}

void
ps2_output_shutdown (void) {
    ps2_output_reset(false);
    clear_pending_cmd();
    clear_repeat();
    reset_pending_since = 0;
    ps2_output_flags &= ~(FLAG_OUTPUT_INITIALIZED | FLAG_SCANNING_ENABLED | FLAG_HOST_ACTIVE);
}
