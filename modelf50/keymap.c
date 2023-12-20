#include <stdint.h>
#include "progmem.h"
#include "keymap.h"
#include "wcass.h"

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


#if SPLIT_PAD_ENTER
#define KC_PENT1    KC_PEQL
#define KC_PENT2    KC_PENT
#else
#define KC_PENT1    KC_PENT
#define KC_PENT2    KC_NO
#endif

#if SPLIT_PAD_PLUS
#define KC_PPLS2    KC_BSPC
#else
#define KC_PPLS2    KC_NO
#endif

#if SPLIT_PAD_ZERO
#define KC_BMID     KC_P00
#else
#define KC_BMID     KC_NO
#endif

const uint8_t PROGMEM keymaps[1][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT_all(
        KC_F1,   KC_F5,   KC_F9,    KC_F13,  KC_F17,  KC_F21,     KC_NLCK, KC_PSLS, KC_PAST, KC_PMNS,
        KC_F2,   KC_F6,   KC_F10,   KC_F14,  KC_F18,  KC_F22,     KC_P7,   KC_P8,   KC_P9,   KC_PPLS,
        KC_F3,   KC_F7,   KC_F11,   KC_F15,  KC_F19,  KC_F23,     KC_P4,   KC_P5,   KC_P6,   KC_PPLS2,
        KC_F4,   KC_F8,   KC_F12,   KC_F16,  KC_F20,  KC_F24,     KC_P1,   KC_P2,   KC_P3,   KC_PENT1,
        KC_PA,   KC_PB,   KC_PC,    KC_PD,   KC_PE,   KC_PF,      KC_P0,   KC_BMID, KC_PDOT, KC_PENT2
    )
};

