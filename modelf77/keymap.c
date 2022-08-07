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

// List of parameters (define each to 0 or 1):
// - SPLIT_RIGHT_SHIFT
// - NON_HHKB_SPLIT_RIGHT_SHIFT
// - SPLIT_BACKSPACE
// - SHORT_SPACE (adds RGUI to the right of space)
// - SPLIT_ENTER (splits the non-US backslash key or the ISO hash key)
// - ISO_LAYOUT (sets defaults for the settings below)
//   - ISO_ENTER
//   - SPLIT_LEFT_SHIFT
//
// Note: To split Enter in both directions, use ISO_ENTER=0 and SPLIT_ENTER=1

#if SPLIT_RIGHT_SHIFT
#if NON_HHKB_SPLIT_RIGHT_SHIFT
#define K_RSF1  KC_APFN
#define K_RSF2  KC_RSFT
#else
#define K_RSF1  KC_RSFT
#define K_RSF2  KC_APFN
#endif
#else // ^ SPLIT_RIGHT_SHIFT
#define K_RSF1  KC_NO
#define K_RSF2  KC_RSFT
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

#if SPLIT_SPACE_RIGHT || SHORT_SPACE
#define K_SSPR  KC_RGUI
#else
#define K_SSPR  KC_NO
#endif

#if SPLIT_SPACE_LEFT
#error "Space split on left side is not implemented. Edit LAYOUT_all macro."
#endif

const uint8_t PROGMEM keymaps[1][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT_all(
        KC_ESC, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, KC_EQL, K_BSPL, KC_BSPC,    KC_P7,   KC_P8,   KC_P9,
        KC_TAB,    KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC, KC_RBRC, K_USBS,         KC_P4,   KC_P5,   KC_P6,
        KC_CAPS,      KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT, K_ISOH, KC_ENT,    KC_P1,   KC_P2,   KC_P3,
        KC_LSFT, K_LSFR, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, K_RSF1, K_RSF2,     KC_P0,   KC_UP,   KC_PDOT,
        KC_LCTRL, KC_LGUI, KC_LALT,                      KC_SPC,         K_SSPR, KC_RALT, KC_NLCK, KC_RCTRL,     KC_LEFT, KC_DOWN, KC_RIGHT
    )
};
