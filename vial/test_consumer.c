// Test MEDIA_KEYS_ENDPOINT against the real usbkbd.c implementation.
// Only the USB hardware layer is mocked.
// This file is AI-generated.
//
// Note: the `main()` function is generated to run every function that
// has a name starting with `test`, and it runs `reset()` before each test.
// Any conditional compilation guards must be _inside_ the function body,
// not around the function.

#define MEDIA_KEYS_ENDPOINT 1
#ifndef MEDIA_KEYS_COUNT
#define MEDIA_KEYS_COUNT 22
#endif
#define ENABLE_MEDIA_KEYS          1
#define ENABLE_APPLE_FN_KEY        0
#define APPLE_FN_IS_MODIFIER       0
#define ENABLE_PS2_DEVICE          0
#define ENABLE_HOST_FINGERPRINT    0
#define ENABLE_KEYBOARD_ENDPOINT   1
#define USB_MAX_KEY_ROLLOVER       6
#define SIMULATED_KEYPRESS_TIME_MS 10

#define delay_milliseconds(ms) \
    do { \
    } while (0)
#define reset_watchdog_timer() \
    do { \
    } while (0)

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Stub for the AVR-specific free_bytes in usb_keyboard_type_debug_report
int free_bytes = 0;

// Platform stubs needed by usbkbd.c (functions it declares but doesn't define)
void
keyboard_reset (void) {
}
uint8_t
current_10ms_tick_count (void) {
    return 0;
}
bool
usb_is_suspended (void) {
    return false;
}
uint8_t
usb_is_configured (void) {
    return 1;
}
uint8_t
usb_address (void) {
    return 0;
}
void
jump_to_bootloader (void) {
}

// usb_kbd_type is declared extern in usbkbd.h
static FILE *usb_kbd_type;

// Mock USB hardware layer
bool
usb_keyboard_send_report (void) {
    return true;
}
bool
usb_keyboard_send_consumer (uint16_t usage) {
    return true;
}

// Include the real implementation
#include "../usbkbd.c"

static int tests_run = 0;
static int tests_failed = 0;
static int verbose = 0;

#define CHECK(cond, msg) \
    do { \
        if (!(cond)) { \
            tests_failed++; \
            if (verbose) \
                printf("FAIL: %s\n", msg); \
        } else if (verbose) \
            printf("PASS: %s\n", msg); \
        tests_run++; \
    } while (0)

static void
reset (void) {
    usb_keyboard_updated = false;
    usb_consumer_usage = 0;
    usb_consumer_updated = false;
}
static void
test_consumer_first_key (void) {
    usb_consumer_usage = 0xFFFF;
#if MEDIA_KEYS_COUNT > 8
    press_virtual(USB_KEY_BRIGHTNESS_UP);
    CHECK(usb_consumer_usage == CC_KEY_BRIGHTNESS_UP, "press: brightness up");
    release_virtual(USB_KEY_BRIGHTNESS_UP);
#else
    press_virtual(USB_KEY_VOLUME_UP);
    CHECK(usb_consumer_usage == CC_KEY_MEDIA_VOLUME_UP, "press: volume up");
    release_virtual(USB_KEY_VOLUME_UP);
#endif
    CHECK(usb_consumer_usage == 0, "release: zero");
}

static void
test_consumer_last_key (void) {
    usb_consumer_usage = 0xFFFF;
#if MEDIA_KEYS_COUNT > 8
    press_virtual(USB_KEY_BROWSE_FAVORITES);
    CHECK(usb_consumer_usage == CC_KEY_BROWSE_FAVORITES, "press: browse fav");
    release_virtual(USB_KEY_BROWSE_FAVORITES);
#else
    press_virtual(USB_KEY_REWIND);
    CHECK(usb_consumer_usage == CC_KEY_MEDIA_REWIND, "press: rewind");
    release_virtual(USB_KEY_REWIND);
#endif
    CHECK(usb_consumer_usage == 0, "release: zero");
}

static void
test_consumer_updated_flag (void) {
    press_virtual(USB_KEY_PLAY_PAUSE);
    CHECK(usb_consumer_updated, "updated flag set on press");
    usb_consumer_updated = false;
    release_virtual(USB_KEY_PLAY_PAUSE);
    CHECK(usb_consumer_updated, "updated flag set on release");
}

static void
test_consumer_reset (void) {
    press_virtual(USB_KEY_PLAY_PAUSE);
    CHECK(usb_consumer_updated, "updated flag set on press");
    CHECK(usb_consumer_usage == CC_KEY_MEDIA_PLAY_PAUSE, "press: play/pause");
    usb_consumer_updated = false;
    usb_keyboard_reset();
    CHECK(usb_consumer_updated, "updated flag set on reset");
    CHECK(usb_consumer_usage == 0, "reset: zero");
}

static void
test_consumer_release_all (void) {
    press_virtual(USB_KEY_PLAY_PAUSE);
    CHECK(usb_consumer_updated, "updated flag set on press");
    CHECK(usb_consumer_usage == CC_KEY_MEDIA_PLAY_PAUSE, "press: play/pause");
    usb_consumer_updated = false;
    usb_keyboard_release_all_keys();
    CHECK(usb_consumer_updated, "updated flag set on release all");
    CHECK(usb_consumer_usage == 0, "release all: zero");
}

static void
test_consumer_apple_fn_ignored (void) {
    // Apple Fn (if not a modifier) should NOT go through consumer endpoint
    usb_consumer_usage = 0xFFFF;
    usb_consumer_updated = false;
    press_virtual(USB_KEY_VIRTUAL_APPLE_FN);
    CHECK(usb_consumer_usage == 0xFFFF, "apple fn press: consumer unchanged");
    CHECK(!usb_consumer_updated, "apple fn press: no consumer update");
    release_virtual(USB_KEY_VIRTUAL_APPLE_FN);
    CHECK(usb_consumer_usage == 0xFFFF, "apple fn release: consumer unchanged");
}

static void
test_consumer_regular_key_no_effect (void) {
    // A key outside the consumer range should not affect consumer state
    usb_consumer_usage = 0xFFFF;
    usb_consumer_updated = false;
    press_virtual(USB_KEY_A);
    CHECK(usb_consumer_usage == 0xFFFF, "non-consumer press: unchanged");
    CHECK(!usb_consumer_updated, "non-consumer press: no update");
    release_virtual(USB_KEY_A);
    CHECK(usb_consumer_usage == 0xFFFF, "non-consumer release: unchanged");
}

static void
test_consumer_two_keys_replace (void) {
    // Press first media key
    usb_consumer_usage = 0xFFFF;
    usb_consumer_updated = false;
    press_virtual(USB_KEY_VOLUME_UP);
    CHECK(usb_consumer_usage == CC_KEY_MEDIA_VOLUME_UP, "first: volume up");
    CHECK(usb_consumer_updated, "first: updated");
    // Press second media key — should replace first
    usb_consumer_updated = false;
#if MEDIA_KEYS_COUNT > 8
    press_virtual(USB_KEY_BRIGHTNESS_DOWN);
    CHECK(usb_consumer_usage == CC_KEY_BRIGHTNESS_DOWN, "second: brightness down");
    release_virtual(USB_KEY_BRIGHTNESS_DOWN);
#else
    press_virtual(USB_KEY_REWIND);
    CHECK(usb_consumer_usage == CC_KEY_MEDIA_REWIND, "second: rewind");
    release_virtual(USB_KEY_REWIND);
#endif
    CHECK(usb_consumer_usage == 0, "release: zero");
}

#include "build/consumer_runner.c"
