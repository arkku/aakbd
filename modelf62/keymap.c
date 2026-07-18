#include <stdint.h>
#include "progmem.h"
#include "keymap.h"
#include "wcass.h"
#include "config.h"
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

// Layout variations to configure:
//
// * ISO_ENTER
// * SPLIT_ENTER
// * SPLIT_LEFT_SHIFT
// * SPLIT_RIGHT_SHIFT
// * NON_HHKB_SPLIT_RIGHT_SHIFT (only effective if SPLIT_RIGHT_SHIFT)
// * SPLIT_BACKSPACE
// * SHORT_SPACE
//
// For simplicity, defining `ISO_LAYOUT=1` enables `ISO_ENTER`, `SPLIT_ENTER`,
// and `SPLIT_LEFT_SHIFT` - these may sound odd but they are the default ISO
// layout, i.e., one extra key on home row ("split enter", compared to ANSI),
// and one extra key on bottom row (split left shift).
//
// Without `ISO_LAYOUT=1` the default is ANSI layout.
//
// The default of `SPLIT_RIGHT_SHIFT` is the "HHKB" split, i.e., shift on the
// left and a 1U extra key on the right. The `NON_HHKB_SPLIT_RIGHT_SHIFT`
// option flips these around, so there is an extra 1U key on the left and
// the shift on the right.
//
// `SHORT_SPACE` adds on extra key to the right of space.
//
// See below for the 4 different Enter options.

#ifndef LAYOUT_ALL_PADS
#define LAYOUT_ALL_PADS VIAL_ENABLE
#endif

#ifndef NON_HHKB_SPLIT_RIGHT_SHIFT
#define NON_HHKB_SPLIT_RIGHT_SHIFT 0
#endif

#ifndef SHORT_SPACE
#define SHORT_SPACE 0
#endif

#ifndef SPLIT_SPACE_RIGHT
#define SPLIT_SPACE_RIGHT SHORT_SPACE
#endif

#if SPLIT_RIGHT_SHIFT
#if NON_HHKB_SPLIT_RIGHT_SHIFT
#define K_RSF1  KC_APFN
#define K_RSF2  KC_RSFT
#else
#define K_RSF1  KC_RSFT
#define K_RSF2  KC_APFN
#endif
#else // ^ SPLIT_RIGHT_SHIFT
#if LAYOUT_ALL_PADS
#define K_RSF1  KC_NO
#else
#define K_RSF1  KC_APFN
#endif
#define K_RSF2  KC_RSFT
#endif

#if SPLIT_BACKSPACE || LAYOUT_ALL_PADS
#define K_BSPL  KC_GRV
#else
#define K_BSPL  KC_NO
#endif

#ifndef SPLIT_LEFT_SHIFT
#define SPLIT_LEFT_SHIFT ISO_LAYOUT
#endif

#if SPLIT_LEFT_SHIFT || LAYOUT_ALL_PADS
#define K_LSFR  KC_NUBS
#else
#define K_LSFR  KC_NO
#endif

#ifndef ISO_ENTER
#define ISO_ENTER ISO_LAYOUT
#endif
#ifndef SPLIT_ENTER
#define SPLIT_ENTER ISO_ENTER
#endif

#if ISO_ENTER && !LAYOUT_ALL_PADS

// ISO and BA enter: 1.5U top row
#define K_USBS  KC_NO

#if SPLIT_ENTER
// ISO enter: 1.25U home row (extra 1U key)
#define K_ISOH  KC_NUHS
#else
// BA enter: 2.25U home row
#define K_ISOH  KC_NO
#endif

#else // ^ ISO / BA enter

// Separate 1.5U key on top row
#define K_USBS  KC_BSLS

#if SPLIT_ENTER || LAYOUT_ALL_PADS
// Tie enter: 1.25U home row (extra 1U key)
#define K_ISOH  KC_NUHS
#else
// ANSI enter: 2.25U home row
#define K_ISOH  KC_NO
#endif

#endif // ^ ANSI / Tie enter

#if SPLIT_SPACE_RIGHT || LAYOUT_ALL_PADS
#define K_SSPR  KC_RGUI
#else
#define K_SSPR  KC_NO
#endif

#if SPLIT_SPACE_LEFT
#error "Space split on left side is not implemented. Edit LAYOUT_all macro."
#endif

const uint8_t PROGMEM keymaps[1][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT_all(
        KC_ESC, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, KC_EQL, K_BSPL, KC_BSPC,
        KC_TAB,    KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC, KC_RBRC, K_USBS,
        KC_CAPS,      KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT, K_ISOH, KC_ENT,
        KC_LSFT, K_LSFR, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, K_RSF1, K_RSF2,
        KC_LCTRL, KC_LGUI, KC_LALT,                      KC_SPC,         K_SSPR, KC_RALT, KC_NLCK, KC_RCTRL
    )
};

#if VIAL_ENABLE

#define LAYOUT_BIT_ISO_LEFT_SHIFT       0
#define LAYOUT_BIT_SPLIT_SPACE          1
#define LAYOUT_BIT_ENTER_0              2
#define LAYOUT_BIT_ENTER_1              3
#define LAYOUT_BIT_SPLIT_RIGHT_SHIFT_0  4
#define LAYOUT_BIT_SPLIT_RIGHT_SHIFT_1  5
#define LAYOUT_BIT_SPLIT_BACKSPACE      6

const uint16_t vial_default_layout_options PROGMEM = (
    ((SPLIT_BACKSPACE ? 1 : 0) << LAYOUT_BIT_SPLIT_BACKSPACE) |
    ((SPLIT_RIGHT_SHIFT && !NON_HHKB_SPLIT_RIGHT_SHIFT ? 1 : 0) << LAYOUT_BIT_SPLIT_RIGHT_SHIFT_0) |
    ((SPLIT_RIGHT_SHIFT && NON_HHKB_SPLIT_RIGHT_SHIFT ? 1 : 0) << LAYOUT_BIT_SPLIT_RIGHT_SHIFT_1) |
    ((SPLIT_ENTER ? 1 : 0) << LAYOUT_BIT_ENTER_0) |
    ((ISO_ENTER ? 1 : 0) << LAYOUT_BIT_ENTER_1) |
    ((SHORT_SPACE ? 1 : 0) << LAYOUT_BIT_SPLIT_SPACE) |
    ((SPLIT_LEFT_SHIFT ? 1 : 0) << LAYOUT_BIT_ISO_LEFT_SHIFT)
);

const uint8_t vial_keyboard_uid[8] PROGMEM = { 0x01, 0x46, 0x62, 0x41, 0x41, 0x4B, 0x42, 0x44 };

const uint8_t PROGMEM vial_unlock_combo_rows[] = { 4, 2 };
const uint8_t PROGMEM vial_unlock_combo_cols[] = { 0, 7 };
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
    update_keycode(K_BSPL, 4, 7, opts & (1 << LAYOUT_BIT_SPLIT_BACKSPACE));
    update_keycode(KC_APFN, 0, 7, opts & ((1 << LAYOUT_BIT_SPLIT_RIGHT_SHIFT_0) | (1 << LAYOUT_BIT_SPLIT_RIGHT_SHIFT_1)));
    update_keycode(KC_NUHS, 7, 6, opts & (1 << LAYOUT_BIT_ENTER_0));
    update_keycode(KC_BSLS, 7, 7, !(opts & (1 << LAYOUT_BIT_ENTER_1)));
    update_keycode(KC_RGUI, 3, 6, opts & (1 << LAYOUT_BIT_SPLIT_SPACE));
    update_keycode(KC_NUBS, 0, 0, opts & (1 << LAYOUT_BIT_ISO_LEFT_SHIFT));
}

#endif
