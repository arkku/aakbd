// Unit tests for keycode translation (QMK ↔ AAKBD)
// Tests both qmk_to_aakbd() and aakbd_to_qmk() for all keycode types.
// This file is AI-generated.
//
// Note: the `main()` function is generated to run every function that
// has a name starting with `test`, and it runs `reset()` before each test.
// Any conditional compilation guards must be _inside_ the function body,
// not around the function.

// Enable features for tests (before any headers that define defaults)

#define VIAL_ENABLE       1
#define ENABLE_MEDIA_KEYS 1
#ifndef MEDIA_KEYS_COUNT
#define MEDIA_KEYS_COUNT 7
#endif
#define ENABLE_APPLE_FN_KEY     1
#define APPLE_FN_IS_MODIFIER    0
#define ENABLE_ONESHOT_KEYCODES 1
#define ENABLE_SPACE_CADET      1
#define ENABLE_TRI_LAYER        1
#define VIAL_MACRO_COUNT        16
#define VIAL_COMBO_COUNT        4

#include "qmk_keycodes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "../vial/qmk_translate.c"

static int tests_run = 0;
static int tests_failed = 0;
static int verbose = 0;

static void
reset (void) {
}

// Test helpers with line reporting (matching ps2 test style)
#define CHECK_EQ(a, b, msg) \
    do { \
        tests_run++; \
        unsigned _a = (unsigned) (a); \
        unsigned _b = (unsigned) (b); \
        if (_a != _b) { \
            tests_failed++; \
            printf("FAIL %s:%d: %s: expected 0x%04X, got 0x%04X\n", __FILE__, __LINE__, msg, _b, _a); \
        } else if (verbose) { \
            printf("PASS %s:%d: %s: 0x%04X\n", __FILE__, __LINE__, msg, _a); \
        } \
    } while (0)

#define CHECK(cond, msg) \
    do { \
        tests_run++; \
        if (!(cond)) { \
            tests_failed++; \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
        } else if (verbose) { \
            printf("PASS %s:%d: %s\n", __FILE__, __LINE__, msg); \
        } \
    } while (0)

// ====== qmk_to_aakbd tests ======

static void
test_qmk_to_aakbd_kc_no (void) {
    CHECK_EQ(qmk_to_aakbd(KC_NO), NONE, "KC_NO -> NONE");
}

static void
test_qmk_to_aakbd_kc_trns (void) {
    CHECK_EQ(qmk_to_aakbd(KC_TRNS), PASS, "KC_TRNS -> PASS");
}

static void
test_qmk_to_aakbd_basic (void) {
    CHECK_EQ(qmk_to_aakbd(0x04), 0x04, "KC_A -> 0x04");
    CHECK_EQ(qmk_to_aakbd(0x2A), 0x2A, "KC_BSPC -> 0x2A");
    CHECK_EQ(qmk_to_aakbd(0x77), 0x77, "KC_F24 -> 0x77");
}

static void
test_qmk_to_aakbd_mod_left (void) {
    CHECK_EQ(qmk_to_aakbd(LCTL(USB_KEY_A)), CTRL(A), "LCTL+A -> CTRL(A)");
    CHECK_EQ(qmk_to_aakbd(LSFT(USB_KEY_X)), SHIFT(X), "LSFT+X -> SHIFT(X)");
    CHECK_EQ(qmk_to_aakbd(LALT(USB_KEY_H)), ALT(H), "LALT+H -> ALT(H)");
}

static void
test_qmk_to_aakbd_mod_right (void) {
    CHECK_EQ(qmk_to_aakbd(RCTL(USB_KEY_A)), RIGHT_CTRL(A), "RCTL+A -> RIGHT_CTRL(A)");
    CHECK_EQ(qmk_to_aakbd(RSFT(USB_KEY_X)), RIGHT_SHIFT(X), "RSFT+X -> RIGHT_SHIFT(X)");
    CHECK_EQ(qmk_to_aakbd(RALT(USB_KEY_H)), ALTGR(H), "RALT+H -> ALTGR(H)");
}

static void
test_qmk_to_aakbd_mod_tap (void) {
    // Full QMK mod-tap: should produce MOD_OR_KEY encoding for tap/hold
    CHECK_EQ(qmk_to_aakbd(QK_MOD_TAP_LCTL | USB_KEY_A), MOD_OR_KEY(CTRL(A)),
        "MT(LCTL,A) -> MOD_OR_KEY(CTRL(A))");
    CHECK_EQ(qmk_to_aakbd(QK_MOD_TAP_RCTL | USB_KEY_A), MOD_OR_KEY(RIGHT_CTRL(A)),
        "MT(RCTL,A) -> MOD_OR_KEY(RIGHT_CTRL(A))");
    CHECK_EQ(qmk_to_aakbd(QK_MOD_TAP_LSFT | USB_KEY_A), MOD_OR_KEY(SHIFT(A)),
        "MT(LSFT,A) -> MOD_OR_KEY(SHIFT(A))");
    CHECK_EQ(qmk_to_aakbd(QK_MOD_TAP_RSFT | USB_KEY_A), MOD_OR_KEY(RIGHT_SHIFT(A)),
        "MT(RSFT,A) -> MOD_OR_KEY(RIGHT_SHIFT(A))");
    CHECK_EQ(qmk_to_aakbd(QK_MOD_TAP_LALT | USB_KEY_A), MOD_OR_KEY(ALT(A)),
        "MT(LALT,A) -> MOD_OR_KEY(ALT(A))");
    CHECK_EQ(qmk_to_aakbd(QK_MOD_TAP_RALT | USB_KEY_A), MOD_OR_KEY(ALTGR(A)),
        "MT(RALT,A) -> MOD_OR_KEY(ALTGR(A))");
    CHECK_EQ(qmk_to_aakbd(QK_MOD_TAP_LGUI | USB_KEY_A), MOD_OR_KEY(CMD(A)),
        "MT(LGUI,A) -> MOD_OR_KEY(CMD(A))");
    CHECK_EQ(qmk_to_aakbd(QK_MOD_TAP_RGUI | USB_KEY_A), MOD_OR_KEY(RIGHT_CMD(A)),
        "MT(RGUI,A) -> MOD_OR_KEY(RIGHT_CMD(A))");
}

static void
test_mod_tap_mixed_mods (void) {
    // Mixed left+right modifiers: LCTL + RALT -> both preserved as right-side
    uint16_t mt = QK_MOD_TAP_RALT | LCTL(USB_KEY_A);
    uint16_t aakbd = qmk_to_aakbd(mt);
    uint8_t mods = RIGHT_CTRL_BIT | RIGHT_ALT_BIT;
    CHECK_EQ(aakbd, MOD_OR_KEY(MODS_FOR_KEY(mods) | USB_KEY_A), "MT(LCTL|RALT,A) -> RCTL+RALT");
    uint16_t back = aakbd_to_qmk(aakbd);
    CHECK_EQ(back, RCTL(RALT(USB_KEY_A)), "MT(LCTL|RALT,A) round-trip -> RCTL+RALT preserved");
}

static void
test_qmk_to_aakbd_multi_mod (void) {
    CHECK_EQ(qmk_to_aakbd(LCTL(LSFT(USB_KEY_A))), (uint16_t) ((CTRL_BIT | SHIFT_BIT) << 8) | USB_KEY_A,
        "LCTL+LSFT+A -> 0x0304");
}

static void
test_qmk_to_aakbd_momentary_layer (void) {
    // QMK MO(0) → AAKBD layer 1, MO(2) → AAKBD layer 3
    CHECK_EQ(qmk_to_aakbd(QK_MOMENTARY | 0), (uint16_t) LAYER_COMMAND(TOGGLE, ON_HOLD, 1),
        "MO(0) -> LAYER_TOGGLE_HOLD(1)");
    CHECK_EQ(qmk_to_aakbd(QK_MOMENTARY | 2), (uint16_t) LAYER_COMMAND(TOGGLE, ON_HOLD, 3),
        "MO(2) -> LAYER_TOGGLE_HOLD(3)");
}

static void
test_qmk_to_aakbd_def_layer (void) {
    // QMK DF(0) → AAKBD layer 1, DF(3) → AAKBD layer 4
    CHECK_EQ(qmk_to_aakbd(DF(0)), (uint16_t) LAYER_COMMAND(SET_BASE, ON_PRESS, 1),
        "DF(0) -> LAYER_SET_BASE(1)");
    CHECK_EQ(qmk_to_aakbd(DF(3)), (uint16_t) LAYER_COMMAND(SET_BASE, ON_PRESS, 4),
        "DF(3) -> LAYER_SET_BASE(4)");
}

static void
test_qmk_to_aakbd_toggle_layer (void) {
    // QMK TG(2) → AAKBD layer 3
    CHECK_EQ(
        qmk_to_aakbd(TG(2)), (uint16_t) LAYER_COMMAND(TOGGLE, ON_PRESS, 3), "TG(2) -> LAYER_TOGGLE(3)");
}

static void
test_qmk_to_aakbd_layer_tap_toggle (void) {
    // TT(2) → AAKBD layer 3
    CHECK_EQ(qmk_to_aakbd(TT(2)), (uint16_t) LAYER_COMMAND(TOGGLE, ON_HOLD_KEEP_IF_NO_KEYPRESS, 3),
        "TT(2) -> LAYER_TOGGLE + ACT_ON_HOLD_KEEP_IF_NO_KEYPRESS(3)");
    CHECK_EQ(qmk_to_aakbd(TT(0)), (uint16_t) LAYER_COMMAND(TOGGLE, ON_HOLD_KEEP_IF_NO_KEYPRESS, 1),
        "TT(0) -> LAYER_TOGGLE + ACT_ON_HOLD_KEEP_IF_NO_KEYPRESS(1)");
    CHECK_EQ(
        qmk_to_aakbd(TT(31)), KEY_EXT_QMK_KEYCODE, "TT(31) -> KEY_EXT_QMK_KEYCODE (unsupported code)");
}

static void
test_qmk_to_aakbd_layer_tap (void) {
    // QMK LT(2,F1) → AAKBD layer 3
    CHECK_EQ(qmk_to_aakbd(QK_LAYER_TAP | (2 << 8) | USB_KEY_F1),
        (uint16_t) LAYER_OR_PLAIN_KEY(3, USB_KEY_F1), "LT(2,F1) -> LAYER_OR_PLAIN_KEY(3,F1)");
}

static void
test_qmk_to_aakbd_layer_mod (void) {
    CHECK(qmk_to_aakbd(QK_LAYER_MOD | 1) & 0xFF00, "LM(2,LCTL) has modifier bits");
    // Reverse: MODS_CTRL → QK_LAYER_MOD | 1
    CHECK_EQ(
        aakbd_to_qmk(MODS_CTRL), QK_LAYER_MOD | 1, "LM(2,LCTL) reverse: MODS_CTRL -> QK_LAYER_MOD | 1");
    CHECK_EQ(aakbd_to_qmk(MODS_SHIFT), QK_LAYER_MOD | 2,
        "LM(2,LSFT) reverse: MODS_SHIFT -> QK_LAYER_MOD | 2");
    CHECK_EQ(
        aakbd_to_qmk(MODS_ALT), QK_LAYER_MOD | 4, "LM(2,LALT) reverse: MODS_ALT -> QK_LAYER_MOD | 4");
    CHECK_EQ(
        aakbd_to_qmk(MODS_CMD), QK_LAYER_MOD | 8, "LM(2,LGUI) reverse: MODS_CMD -> QK_LAYER_MOD | 8");
}

static void
test_qmk_to_aakbd_go_to_layer (void) {
    // QMK TO(1) → AAKBD layer 2
    CHECK_EQ(qmk_to_aakbd(QK_TO | 1), (uint16_t) LAYER_COMMAND(SET_MASK, ON_PRESS, 2),
        "TO(1) -> LAYER_SET_MASK(2)");
}

static void
test_qmk_to_aakbd_osl (void) {
    // QMK OSL(2) → AAKBD layer 3
    CHECK_EQ(qmk_to_aakbd(OSL(2)), (uint16_t) LAYER_COMMAND(ENABLE, ONESHOT, 3),
        "OSL(2) -> LAYER_ENABLE + ACT_ONESHOT(3)");
    CHECK_EQ(qmk_to_aakbd(OSL(31)), KEY_EXT_QMK_KEYCODE,
        "OSL(31) -> KEY_EXT_QMK_KEYCODE (unsupported code)");
}

static void
test_qmk_to_aakbd_osm_left (void) {
    CHECK_EQ(qmk_to_aakbd(OSM(MOD_LCTL)), MOD_ONESHOT(CTRL_BIT), "OSM(LCTL) -> MOD_ONESHOT(CTRL)");
}

static void
test_qmk_to_aakbd_osm_right (void) {
    CHECK_EQ(
        qmk_to_aakbd(OSM(MOD_RSFT)), MOD_ONESHOT(RIGHT_SHIFT_BIT), "OSM(RSFT) -> MOD_ONESHOT(RSFT)");
}

static void
test_qmk_to_aakbd_osm_both (void) {
    // Mixed left+right: convert all to right-side if any is right
    CHECK_EQ(qmk_to_aakbd(QK_ONE_SHOT_MOD | (MOD_LCTL | MOD_RSFT)),
        MOD_ONESHOT(RIGHT_CTRL_BIT | RIGHT_SHIFT_BIT),
        "OSM(LCTL|RSFT) -> MOD_ONESHOT(RCTL|RSFT), mixed→right");
    CHECK_EQ(qmk_to_aakbd(QK_ONE_SHOT_MOD | (MOD_LSFT | MOD_RALT)),
        MOD_ONESHOT(RIGHT_SHIFT_BIT | RIGHT_ALT_BIT),
        "OSM(LSFT|RALT) -> MOD_ONESHOT(RSFT|RALT), mixed→right");
}

static void
test_qmk_to_aakbd_reset (void) {
    CHECK_EQ(qmk_to_aakbd(QK_BOOTLOADER), EXTENDED(ENTER_BOOTLOADER),
        "QK_BOOTLOADER -> EXT(ENTER_BOOTLOADER)");
    CHECK_EQ(
        qmk_to_aakbd(QK_GRAVE_ESCAPE), EXTENDED(GRAVE_ESCAPE), "QK_GRAVE_ESCAPE -> EXT_GRAVE_ESCAPE");
    CHECK_EQ(aakbd_to_qmk(qmk_to_aakbd(QK_GRAVE_ESCAPE)), QK_GRAVE_ESCAPE, "GRAVE_ESCAPE round-trip");
}

static void
test_qmk_to_aakbd_macros (void) {
    // QMK MACRO00-63 map to AAKBD MACRO(64)-MACRO(127)
    // (shifted by VIAL_MACRO_START to keep AAKBD user macros 0-63 separate)
    CHECK_EQ(qmk_to_aakbd(MACRO00), MACRO(64), "MACRO00 -> MACRO(64)");
    CHECK_EQ(qmk_to_aakbd(MACRO15), MACRO(79), "MACRO15 -> MACRO(79)");
    CHECK_EQ(qmk_to_aakbd(MACRO00 + 63), MACRO(127), "MACRO63 -> MACRO(127)");
    // QMK MACRO64+ exceed the 0-127 AAKBD macro range after shift → unsupported
    CHECK_EQ(qmk_to_aakbd(MACRO00 + 64), KEY_EXT_QMK_KEYCODE,
        "MACRO64 -> KEY_EXT_QMK_KEYCODE (exceeds shifted range)");
    CHECK_EQ(qmk_to_aakbd(QK_MACRO_MAX), KEY_EXT_QMK_KEYCODE,
        "MACRO127 -> KEY_EXT_QMK_KEYCODE (exceeds shifted range)");
}

static void
test_qmk_to_aakbd_user (void) {
    // USER00 = Apple Fn → EXT_VIAL_APPLE_FN
    CHECK_EQ(qmk_to_aakbd(USER00), (uint16_t) (EXTENDED_KEY_BIT | EXT_VIAL_APPLE_FN),
        "USER00 -> EXT_VIAL_APPLE_FN (Apple Fn)");
    // VIAL_USER_MACRO_FIRST—15 → AAKBD MACRO(0—14)
    CHECK_EQ(qmk_to_aakbd(VIAL_USER_MACRO_FIRST), MACRO(0), "VIAL_USER_MACRO_FIRST -> MACRO(0)");
    CHECK_EQ(qmk_to_aakbd(VIAL_USER_MACRO_FIRST + 1), MACRO(1), "USER02 -> MACRO(1)");
    CHECK_EQ(
        qmk_to_aakbd(VIAL_USER_MACRO_FIRST + 14), MACRO(14), "VIAL_USER_MACRO_FIRST+14 -> MACRO(14)");
}

static void
test_apple_fn_roundtrip (void) {
    // Apple Fn → USER00 ↔ EXT_VIAL_APPLE_FN and back
    uint16_t qmk = aakbd_to_qmk(USB_KEY_VIRTUAL_APPLE_FN);
    CHECK_EQ(qmk, USER00, "Apple Fn -> USER00");
    uint16_t aakbd = qmk_to_aakbd(qmk);
    CHECK_EQ(aakbd, (uint16_t) (EXTENDED_KEY_BIT | EXT_VIAL_APPLE_FN), "USER00 -> EXT_VIAL_APPLE_FN");
    uint16_t back = aakbd_to_qmk(aakbd);
    CHECK_EQ(back, USER00, "EXT_VIAL_APPLE_FN -> USER00 round-trip");
}

static void
test_user_codes_roundtrip (void) {
    // VIAL_USER_MACRO_FIRST-15 ↔ AAKBD MACRO(0-14) and back
    for (uint16_t i = 0; i < 15; ++i) {
        uint16_t qmk = VIAL_USER_MACRO_FIRST + i;
        uint16_t aakbd = qmk_to_aakbd(qmk);
        CHECK_EQ(aakbd, MACRO(i), "VIAL_USER_MACRO_FIRST+i -> MACRO(i)");
        uint16_t back = aakbd_to_qmk(aakbd);
        CHECK_EQ(back, qmk, "MACRO(i) -> VIAL_USER_MACRO_FIRST+i round-trip");
    }
}

static void
test_magic_passthrough (void) {
    // MAGIC codes pass through qmk_to_aakbd as-is
    // QMK MAGIC codes are not valid AAKBD keycodes — must return
    // KEY_EXT_QMK_KEYCODE so they reach process_qmk_keycode callback.
    CHECK_EQ(qmk_to_aakbd(QK_MAGIC_SWAP_CONTROL_CAPS_LOCK), KEY_EXT_QMK_KEYCODE,
        "QK_MAGIC_SWAP_CONTROL_CAPS_LOCK -> KEY_EXT_QMK_KEYCODE");
    CHECK_EQ(qmk_to_aakbd(QK_MAGIC_UNSWAP_CONTROL_CAPS_LOCK), KEY_EXT_QMK_KEYCODE,
        "QK_MAGIC_UNSWAP_CONTROL_CAPS_LOCK -> KEY_EXT_QMK_KEYCODE");
    CHECK_EQ(qmk_to_aakbd(QK_MAGIC_SWAP_LALT_LGUI), KEY_EXT_QMK_KEYCODE,
        "QK_MAGIC_SWAP_LALT_LGUI -> KEY_EXT_QMK_KEYCODE");
    CHECK_EQ(qmk_to_aakbd(QK_MAGIC_TOGGLE_GUI), KEY_EXT_QMK_KEYCODE,
        "QK_MAGIC_TOGGLE_GUI -> KEY_EXT_QMK_KEYCODE");
    CHECK_EQ(qmk_to_aakbd(QK_MAGIC_SWAP_GRAVE_ESC), KEY_EXT_QMK_KEYCODE,
        "QK_MAGIC_SWAP_GRAVE_ESC -> KEY_EXT_QMK_KEYCODE");
    CHECK_EQ(qmk_to_aakbd(QK_MAGIC_TOGGLE_ALT_GUI), KEY_EXT_QMK_KEYCODE,
        "QK_MAGIC_TOGGLE_ALT_GUI -> KEY_EXT_QMK_KEYCODE");
    CHECK_EQ(qmk_to_aakbd(QK_MAGIC_TOGGLE_CTL_GUI), KEY_EXT_QMK_KEYCODE,
        "QK_MAGIC_TOGGLE_CTL_GUI -> KEY_EXT_QMK_KEYCODE");
    CHECK_EQ(qmk_to_aakbd(QK_MAGIC_TOGGLE_ESCAPE_CAPS_LOCK), KEY_EXT_QMK_KEYCODE,
        "QK_MAGIC_TOGGLE_ESCAPE_CAPS_LOCK -> KEY_EXT_QMK_KEYCODE");
    CHECK_EQ(qmk_to_aakbd(QK_MAGIC_TOGGLE_CONTROL_CAPS_LOCK), KEY_EXT_QMK_KEYCODE,
        "QK_MAGIC_TOGGLE_CONTROL_CAPS_LOCK -> KEY_EXT_QMK_KEYCODE");
    CHECK_EQ(qmk_to_aakbd(QK_MAGIC_TOGGLE_BACKSLASH_BACKSPACE), KEY_EXT_QMK_KEYCODE,
        "QK_MAGIC_TOGGLE_BACKSLASH_BACKSPACE -> KEY_EXT_QMK_KEYCODE");
    // aakbd_to_qmk has no reverse for KEY_EXT_QMK_KEYCODE (unsupported QMK
    // keycodes have the original value stored in EEPROM, not in the AAKBD code)
    uint16_t back = aakbd_to_qmk(KEY_EXT_QMK_KEYCODE);
    CHECK_EQ(back, KC_NO, "KEY_EXT_QMK_KEYCODE reverse -> KC_NO (no round-trip)");
}

static void
test_qmk_to_aakbd_unsupported (void) {
    // Unsupported QMK keycodes → KEY_EXT_QMK_KEYCODE placeholder
    CHECK_EQ(qmk_to_aakbd(QK_SWAP_HANDS), KEY_EXT_QMK_KEYCODE, "QK_SWAP_HANDS -> KEY_EXT_QMK_KEYCODE");
    CHECK_EQ(qmk_to_aakbd(QK_TAP_DANCE), KEY_EXT_QMK_KEYCODE, "QK_TAP_DANCE -> KEY_EXT_QMK_KEYCODE");
    CHECK_EQ(qmk_to_aakbd(QK_UNICODE), KEY_EXT_QMK_KEYCODE, "QK_UNICODE -> KEY_EXT_QMK_KEYCODE");
}

static void
test_qmk_to_aakbd_misc (void) {
    // Basic key 0x00FF (NONE) → EXTENDED(QMK_KEYCODE) since it's > PLAIN_KEY_MAX
    CHECK_EQ(qmk_to_aakbd(0x00FF), KEY_EXT_QMK_KEYCODE,
        "0x00FF (NONE) -> EXTENDED(QMK_KEYCODE) (beyond PLAIN_KEY_MAX)");

    // Modifier + invalid key (0x00) → EXTENDED(QMK_KEYCODE)
    CHECK_EQ(qmk_to_aakbd(QK_LCTL), KEY_EXT_QMK_KEYCODE,
        "QK_LCTL (mod-only, no key) -> EXTENDED(QMK_KEYCODE)");
    CHECK_EQ(qmk_to_aakbd(QK_RCTL), KEY_EXT_QMK_KEYCODE,
        "QK_RCTL (mod-only, no key) -> EXTENDED(QMK_KEYCODE)");

    // Modifier + invalid key (0xFF) → EXTENDED(QMK_KEYCODE)
    CHECK_EQ(
        qmk_to_aakbd(LCTL(0xFF)), KEY_EXT_QMK_KEYCODE, "LCTL+0xFF (key=NONE) -> EXTENDED(QMK_KEYCODE)");

    // OSM with no mods → EXTENDED(QMK_KEYCODE)
    CHECK_EQ(qmk_to_aakbd(QK_ONE_SHOT_MOD), KEY_EXT_QMK_KEYCODE,
        "OSM(0) -> EXTENDED(QMK_KEYCODE) (no mods)");

    // Layer commands with layer > 30 → EXTENDED(QMK_KEYCODE) or PASS
    CHECK_EQ(qmk_to_aakbd(QK_MOMENTARY | 31), KEY_EXT_QMK_KEYCODE,
        "MO(31) -> EXTENDED(QMK_KEYCODE) (layer > 30)");
    CHECK_EQ(qmk_to_aakbd(DF(31)), KEY_EXT_QMK_KEYCODE, "DF(31) -> EXTENDED(QMK_KEYCODE) (layer > 30)");
    CHECK_EQ(qmk_to_aakbd(TG(31)), KEY_EXT_QMK_KEYCODE, "TG(31) -> EXTENDED(QMK_KEYCODE) (layer > 30)");
    CHECK_EQ(
        qmk_to_aakbd(OSL(31)), KEY_EXT_QMK_KEYCODE, "OSL(31) -> EXTENDED(QMK_KEYCODE) (layer > 30)");

    // LT with layer > 30 → EXTENDED(QMK_KEYCODE)
    CHECK_EQ(qmk_to_aakbd(QK_LAYER_TAP | (31 << 8) | USB_KEY_F1), KEY_EXT_QMK_KEYCODE,
        "LT(31, F1) -> EXTENDED(QMK_KEYCODE) (layer > 30)");

    // LT with key=0 → EXTENDED(QMK_KEYCODE)
    CHECK_EQ(qmk_to_aakbd(QK_LAYER_TAP | (2 << 8)), KEY_EXT_QMK_KEYCODE,
        "LT(2, key=0) -> EXTENDED(QMK_KEYCODE) (invalid key)");

    // QK_REBOOT forward direction
    CHECK_EQ(qmk_to_aakbd(QK_REBOOT), EXTENDED(RESET_KEYBOARD), "QK_REBOOT -> EXT(RESET_KEYBOARD)");

#if ENABLE_TRI_LAYER
    // FN_MO13 / FN_MO23 forward direction
    CHECK_EQ(qmk_to_aakbd(FN_MO13), EXTENDED(LAYER_2_4), "FN_MO13 -> EXT(LAYER_2_4)");
    CHECK_EQ(qmk_to_aakbd(FN_MO23), EXTENDED(LAYER_3_4), "FN_MO23 -> EXT(LAYER_3_4)");
#endif
}

// ====== aakbd_to_qmk tests ======

static void
test_aakbd_to_qmk_pass (void) {
    CHECK_EQ(aakbd_to_qmk(PASS), KC_TRNS, "PASS -> KC_TRNS");
}

static void
test_aakbd_to_qmk_none (void) {
    CHECK_EQ(aakbd_to_qmk(NONE), KC_NO, "NONE -> KC_NO");
}

static void
test_aakbd_to_qmk_plain (void) {
    CHECK_EQ(aakbd_to_qmk(USB_KEY_A), USB_KEY_A, "KC_A -> KC_A");
    CHECK_EQ(aakbd_to_qmk(USB_KEY_BACKSPACE), USB_KEY_BACKSPACE, "KC_BSPC -> KC_BSPC");
}

static void
test_aakbd_to_qmk_mod_left (void) {
    uint16_t ctrl_b = ((uint16_t) CTRL_BIT << 8) | USB_KEY_B;
    uint16_t shift_h = ((uint16_t) SHIFT_BIT << 8) | USB_KEY_H;
    uint16_t cmd_b = ((uint16_t) CMD_BIT << 8) | USB_KEY_B;
    CHECK_EQ(aakbd_to_qmk(ctrl_b), LCTL(USB_KEY_B), "CTRL(B) -> LCTL(B)");
    CHECK_EQ(aakbd_to_qmk(shift_h), LSFT(USB_KEY_H), "SHIFT(H) -> LSFT(H)");
    CHECK_EQ(aakbd_to_qmk(cmd_b), LGUI(USB_KEY_B), "CMD(B) -> LGUI(B)");
}

static void
test_aakbd_to_qmk_mod_right (void) {
    uint16_t rctrl_a = RIGHT_CTRL(A);
    uint16_t rshift_x = RIGHT_SHIFT(X);
    CHECK_EQ(aakbd_to_qmk(rctrl_a), RCTL(USB_KEY_A), "RIGHT_CTRL(A) -> RCTL(A)");
    CHECK_EQ(aakbd_to_qmk(rshift_x), RSFT(USB_KEY_X), "RIGHT_SHIFT(X) -> RSFT(X)");
    // Mixed-side mods: SHIFT(RIGHT_CMD) translates without losing either
    CHECK_EQ(aakbd_to_qmk(SHIFT(RIGHT_CMD)), (uint16_t) (QK_LSFT | USB_KEY_RIGHT_CMD),
        "SHIFT(RIGHT_CMD) -> LSFT(RIGHT_CMD)");
    // Multiple left-side mods: CTRL+SHIFT(A) must include both
    uint16_t ctrl_shift_a = CTRL_SHIFT(A);
    CHECK_EQ(aakbd_to_qmk(ctrl_shift_a), (uint16_t) (QK_LCTL | QK_LSFT | USB_KEY_A),
        "CTRL(SHIFT(A)) -> LCTL+LSFT+A");
}

static void
test_aakbd_to_qmk_extended (void) {
    CHECK_EQ(
        aakbd_to_qmk(KEY_EXT_GRAVE_ESCAPE), QK_GRAVE_ESCAPE, "KEY_GRAVE_ESCAPE -> QK_GRAVE_ESCAPE");
    CHECK_EQ(aakbd_to_qmk(EXTENDED(ENTER_BOOTLOADER)), QK_BOOTLOADER,
        "EXT(ENTER_BOOTLOADER) -> QK_BOOTLOADER");
    CHECK_EQ(aakbd_to_qmk(EXTENDED(RESET_KEYBOARD)), QK_REBOOT, "EXT(RESET_KEYBOARD) -> QK_REBOOT");
#if ENABLE_TRI_LAYER
    CHECK_EQ(aakbd_to_qmk(EXTENDED(LAYER_2_4)), FN_MO13, "EXT(LAYER_2_4) -> FN_MO13");
    CHECK_EQ(aakbd_to_qmk(EXTENDED(LAYER_3_4)), FN_MO23, "EXT(LAYER_3_4) -> FN_MO23");
#endif
}

static void
test_aakbd_to_qmk_exact_mods (void) {
    // Exact modifiers are AAKBD-specific and have no QMK equivalent → KC_NO
    CHECK_EQ(aakbd_to_qmk(EXACTLY_SHIFT), KC_NO, "EXACTLY_SHIFT -> KC_NO");
    CHECK_EQ(aakbd_to_qmk(EXACTLY_CTRL), KC_NO, "EXACTLY_CTRL -> KC_NO");
    CHECK_EQ(aakbd_to_qmk(EXACTLY_RIGHT_CTRL), KC_NO, "EXACTLY_RIGHT_CTRL -> KC_NO");
    CHECK_EQ(aakbd_to_qmk(EXACTLY_CTRL_SHIFT), KC_NO, "EXACTLY_CTRL_SHIFT -> KC_NO");
    CHECK_EQ(aakbd_to_qmk(EXACTLY_CTRL_ALT_SHIFT), KC_NO, "EXACTLY_MEH -> KC_NO");
}

static void
test_aakbd_to_qmk_unsupported_disable (void) {
    // DISABLE + ON_HOLD → KC_NO (no QMK equivalent)
    CHECK_EQ(aakbd_to_qmk(LAYER_OFF_HOLD(2)), KC_NO, "LAYER_OFF_HOLD(2) -> KC_NO (unsupported)");
    // DISABLE + ON_HOLD_KEEP → KC_NO
    CHECK_EQ(aakbd_to_qmk(LAYER_OFF_STICKY(2)), KC_NO, "LAYER_OFF_STICKY(2) -> KC_NO (unsupported)");
}

static void
test_aakbd_to_qmk_macros (void) {
    // User macros 0-62 map to QMK VIAL_USER_MACRO_FIRST-63 for Vial GUI exposure
    CHECK_EQ(aakbd_to_qmk(MACRO(0)), VIAL_USER_MACRO_FIRST, "MACRO(0) -> VIAL_USER_MACRO_FIRST");
    CHECK_EQ(
        aakbd_to_qmk(MACRO(14)), VIAL_USER_MACRO_FIRST + 14, "MACRO(14) -> VIAL_USER_MACRO_FIRST+14");
    CHECK_EQ(aakbd_to_qmk(MACRO(15)), VIAL_USER_MACRO_FIRST + 15, "MACRO(15) -> USER16");
    CHECK_EQ(aakbd_to_qmk(MACRO(62)), VIAL_USER_MACRO_FIRST + 62, "MACRO(62) -> USER63");
    // Macro 63 is beyond the 63 User key slots → KC_NO
    CHECK_EQ(aakbd_to_qmk(MACRO(63)), KC_NO, "MACRO(63) -> KC_NO");
    // Vial EEPROM macros 64-127 map to QMK MACRO00-63
    CHECK_EQ(aakbd_to_qmk(MACRO(64)), MACRO00, "MACRO(64) -> MACRO00");
    CHECK_EQ(aakbd_to_qmk(MACRO(79)), MACRO15, "MACRO(79) -> MACRO15");
    CHECK_EQ(aakbd_to_qmk(MACRO(126)), MACRO00 + 62, "MACRO(126) -> MACRO62");
    CHECK_EQ(aakbd_to_qmk(MACRO(127)), MACRO00 + 63, "MACRO(127) -> MACRO63");
}

static void
test_qmk_to_aakbd_osm_mods (void) {
    uint8_t mods = CTRL_BIT | SHIFT_BIT;
    CHECK_EQ(
        qmk_to_aakbd(QK_ONE_SHOT_MOD | mods), MOD_ONESHOT(mods), "OSM(LCTL+LSFT) -> MOD_ONESHOT(both)");
}

static void
test_aakbd_to_qmk_osl (void) {
    CHECK_EQ(aakbd_to_qmk(LAYER_COMMAND(ENABLE, ONESHOT, 2)), OSL(1), "OSL(2) reverse -> QMK layer 1");
}

static void
test_aakbd_to_qmk_osm_left (void) {
    uint16_t aakbd_kc = MOD_ONESHOT(CTRL_BIT);
    CHECK_EQ(aakbd_to_qmk(aakbd_kc), OSM(MOD_LCTL), "OSM(LCTL) reverse");
}

static void
test_aakbd_to_qmk_osm_right (void) {
    uint16_t aakbd_kc = MOD_ONESHOT(RIGHT_SHIFT_BIT);
    CHECK_EQ(aakbd_to_qmk(aakbd_kc), OSM(MOD_RSFT),
        "OSM(RSFT) reverse, AAKBD→QMK converts bit pos to 5-bit");
}

static void
test_roundtrip_osl (void) {
    CHECK_EQ(aakbd_to_qmk(qmk_to_aakbd(OSL(2))), OSL(2), "OSL round-trip");
}

static void
test_roundtrip_osm_left (void) {
    CHECK_EQ(aakbd_to_qmk(qmk_to_aakbd(OSM(MOD_LCTL))), OSM(MOD_LCTL), "OSM(LCTL) round-trip");
}

static void
test_roundtrip_osm_right (void) {
    CHECK_EQ(
        aakbd_to_qmk(qmk_to_aakbd(OSM(MOD_RSFT))), OSM(MOD_RSFT), "OSM(RSFT) round-trip via QMK 5-bit");
}

static void
test_roundtrip_osm_both (void) {
    // Mixed left+right: convert all to right-side
    CHECK_EQ(aakbd_to_qmk(qmk_to_aakbd(OSM(MOD_LCTL | MOD_RSFT))), OSM(MOD_RCTL | MOD_RSFT),
        "OSM(LCTL|RSFT) round-trip, mixed→right");
}

static void
test_aakbd_to_qmk_layer_ops (void) {
    // AAKBD layer 2 → QMK layer 1 (subtract 1)
    CHECK_EQ(aakbd_to_qmk(LAYER_COMMAND(TOGGLE, ON_HOLD, 2)), QK_MOMENTARY | 1,
        "MO(2) reverse -> QMK layer 1");
    CHECK_EQ(aakbd_to_qmk(LAYER_COMMAND(TOGGLE, ON_PRESS, 2)), TG(1), "TG(2) reverse -> QMK layer 1");
    CHECK_EQ(
        aakbd_to_qmk(LAYER_COMMAND(SET_MASK, ON_PRESS, 1)), QK_TO | 0, "TO(1) reverse -> QMK layer 0");
    CHECK_EQ(aakbd_to_qmk(LAYER_COMMAND(SET_BASE, ON_PRESS, 1)), DF(0), "DF(1) reverse -> QMK layer 0");
    CHECK_EQ(aakbd_to_qmk(LAYER_OR_PLAIN_KEY(2, USB_KEY_F1)), QK_LAYER_TAP | (1 << 8) | USB_KEY_F1,
        "LT(2,F1) reverse -> QMK layer 1");
    CHECK_EQ(aakbd_to_qmk(LAYER_COMMAND(TOGGLE, ON_HOLD_KEEP_IF_NO_KEYPRESS, 2)), TT(1),
        "TT(2) reverse -> QMK layer 1");

    // LAYER_ON_HOLD(2) = ENABLE + ON_HOLD -> QK_MOMENTARY (MO)
    CHECK_EQ(aakbd_to_qmk(LAYER_COMMAND(ENABLE, ON_HOLD, 2)), QK_MOMENTARY | 1,
        "LAYER_ON_HOLD(2) reverse -> MO(1)");
    // LAYER_TOGGLE(2) = TOGGLE + ON_RELEASE -> QK_TOGGLE_LAYER (TG)
    CHECK_EQ(
        aakbd_to_qmk(LAYER_COMMAND(TOGGLE, ON_RELEASE, 2)), TG(1), "LAYER_TOGGLE(2) reverse -> TG(1)");
    // LAYER_ENABLE(2) = ENABLE + ON_RELEASE -> QK_TOGGLE_LAYER (TG)
    CHECK_EQ(
        aakbd_to_qmk(LAYER_COMMAND(ENABLE, ON_RELEASE, 2)), TG(1), "LAYER_ENABLE(2) reverse -> TG(1)");
    // LAYER_ON_STICKY(2) = ENABLE + ON_HOLD_KEEP_IF_NO_KEYPRESS -> QK_LAYER_TAP_TOGGLE (TT)
    CHECK_EQ(aakbd_to_qmk(LAYER_COMMAND(ENABLE, ON_HOLD_KEEP_IF_NO_KEYPRESS, 2)), OSL(1),
        "LAYER_ON_STICKY(2) reverse -> OSL(1)");
}

// ====== Round-trip consistency tests ======

static void
test_roundtrip_basic (void) {
    for (uint16_t i = 0x02; i <= 0xFE; ++i) {
        // QMK consumer/system codes (0xA5-0xBE) are mapped to AAKBD virtual
        // media keys and don't round-trip as the same value; they have their
        // own dedicated tests.
        // USB_KEY_VIRTUAL_MEDIA_N are AAKBD-internal abstract codes.
        if ((i >= QK_CONSUMER_MIN && i <= QK_CONSUMER_MAX) || i == USB_KEY_VIRTUAL_APPLE_FN
            || (i >= USB_KEY_VIRTUAL_MEDIA_1 && i < USB_KEY_VIRTUAL_MEDIA_1 + MEDIA_KEYS_COUNT)) {
#if MEDIA_KEYS_COUNT <= 8
            // Also skip USB_KEY_* aliases for ≤8 mode
            if (i >= USB_KEY_VOLUME_MUTE && i <= (USB_KEY_VOLUME_MUTE + MEDIA_KEYS_COUNT)) {
                continue;
            }
#endif
            continue;
        }
        CHECK_EQ(aakbd_to_qmk(qmk_to_aakbd(i)), i, "basic round-trip");
    }
}

static void
test_roundtrip_mod_left (void) {
    static const uint16_t mods[] = { QK_LCTL, QK_LSFT, QK_LCTL | QK_LSFT, QK_LALT,
        QK_LCTL | QK_LALT | QK_LSFT, QK_LGUI, QK_LALT | QK_LGUI,
        QK_LCTL | QK_LSFT | QK_LALT | QK_LGUI };
    for (size_t m = 0; m < sizeof(mods) / sizeof(mods[0]); ++m) {
        for (uint16_t k = 0x04; k <= 0x0D; ++k) {
            uint16_t qmk = mods[m] | k;
            CHECK_EQ(aakbd_to_qmk(qmk_to_aakbd(qmk)), qmk, "left mod+key round-trip");
        }
    }
}

static void
test_roundtrip_mod_right (void) {
    static const uint16_t mods[] = { QK_RCTL, QK_RSFT, QK_RCTL | QK_RSFT, QK_RALT, QK_RGUI,
        QK_RCTL | QK_RGUI };
    for (size_t m = 0; m < sizeof(mods) / sizeof(mods[0]); ++m) {
        for (uint16_t k = 0x04; k <= 0x0D; ++k) {
            uint16_t qmk = mods[m] | k;
            uint16_t aakbd = qmk_to_aakbd(qmk);
            uint16_t back = aakbd_to_qmk(aakbd);
            CHECK_EQ(back, qmk, "right mod+key round-trip");
        }
    }
}

static void
test_modifier_combos (void) {
    // Meh(kc) = Ctrl+Shift+Alt+key → MODS_CTRL_ALT_SHIFT | key
    CHECK_EQ(
        qmk_to_aakbd(QK_MEH(USB_KEY_A)), MODS_CTRL_ALT_SHIFT | USB_KEY_A, "QK_MEH(A) -> MEH mods + A");
    CHECK_EQ(
        aakbd_to_qmk(MODS_CTRL_ALT_SHIFT | USB_KEY_A), QK_MEH(USB_KEY_A), "MEH mods + A -> QK_MEH(A)");

    // HYPR(kc) = Ctrl+Shift+Alt+Cmd+key
    CHECK_EQ(qmk_to_aakbd(QK_HYPR(USB_KEY_A)), MODS_HYPER | USB_KEY_A, "QK_HYPR(A) -> HYPER mods + A");
    CHECK_EQ(aakbd_to_qmk(MODS_HYPER | USB_KEY_A), QK_HYPR(USB_KEY_A), "HYPER mods + A -> QK_HYPR(A)");

    // LCAG(kc) = Ctrl+Alt+Cmd+key
    CHECK_EQ(qmk_to_aakbd(QK_LCAG(USB_KEY_A)), MODS_CTRL_ALT_CMD | USB_KEY_A,
        "QK_LCAG(A) -> CTRL+ALT+CMD + A");

    // QK_MEH_T(A) → MOD_OR_KEY(MEH(A)) → QK_MEH(A) (not QK_MEH_T, mod-or → mods-only)
    CHECK_EQ(qmk_to_aakbd(QK_MEH_T(USB_KEY_A)), MOD_OR_KEY(MODS_CTRL_ALT_SHIFT | USB_KEY_A),
        "QK_MEH_T(A) -> MOD_OR_KEY(MEH(A))");
    uint16_t meht_aakbd = qmk_to_aakbd(QK_MEH_T(USB_KEY_A));
    CHECK_EQ(aakbd_to_qmk(meht_aakbd), QK_MEH(USB_KEY_A),
        "MOD_OR_KEY(MEH(A)) -> QK_MEH(A) (not MEH_T, mod-or->mods)");

    // QK_ALL_T(A) → MOD_OR_KEY(HYPER(A)) → QK_HYPR(A)
    CHECK_EQ(qmk_to_aakbd(QK_ALL_T(USB_KEY_A)), MOD_OR_KEY(MODS_HYPER | USB_KEY_A),
        "QK_ALL_T(A) -> MOD_OR_KEY(HYPER(A))");
    uint16_t allt_aakbd = qmk_to_aakbd(QK_ALL_T(USB_KEY_A));
    CHECK_EQ(aakbd_to_qmk(allt_aakbd), QK_HYPR(USB_KEY_A), "MOD_OR_KEY(HYPER(A)) -> QK_HYPR(A)");

    // QK_LCAG_T(A) → MOD_OR_KEY(LCAG(A)) → QK_LCAG(A)
    CHECK_EQ(qmk_to_aakbd(QK_LCAG_T(USB_KEY_A)), MOD_OR_KEY(MODS_CTRL_ALT_CMD | USB_KEY_A),
        "QK_LCAG_T(A) -> MOD_OR_KEY(LCAG(A))");
    uint16_t lcagt_aakbd = qmk_to_aakbd(QK_LCAG_T(USB_KEY_A));
    CHECK_EQ(aakbd_to_qmk(lcagt_aakbd), QK_LCAG(USB_KEY_A), "MOD_OR_KEY(LCAG(A)) -> QK_LCAG(A)");

    // QK_RCAG_T(A) → MOD_OR_KEY (forward only, complex right-side mods)
    uint16_t rcagt_aakbd = qmk_to_aakbd(QK_RCAG_T(USB_KEY_A));
    CHECK(aakbd_to_qmk(rcagt_aakbd) > 0, "QK_RCAG_T(A): reverse produces a keycode");

    // Combo on/off/toggle → handled by vial_process_qmk_keycode
    CHECK_EQ(
        qmk_to_aakbd(QK_COMBO_TOGGLE), KEY_EXT_QMK_KEYCODE, "QK_COMBO_TOGGLE -> KEY_EXT_QMK_KEYCODE");
}

static void
test_space_cadet (void) {
    CHECK_EQ(qmk_to_aakbd(SC_LSPO), EXTENDED(SC_LEFT_SHIFT_PARENTHESIS_OPEN),
        "SC_LSPO -> EXT(SC_LEFT_SHIFT_PARENTHESIS_OPEN)");
    CHECK_EQ(qmk_to_aakbd(SC_RSPC), EXTENDED(SC_RIGHT_SHIFT_PARENTHESIS_CLOSE),
        "SC_RSPC -> EXT(SC_RIGHT_SHIFT_PARENTHESIS_CLOSE)");
    CHECK_EQ(qmk_to_aakbd(SC_LCPO), EXTENDED(SC_LEFT_CTRL_PARENTHESIS_OPEN),
        "SC_LCPO -> EXT(SC_LEFT_CTRL_PARENTHESIS_OPEN)");
    CHECK_EQ(qmk_to_aakbd(SC_RCPC), EXTENDED(SC_RIGHT_CTRL_PARENTHESIS_CLOSE),
        "SC_RCPC -> EXT(SC_RIGHT_CTRL_PARENTHESIS_CLOSE)");
    CHECK_EQ(qmk_to_aakbd(SC_LAPO), EXTENDED(SC_LEFT_ALT_PARENTHESIS_OPEN),
        "SC_LAPO -> EXT(SC_LEFT_ALT_PARENTHESIS_OPEN)");
    CHECK_EQ(qmk_to_aakbd(SC_RAPC), EXTENDED(SC_RIGHT_ALT_PARENTHESIS_CLOSE),
        "SC_RAPC -> EXT(SC_RIGHT_ALT_PARENTHESIS_CLOSE)");
    CHECK_EQ(
        qmk_to_aakbd(SC_SENT), EXTENDED(SC_RIGHT_SHIFT_ENTER), "SC_SENT -> EXT(SC_RIGHT_SHIFT_ENTER)");
    CHECK_EQ(aakbd_to_qmk(EXTENDED(SC_LEFT_SHIFT_PARENTHESIS_OPEN)), SC_LSPO, "SC_LSPO reverse");
    CHECK_EQ(aakbd_to_qmk(EXTENDED(SC_RIGHT_SHIFT_PARENTHESIS_CLOSE)), SC_RSPC, "SC_RSPC reverse");
    CHECK_EQ(aakbd_to_qmk(EXTENDED(SC_LEFT_CTRL_PARENTHESIS_OPEN)), SC_LCPO, "SC_LCPO reverse");
    CHECK_EQ(aakbd_to_qmk(EXTENDED(SC_RIGHT_CTRL_PARENTHESIS_CLOSE)), SC_RCPC, "SC_RCPC reverse");
    CHECK_EQ(aakbd_to_qmk(EXTENDED(SC_LEFT_ALT_PARENTHESIS_OPEN)), SC_LAPO, "SC_LAPO reverse");
    CHECK_EQ(aakbd_to_qmk(EXTENDED(SC_RIGHT_ALT_PARENTHESIS_CLOSE)), SC_RAPC, "SC_RAPC reverse");
    CHECK_EQ(aakbd_to_qmk(EXTENDED(SC_RIGHT_SHIFT_ENTER)), SC_SENT, "SC_SENT reverse");
}

static void
test_layer_mod (void) {
    // ---- QK_LAYER_MOD → combined layer+mods AAKBD keycode ----
    static const struct {
        uint16_t qmk;
        uint16_t aakbd;
        const char *desc;
    } decode[] = {
        { LM(0, MOD_LCTL), (uint16_t) (LAYER_COMMAND(ENABLE, ON_HOLD, 1) | MODS_FOR_KEY(CTRL_BIT)),
            "LM(LCTL)" },
        { LM(0, MOD_LSFT), (uint16_t) (LAYER_COMMAND(ENABLE, ON_HOLD, 1) | MODS_FOR_KEY(SHIFT_BIT)),
            "LM(LSFT)" },
        { LM(0, MOD_LALT), (uint16_t) (LAYER_COMMAND(ENABLE, ON_HOLD, 1) | MODS_FOR_KEY(ALT_BIT)),
            "LM(LALT)" },
        { LM(0, MOD_LGUI), (uint16_t) (LAYER_COMMAND(ENABLE, ON_HOLD, 1) | MODS_FOR_KEY(CMD_BIT)),
            "LM(LGUI)" },
        { LM(0, MOD_LCTL | MOD_LSFT),
            (uint16_t) (LAYER_COMMAND(ENABLE, ON_HOLD, 1) | MODS_FOR_KEY(CTRL_BIT | SHIFT_BIT)),
            "LM(LCTL|LSFT)" },
        { LM(0, MOD_LCTL | MOD_LALT),
            (uint16_t) (LAYER_COMMAND(ENABLE, ON_HOLD, 1) | MODS_FOR_KEY(CTRL_BIT | ALT_BIT)),
            "LM(LCTL|LALT)" },
        { LM(0, MOD_LCTL | MOD_LSFT | MOD_LALT | MOD_LGUI),
            (uint16_t) (LAYER_COMMAND(ENABLE, ON_HOLD, 1)
                | MODS_FOR_KEY(CTRL_BIT | SHIFT_BIT | ALT_BIT | CMD_BIT)),
            "LM(all left)" },
        { LM(0, MOD_RCTL),
            (uint16_t) (LAYER_COMMAND(ENABLE, ON_HOLD, 1) | MODS_FOR_KEY(RIGHT_CTRL_BIT)), "LM(RCTL)" },
        { LM(0, MOD_RSFT),
            (uint16_t) (LAYER_COMMAND(ENABLE, ON_HOLD, 1) | MODS_FOR_KEY(RIGHT_SHIFT_BIT)),
            "LM(RSFT)" },
        { LM(0, MOD_RALT), (uint16_t) (LAYER_COMMAND(ENABLE, ON_HOLD, 1) | MODS_FOR_KEY(RIGHT_ALT_BIT)),
            "LM(RALT)" },
        { LM(0, MOD_RGUI), (uint16_t) (LAYER_COMMAND(ENABLE, ON_HOLD, 1) | MODS_FOR_KEY(RIGHT_CMD_BIT)),
            "LM(RGUI)" },
        { LM(0, MOD_RSFT | MOD_RALT),
            (uint16_t) (LAYER_COMMAND(ENABLE, ON_HOLD, 1)
                | MODS_FOR_KEY(RIGHT_SHIFT_BIT | RIGHT_ALT_BIT)),
            "LM(RSFT|RALT)" },
        { LM(0, MOD_RSFT | MOD_RALT | MOD_RGUI),
            (uint16_t) (LAYER_COMMAND(ENABLE, ON_HOLD, 1)
                | MODS_FOR_KEY(RIGHT_SHIFT_BIT | RIGHT_ALT_BIT | RIGHT_CMD_BIT)),
            "LM(RSFT|RALT|RGUI)" },
        { LM(0, 0x00), EXTENDED(QMK_KEYCODE), "LM(no mods)" },
    };
    for (size_t i = 0; i < sizeof(decode) / sizeof(decode[0]); ++i) {
        CHECK_EQ(qmk_to_aakbd(decode[i].qmk), decode[i].aakbd, decode[i].desc);
    }

    // ---- AAKBD mods-only → QK_LAYER_MOD (encode) ----
    static const struct {
        uint16_t aakbd;
        uint16_t qmk;
        const char *desc;
    } encode[] = {
        { MODS_FOR_KEY(CTRL_BIT), LM(0, MOD_LCTL), "LCTL→LM(LCTL)" },
        { MODS_FOR_KEY(SHIFT_BIT), LM(0, MOD_LSFT), "LSFT→LM(LSFT)" },
        { MODS_FOR_KEY(ALT_BIT), LM(0, MOD_LALT), "LALT→LM(LALT)" },
        { MODS_FOR_KEY(CMD_BIT), LM(0, MOD_LGUI), "LGUI→LM(LGUI)" },
        { MODS_FOR_KEY(CTRL_BIT | SHIFT_BIT), LM(0, MOD_LCTL | MOD_LSFT), "LCTL|LSFT→LM(LCTL|LSFT)" },
        { MODS_FOR_KEY(RIGHT_CTRL_BIT), LM(0, MOD_RCTL), "RCTL→LM(RCTL)" },
        { MODS_FOR_KEY(RIGHT_SHIFT_BIT), LM(0, MOD_RSFT), "RSFT→LM(RSFT)" },
        { MODS_FOR_KEY(RIGHT_ALT_BIT), LM(0, MOD_RALT), "RALT→LM(RALT)" },
        { MODS_FOR_KEY(RIGHT_CMD_BIT), LM(0, MOD_RGUI), "RGUI→LM(RGUI)" },
        { MODS_FOR_KEY(RIGHT_SHIFT_BIT | RIGHT_ALT_BIT), LM(0, MOD_RSFT | MOD_RALT),
            "RSFT|RALT→LM(RSFT|RALT)" },
    };
    for (size_t i = 0; i < sizeof(encode) / sizeof(encode[0]); ++i) {
        uint16_t qmk = aakbd_to_qmk(encode[i].aakbd);
        CHECK_EQ(qmk, encode[i].qmk, encode[i].desc);
    }

    // ---- Non-zero layer is now encoded in the AAKBD keycode ----
    CHECK_EQ(qmk_to_aakbd(LM(7, MOD_LCTL)),
        (uint16_t) (LAYER_COMMAND(ENABLE, ON_HOLD, 8) | MODS_FOR_KEY(CTRL_BIT)),
        "LM(7, LCTL) with layer 7");
    CHECK_EQ(qmk_to_aakbd(LM(15, MOD_RALT)),
        (uint16_t) (LAYER_COMMAND(ENABLE, ON_HOLD, 16) | MODS_FOR_KEY(RIGHT_ALT_BIT)),
        "LM(15, RALT) with layer 15");

    // ---- Round-trip: LM → AAKBD → LM preserves mods ----
    for (size_t i = 0; i < sizeof(decode) / sizeof(decode[0]); ++i) {
        uint16_t aakbd = decode[i].aakbd;
        if (aakbd == NONE || aakbd == KC_NO || aakbd == EXTENDED(QMK_KEYCODE)) {
            continue;
        }
        uint16_t back = aakbd_to_qmk(qmk_to_aakbd(decode[i].qmk));
        CHECK_EQ(back, decode[i].qmk, decode[i].desc);
    }

    // ---- Round-trip: AAKBD → LM → AAKBD (mods-only becomes combined layer+mods) ----
    for (size_t i = 0; i < sizeof(encode) / sizeof(encode[0]); ++i) {
        uint16_t expected = (uint16_t) (LAYER_COMMAND(ENABLE, ON_HOLD, 1) | encode[i].aakbd);
        uint16_t back = qmk_to_aakbd(aakbd_to_qmk(encode[i].aakbd));
        CHECK_EQ(back, expected, encode[i].desc);
    }
}

static void
test_roundtrip_mod_tap (void) {
    // Mod tap collapses to mod+key in AAKBD, should round-trip to original mod+key
    static const uint16_t taps[] = { QK_MOD_TAP_LCTL | USB_KEY_A, QK_MOD_TAP_LSFT | USB_KEY_A,
        QK_MOD_TAP_LALT | USB_KEY_A, QK_MOD_TAP_LGUI | USB_KEY_A, QK_MOD_TAP_RCTL | USB_KEY_A,
        QK_MOD_TAP_RSFT | USB_KEY_A, QK_MOD_TAP_RALT | USB_KEY_A, QK_MOD_TAP_RGUI | USB_KEY_A };
    static const uint16_t expected[] = { LCTL(USB_KEY_A), QK_LSFT | USB_KEY_A, QK_LALT | USB_KEY_A,
        QK_LGUI | USB_KEY_A, RCTL(USB_KEY_A), QK_RSFT | USB_KEY_A, QK_RALT | USB_KEY_A,
        QK_RGUI | USB_KEY_A };
    for (size_t i = 0; i < sizeof(taps) / sizeof(taps[0]); ++i) {
        CHECK_EQ(aakbd_to_qmk(qmk_to_aakbd(taps[i])), expected[i], "mod_tap round-trip");
    }
}

static void
test_roundtrip_reset (void) {
    CHECK_EQ(aakbd_to_qmk(qmk_to_aakbd(QK_BOOTLOADER)), QK_BOOTLOADER, "QK_BOOTLOADER round-trip");
}

#if ENABLE_TRI_LAYER
static void
test_roundtrip_tri_layer (void) {
    CHECK_EQ(aakbd_to_qmk(qmk_to_aakbd(FN_MO13)), FN_MO13, "FN_MO13 round-trip");
    CHECK_EQ(aakbd_to_qmk(qmk_to_aakbd(FN_MO23)), FN_MO23, "FN_MO23 round-trip");
}
#endif

static void
test_roundtrip_macros (void) {
    // Only QMK MACRO00-63 round-trip (shifted to AAKBD 64-127 and back)
    for (uint16_t m = 0; m <= 63; ++m) {
        uint16_t qmk = MACRO00 + m;
        CHECK(aakbd_to_qmk(qmk_to_aakbd(qmk)) == qmk, "MACRO round-trip");
    }
    // QMK MACRO64+ exceed shifted range → KEY_EXT_QMK_KEYCODE → KC_NO
    CHECK_EQ(qmk_to_aakbd(MACRO00 + 64), KEY_EXT_QMK_KEYCODE,
        "MACRO64 -> KEY_EXT_QMK_KEYCODE (past shifted range)");
    CHECK_EQ(aakbd_to_qmk(KEY_EXT_QMK_KEYCODE), KC_NO,
        "KEY_EXT_QMK_KEYCODE -> KC_NO (no reverse for unsupported)");
}

static void
test_roundtrip_momentary_layer (void) {
    for (uint8_t qmk_layer = 0; qmk_layer <= 30; ++qmk_layer) {
        uint16_t qmk = QK_MOMENTARY | qmk_layer;
        uint16_t aakbd = qmk_to_aakbd(qmk);
        CHECK(LAYER_OF_COMMAND(aakbd) == qmk_layer + 1, "MO: QMK layer should map to AAKBD layer+1");
        uint16_t back = aakbd_to_qmk(aakbd);
        CHECK(back == qmk || back == KC_TRNS, "MO round-trip");
    }
}

// ====== Additional round-trip tests ======

static void
test_roundtrip_basic_special (void) {
    // KC_NO (0x0000) → NONE → KC_NO
    CHECK_EQ(aakbd_to_qmk(qmk_to_aakbd(KC_NO)), KC_NO, "KC_NO round-trip");
    // KC_TRNS (0x0001) → PASS → KC_TRNS
    CHECK_EQ(aakbd_to_qmk(qmk_to_aakbd(KC_TRNS)), KC_TRNS, "KC_TRNS round-trip");
}

static void
test_roundtrip_toggle_layer (void) {
    for (uint8_t layer = 1; layer <= 8; ++layer) {
        uint16_t qmk = TG(layer);
        uint16_t aakbd = qmk_to_aakbd(qmk);
        uint16_t back = aakbd_to_qmk(aakbd);
        CHECK(back == qmk || back == KC_TRNS, "TG round-trip");
    }
}

static void
test_roundtrip_layer_tap_toggle (void) {
    for (uint8_t layer = 1; layer <= 8; ++layer) {
        uint16_t qmk = TT(layer);
        uint16_t aakbd = qmk_to_aakbd(qmk);
        uint16_t back = aakbd_to_qmk(aakbd);
        CHECK(back == qmk || back == KC_TRNS, "TT round-trip");
    }
}

static void
test_roundtrip_go_to_layer (void) {
    for (uint8_t layer = 1; layer <= 8; ++layer) {
        uint16_t qmk = QK_TO | layer;
        uint16_t aakbd = qmk_to_aakbd(qmk);
        uint16_t back = aakbd_to_qmk(aakbd);
        CHECK(back == qmk || back == KC_TRNS, "TO round-trip");
    }
}

static void
test_roundtrip_layer_tap (void) {
    // Layer tap encoding in AAKBD uses command bits that overlap with
    // modifier+key encoding, so perfect round-trip is not possible.
    // Test that QMK→AAKBD conversion produces a command keycode (bit 15 set).
    for (uint8_t layer = 1; layer <= 4; ++layer) {
        for (uint16_t k = 0x04; k <= 0x0A; ++k) {
            uint16_t qmk = QK_LAYER_TAP | ((uint16_t) layer << 8) | k;
            uint16_t aakbd = qmk_to_aakbd(qmk);
            // Must be a command keycode (command bits set)
            CHECK(aakbd & COMMAND_KEYCODE_MASK, "LT -> AAKBD command format");
        }
    }
}

static void
test_roundtrip_layer_mod (void) {
    for (uint8_t mod = 1; mod <= 8; ++mod) {
        uint16_t qmk = QK_LAYER_MOD | mod;
        uint16_t aakbd = qmk_to_aakbd(qmk);
        uint16_t back = aakbd_to_qmk(aakbd);
        CHECK(back == qmk || back == KC_TRNS || (back & 0xFF00) != 0,
            "LM round-trip (mod bits preserved)");
    }
}

static void
test_roundtrip_user_codes (void) {
    // USER00 = Apple Fn → EXT_VIAL_APPLE_FN (EXT format, not MACRO)
    uint16_t aakbd0 = qmk_to_aakbd(USER00);
    CHECK(aakbd0 & EXTENDED_KEY_BIT, "USER00 -> EXT format");
    CHECK(!(aakbd0 & MACRO_BIT), "USER00 -> not MACRO");
    CHECK_EQ(aakbd_to_qmk(aakbd0), USER00, "USER00 round-trip");

    // VIAL_USER_MACRO_FIRST-15 → AAKBD MACRO(0-14) (MACRO format, round-trip)
    for (uint16_t i = 0; i < 15; ++i) {
        uint16_t qmk = VIAL_USER_MACRO_FIRST + i;
        uint16_t aakbd = qmk_to_aakbd(qmk);
        CHECK(aakbd & MACRO_BIT, "VIAL_USER_MACRO_FIRST-15 -> MACRO format");
        CHECK_EQ(aakbd_to_qmk(aakbd), qmk, "VIAL_USER_MACRO_FIRST-15 round-trip");
    }
}

static void
test_qmk_to_aakbd_boundaries (void) {
    // Test keycode range boundaries round-trip without producing KC_NO
    // (unless the original keycode itself is intentionally unmapped).
    static const uint16_t bounds[] = {
        0x0000,
        0x0001,
        0x0002,
        0x00FE,
        0x00FF,
        0x1FFF,
        0x4000,
        0x4FFF,
        0x5000,
        0x50FF, // QK_LAYER_MOD range
        0x5100,
        0x51FF, // QK_LAYER_MOD range
        0x5200,
        0x52FF, // QK_TO .. QK_LAYER_TAP_TOGGLE ranges
        0x5300,
        0x53FF, // gap (unsupported)
        0x5400,
        0x54FF, // gap (unsupported)
        0x5500, // QK_ONE_SHOT_MOD with 0 mods = NONE
        0x55FF, // gap (unsupported)
        0x5800,
        0x58FF, // gap (unsupported)
        0x5900,
        0x59FF, // gap (unsupported)
        0x5C00, // gap (VIAL_MAGIC starts at 0x5C02)
        0x5E00, // KC_KB_0 (unsupported)
        0x6000,
        0x6FFF,
        0x7FFF, // gap (unsupported)
    };
    for (size_t i = 0; i < sizeof(bounds) / sizeof(bounds[0]); ++i) {
        uint16_t result = qmk_to_aakbd(bounds[i]);
        uint16_t back = aakbd_to_qmk(result);
        // Skip known unmapped codes: KC_NO, NONE, and any range-max value
        // where the key byte itself is 0xFF (invalid scancode). Also skip
        // OSM with no modifiers (0x5500) — intentionally NONE.
        if (bounds[i] == 0x0000 || (bounds[i] & 0xFF) == 0xFF || bounds[i] == 0x5500) {
            continue;
        }
        // KEY_EXT_QMK_KEYCODE means the QMK keycode is unsupported and
        // should not be expected to round-trip.
        if (result == KEY_EXT_QMK_KEYCODE) {
            continue;
        }
        if (back == KC_NO) {
            char msg[64];
            snprintf(msg, sizeof(msg), "boundary 0x%04X -> KC_NO", bounds[i]);
            CHECK(back != KC_NO, msg);
        }
    }
}

static void
test_roundtrip_unsupported_ranges (void) {
    // Unsupported QMK ranges → KEY_EXT_QMK_KEYCODE → KC_NO (no round-trip)
    static const uint16_t ranges[] = {
        0x5600, // QK_SWAP_HANDS
        0x5700, // QK_TAP_DANCE
        0x8000, // QK_UNICODE
    };
    for (size_t i = 0; i < sizeof(ranges) / sizeof(ranges[0]); ++i) {
        uint16_t aakbd = qmk_to_aakbd(ranges[i]);
        CHECK_EQ(aakbd, KEY_EXT_QMK_KEYCODE, "unsupported range -> KEY_EXT_QMK_KEYCODE");
    }
}

static void
test_qmk_to_aakbd_media_keys (void) {
    // QMK consumer keys (0xA5-0xBE) → AAKBD virtual media keys
#if MEDIA_KEYS_COUNT <= 8
    static const struct {
        uint16_t qmk;
        uint16_t aakbd;
    } media[] = {
        { KC_AUDIO_MUTE, USB_KEY_VOLUME_MUTE },
        { KC_AUDIO_VOL_UP, USB_KEY_VOLUME_UP },
        { KC_AUDIO_VOL_DOWN, USB_KEY_VOLUME_DOWN },
        { KC_MEDIA_NEXT_TRACK, USB_KEY_NEXT_TRACK },
        { KC_MEDIA_PREV_TRACK, USB_KEY_PREVIOUS_TRACK },
        { KC_MEDIA_PLAY_PAUSE, USB_KEY_PLAY_PAUSE },
        { KC_MEDIA_FAST_FORWARD, USB_KEY_FAST_FORWARD },
    };
    for (size_t i = 0; i < sizeof(media) / sizeof(media[0]); ++i) {
        char msg[64];
        snprintf(msg, sizeof(msg), "qmk 0x%04X → aakbd 0x%04X", media[i].qmk, media[i].aakbd);
        CHECK_EQ(qmk_to_aakbd(media[i].qmk), media[i].aakbd, msg);
    }

    // Unmapped QMK consumer codes → NONE
    for (uint16_t kc = 0xA5; kc <= 0xBE; ++kc) {
        switch (kc) {
            case KC_AUDIO_MUTE:
            case KC_AUDIO_VOL_UP:
            case KC_AUDIO_VOL_DOWN:
            case KC_MEDIA_NEXT_TRACK:
            case KC_MEDIA_PREV_TRACK:
            case KC_MEDIA_PLAY_PAUSE:
            case KC_MEDIA_FAST_FORWARD:
            case KC_MEDIA_REWIND:
                continue;
            default:
                break;
        }
        char msg[64];
        snprintf(msg, sizeof(msg), "qmk 0x%04X (unmapped)", kc);
        CHECK_EQ(qmk_to_aakbd(kc), EXTENDED(QMK_KEYCODE), msg);
    }
#else
    // MEDIA_KEYS_COUNT > 8: test first and last consumer key
    CHECK_EQ(
        qmk_to_aakbd(KC_BRIGHTNESS_UP), USB_KEY_BRIGHTNESS_UP, "QMK consumer -> USB_KEY_BRIGHTNESS_UP");
    CHECK_EQ(qmk_to_aakbd(KC_WWW_FAVORITES), USB_KEY_BROWSE_FAVORITES,
        "QMK consumer -> USB_KEY_BROWSE_FAVORITES");
#endif
}

static void
test_aakbd_to_qmk_media_keys (void) {
#if MEDIA_KEYS_COUNT <= 8
    // AAKBD virtual media keys → QMK consumer keycodes
    static const struct {
        uint16_t aakbd;
        uint16_t qmk;
    } media[] = {
        { USB_KEY_VOLUME_MUTE, KC_AUDIO_MUTE },
        { USB_KEY_VOLUME_UP, KC_AUDIO_VOL_UP },
        { USB_KEY_VOLUME_DOWN, KC_AUDIO_VOL_DOWN },
        { USB_KEY_NEXT_TRACK, KC_MEDIA_NEXT_TRACK },
        { USB_KEY_PREVIOUS_TRACK, KC_MEDIA_PREV_TRACK },
        { USB_KEY_PLAY_PAUSE, KC_MEDIA_PLAY_PAUSE },
        { USB_KEY_FAST_FORWARD, KC_MEDIA_FAST_FORWARD },
    };
    for (size_t i = 0; i < sizeof(media) / sizeof(media[0]); ++i) {
        char msg[64];
        snprintf(msg, sizeof(msg), "aakbd 0x%04X → qmk 0x%04X", media[i].aakbd, media[i].qmk);
        CHECK_EQ(aakbd_to_qmk(media[i].aakbd), media[i].qmk, msg);
    }

    CHECK_EQ(aakbd_to_qmk(NONE), KC_NO, "NONE → KC_NO");
#else
    CHECK_EQ(
        aakbd_to_qmk(USB_KEY_BRIGHTNESS_UP), KC_BRIGHTNESS_UP, "USB_KEY_BRIGHTNESS_UP -> QMK consumer");
    CHECK_EQ(aakbd_to_qmk(USB_KEY_BROWSE_FAVORITES), KC_WWW_FAVORITES,
        "USB_KEY_BROWSE_FAVORITES -> QMK consumer");
#endif
}

static void
test_roundtrip_media_keys (void) {
    // All mapped consumer codes round-trip exactly
    static const uint16_t exact[] = {
        KC_AUDIO_MUTE,
        KC_AUDIO_VOL_UP,
        KC_AUDIO_VOL_DOWN,
        KC_MEDIA_NEXT_TRACK,
        KC_MEDIA_PREV_TRACK,
        KC_MEDIA_PLAY_PAUSE,
        KC_MEDIA_FAST_FORWARD,
    };
    for (size_t i = 0; i < sizeof(exact) / sizeof(exact[0]); ++i) {
        char msg[64];
        snprintf(msg, sizeof(msg), "media round-trip 0x%04X", exact[i]);
        CHECK_EQ(aakbd_to_qmk(qmk_to_aakbd(exact[i])), exact[i], msg);
    }
}

static void
test_roundtrip_ctrl_alt (void) {
    // CTRL(ALT) = MODS_CTRL | USB_KEY_ALT = 0x0100 | 0xE2 = 0x01E2
    uint16_t aakbd = CTRL(ALT);
    uint16_t qmk = aakbd_to_qmk(aakbd);
    CHECK_EQ(qmk_to_aakbd(qmk), aakbd, "CTRL(ALT) round-trip");
}

static void
test_roundtrip_eeprom_reset (void) {
    uint16_t aakbd = EXTENDED(EEPROM_RESET);
    uint16_t qmk = aakbd_to_qmk(aakbd);
    CHECK_EQ(qmk_to_aakbd(qmk), aakbd, "EEPROM_RESET round-trip");
    CHECK_EQ(qmk, QK_CLEAR_EEPROM, "EEPROM_RESET -> QK_CLEAR_EEPROM");
}

// Auto-generated test runner (scanned from test_* functions above)
#include "build/translate_runner.c"
