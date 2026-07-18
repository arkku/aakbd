#include <stdint.h>
#include "config.h"
#include "wcass.h"
#include "keymap.h"
#include "progmem.h"

#if VIAL_ENABLE
#include "vial/vial.h"
#include "vial/qmk_translate.h"
#include "vial/dynamic_keymap.h"
#endif

#define _BASE   0
#define _FN     KC_APFN

// Keep in mind that the keymaps here _must_ have each physical key mapped to a
// unique keycode! It is highly recommended to remap the keys in layers.c
// rather than editing these keymaps. That being said, if your keys all have
// unique keycaps, it may make sense to edit the layout here to match those.
// But keep in mind that this file is included in the Git repository, so
// updating that may overwrite your local changes. You can use
// `keymap_custom.c` to do local variant.
//
// Make sure to define the correct layout parameters for your used keys!
// Defining unused keys here may make calibration worse since flipperless
// keys may extend the range of encountered capacitances.

// Since there is no canonical mapping for this keyboard, the first two blocks
// of 3×5 keys are F1-F24 vertically in groups of four starting from the top,
// and the bottom 3 keys in each block are using keypad keys A-F (for
// hexadecimal input, seldom seen in reality) to make it easy to remember which
// key is which when remapping them in `layers.c`. The rightmost block is a
// standard PC numpad by default, but the three double-wide keys can be split
// using parameters (see below).

// List of parameters (define each to 0 or 1):
// - SPLIT_PAD_ENTER
// - SPLIT_PAD_PLUS
// - SPLIT_PAD_ZERO
//
// If all are zero, the rightmost block is the normal PC numpad, where each
// of the enter, plus and zero keys are double size. The splits for them are
// `KC_PEQL`, `KC_BSPC` and `KC_P00`, respectively. In the physical keyboard,
// the top or left pad is the one used for the non-split key.

#ifndef LAYOUT_ALL_PADS
#define LAYOUT_ALL_PADS VIAL_ENABLE
#endif

#if LAYOUT_ALL_PADS
#define KC_PENT1    KC_PENT
#define KC_PENT2    KC_PEQL
#elif SPLIT_PAD_ENTER
#define KC_PENT1    KC_PEQL
#define KC_PENT2    KC_PENT
#else
#define KC_PENT1    KC_PENT
#define KC_PENT2    KC_NO
#endif

#if SPLIT_PAD_PLUS || LAYOUT_ALL_PADS
#define KC_PPLS1    KC_PPLS
#define KC_PPLS2    KC_BSPC
#else
#define KC_PPLS1    KC_PPLS
#define KC_PPLS2    KC_NO
#endif

#if SPLIT_PAD_ZERO
#define KC_P0L      KC_P0
#define KC_P0R      KC_P00
#elif LAYOUT_ALL_PADS
#define KC_P0L      KC_0
#define KC_P0R      KC_P0
#else
#define KC_P0L      KC_NO
#define KC_P0R      KC_P0
#endif

#if !defined(CUSTOM_KEYMAP) || !CUSTOM_KEYMAP

// See config.h information about the key naming scheme if you are editing
// your `layers.c` and need to know the name of each key.

// You can also define `CUSTOM_KEYMAP=1` and copy this to your `layers.c`
// and do your "natural" layout there (this file uses the QMK keycode syntax,
// but you can use AAKBD syntax as well, just remember that the keycodes must
// be "plain", i.e., no macros or extended keys or layer commands allowed.

const uint8_t PROGMEM keymaps[1][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT_all(
        KC_R1C1, KC_R1C2, KC_R1C3, KC_R1C4, KC_R1C5, KC_R1C6,   KC_NLCK, KC_PSLS, KC_PAST, KC_PMNS,
        KC_R2C1, KC_R2C2, KC_R2C3, KC_R2C4, KC_R2C5, KC_R2C6,   KC_P7,   KC_P8,   KC_P9,   KC_PPLS1,
        KC_R3C1, KC_R3C2, KC_R3C3, KC_R3C4, KC_R3C5, KC_R3C6,   KC_P4,   KC_P5,   KC_P6,   KC_PPLS2,
        KC_R4C1, KC_R4C2, KC_R4C3, KC_R4C4, KC_R4C5, KC_R4C6,   KC_P1,   KC_P2,   KC_P3,   KC_PENT1,
        KC_R5C1, KC_R5C2, KC_R5C3, KC_R5C4, KC_R5C5, KC_R5C6,   KC_P0L,  KC_P0R,  KC_PDOT, KC_PENT2
    )
};

#endif

#if VIAL_ENABLE

#define LAYOUT_BIT_SPLIT_PAD_ZERO           0
#define LAYOUT_BIT_SPLIT_PAD_PLUS           1
#define LAYOUT_BIT_SPLIT_PAD_ENTER          2

const uint16_t vial_default_layout_options PROGMEM = (
    ((SPLIT_PAD_ENTER ? 1 : 0) << LAYOUT_BIT_SPLIT_PAD_ENTER) |
    ((SPLIT_PAD_PLUS ? 1 : 0) << LAYOUT_BIT_SPLIT_PAD_PLUS) |
    ((SPLIT_PAD_ZERO ? 1 : 0) << LAYOUT_BIT_SPLIT_PAD_ZERO)
);

const uint8_t vial_keyboard_uid[8] PROGMEM = { 0x01, 0x46, 0x50, 0x41, 0x41, 0x4B, 0x42, 0x44 };

const uint8_t PROGMEM vial_unlock_combo_rows[] = { 4, 6 };
const uint8_t PROGMEM vial_unlock_combo_cols[] = { 0, 6 };
const uint8_t vial_unlock_combo_len = 2;

static void update_keycode(uint8_t keycode, uint8_t row, uint8_t col, bool should_be_active) {
    const uint16_t current = dynamic_keymap_get_qmk_keycode(0, row, col);
    if (should_be_active) {
        if (current == KC_NO) {
            dynamic_keymap_set_qmk_keycode(0, row, col, aakbd_to_qmk(keycode));
        }
    } else if (current != KC_NO) {
        dynamic_keymap_set_qmk_keycode(0, row, col, KC_NO);
    }
}

void dynamic_keymap_layout_updated(const uint16_t old_opts, const uint16_t opts) {
    update_keycode(KC_PENT, 3, 6, opts & (1 << LAYOUT_BIT_SPLIT_PAD_ENTER));
    update_keycode(KC_PPLS, 7, 6, opts & (1 << LAYOUT_BIT_SPLIT_PAD_PLUS));
    update_keycode(KC_P0, 3, 4, opts & (1 << LAYOUT_BIT_SPLIT_PAD_ZERO));
}

#endif
