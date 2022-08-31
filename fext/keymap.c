#include <stdint.h>
#include "progmem.h"
#include "keymap.h"
#include "universal.h"

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

// List of parameters (define each to 0 or 1):
// - SPLIT_RIGHT_SHIFT
// - SPLIT_BACKSPACE
// - SPLIT_ENTER (splits the non-US backslash key or the ISO hash key)
// - ISO_LAYOUT (sets defaults for the settings below)
//   - ISO_ENTER
//   - SPLIT_LEFT_SHIFT
// - ADDED_GUI_KEYS (adds the Windows/GUI keys, requires modification)
//
// Note: To split Enter in both directions, use ISO_ENTER=0 and SPLIT_ENTER=1

#if SPLIT_RIGHT_SHIFT
#define K_RSFS  KC_APFN
#else // ^ SPLIT_RIGHT_SHIFT
#define K_RSFS  KC_NO
#endif

#if SPLIT_BACKSPACE
//#define K_BSPL  KC_INT3
#define K_BSPL  KC_GRV
#else
#define K_BSPL  KC_NO
#endif

#ifndef SPLIT_LEFT_SHIFT
#define SPLIT_LEFT_SHIFT ISO_LAYOUT
#endif
#ifndef ISO_ENTER
#ifndef BA_ENTER
#define BA_ENTER 0
#endif
#define ISO_ENTER (ISO_LAYOUT || BA_ENTER)
#endif

#if SPLIT_LEFT_SHIFT
#define K_LSFR  KC_NUBS
#else
#define K_LSFR  KC_NO
#endif

#if ISO_ENTER
#if SPLIT_ENTER
#define K_USBS  KC_BSLS
#else
#define K_USBS  KC_NO
#endif
#if BA_ENTER
#define K_ISOH  KC_NO
#else
#define K_ISOH  KC_NUHS
#endif
#else // ^ ISO_ENTER
#define K_USBS  KC_BSLS
#if SPLIT_ENTER
#define K_ISOH  KC_NUBS
#else
#define K_ISOH  KC_NO
#endif
#endif

#ifndef ADDED_GUI_KEYS
#define ADDED_GUI_KEYS 0
#endif

#if ADDED_GUI_KEYS
#define K_GUIL KC_LGUI
#define K_GUIR KC_RGUI
#else
#define K_GUIL KC_NO
#define K_GUIR KC_NO
#endif

const uint8_t PROGMEM keymaps[1][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT_all(
        KC_ESC,        KC_F1, KC_F2, KC_F3, KC_F4,    KC_F5, KC_F6, KC_F7, KC_F8,     KC_F9, KC_F10, KC_F11,   KC_F12,    KC_PSCR, KC_SLCK, KC_PAUS,
        KC_GRV, KC_1,  KC_2,  KC_3,  KC_4,  KC_5,  KC_6,  KC_7,  KC_8,  KC_9,  KC_0,  KC_MINS, KC_EQL, K_BSPL, KC_BSPC,   KC_INS,  KC_HOME, KC_PGUP,    KC_NLCK, KC_PSLS, KC_PAST, KC_PMNS,
        KC_TAB,    KC_Q,  KC_W,  KC_E,  KC_R,  KC_T,  KC_Y,  KC_U,  KC_I,  KC_O,  KC_P,  KC_LBRC, KC_RBRC,    K_USBS,     KC_DEL,  KC_END,  KC_PGDN,    KC_P7,   KC_P8,   KC_P9,   KC_PPLS,
        KC_CAPS,      KC_A,  KC_S,  KC_D,  KC_F,  KC_G,  KC_H,  KC_J,  KC_K,  KC_L,  KC_SCLN, KC_QUOT, K_ISOH,  KC_ENT,                                 KC_P4,   KC_P5,   KC_P6,   KC_NO,
        KC_LSFT, K_LSFR,  KC_Z,  KC_X,  KC_C,  KC_V,  KC_B,  KC_N,  KC_M,  KC_COMM, KC_DOT, KC_SLSH, K_RSFS,  KC_RSFT,              KC_UP,              KC_P1,   KC_P2,   KC_P3,   KC_PENT,
        KC_LCTRL, K_GUIL,  KC_LALT,              KC_SPC,                              KC_RALT,    K_GUIR,    KC_RCTRL,    KC_LEFT, KC_DOWN, KC_RGHT,    KC_NO,   KC_P0,   KC_PDOT, KC_NO
    )
};

