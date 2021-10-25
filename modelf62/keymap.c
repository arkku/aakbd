#include <stdint.h>
#include <avr/pgmspace.h>
#include "keymap.h"
#include "wcass.h"

#define _BASE   0
#define _FN     KC_APFN
#define MO(fn)  (fn)

// Note: Most of these keymaps are untested copypastes. I have also moved the
// caps lock and left ctrl to the "normal" positions (i.e., caps lock above
// left shift). The swap is easy to do in layers.c, this way you don't need to
// wonder if it is already swapped here.
//
// Keep in mind that the keymaps here _must_ have each physical key mapped to a
// unique keycode! It is highly recommended to remap the keys in layers.c
// rather than editing these keymaps. That being said, if your keys all have
// unique keycaps, it may make sense to edit the layout here to match those.
// But keep in mind that this file is included in the Git repository, so
// updating that may overwrite your local changes.

const uint8_t PROGMEM keymaps[1][MATRIX_ROWS][MATRIX_COLS] = {
#if ISO_LAYOUT
#if SPLIT_RIGHT_SHIFT
#if SPLIT_BACKSPACE
    // ISO, split right shift, split backspace
    [_BASE] = LAYOUT_iso_hhkb_split_shift_split_backspace(
        KC_ESC, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, KC_EQL, KC_GRV, KC_BSPC,
        KC_TAB,    KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC, KC_RBRC, KC_ENT,
        KC_CAPS,      KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT, KC_NUHS,
        KC_LSFT, KC_NUBS, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT, KC_APFN,
        KC_LCTRL, KC_LGUI, KC_LALT,                      KC_SPC,                  KC_RALT, KC_NLCK, KC_RCTRL
    )
#else // ^ SPLIT_BACKSPACE
#if NON_HHKB_SPLIT_RIGHT_SHIFT
#undef NON_HHKB_SPLIT_RIGHT_SHIFT
    // ISO, split right shift (NON-HHKB), regular backspace
    [_BASE] = LAYOUT_iso_nonhhkb_split_shift_regular_backspace(
        KC_ESC, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, KC_EQL, KC_BSPC,
        KC_TAB,    KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC, KC_RBRC, KC_ENT,
        KC_CAPS,     KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT, KC_NONUS_HASH,
        KC_LSFT, KC_NUBS, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_APFN, KC_RSFT,
        KC_LCTRL, KC_LGUI, KC_LALT,                      KC_SPC,                  KC_RALT, KC_NLCK, KC_RCTRL
    ),
#else
    // ISO, split right shift, regular backspace
    [_BASE] = LAYOUT_iso_hhkb_split_shift_regular_backspace(
        KC_ESC, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, KC_EQL, KC_BSPC,
        KC_TAB,    KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC, KC_RBRC, KC_ENT,
        KC_CAPS,      KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT, KC_NUHS,
        KC_LSFT, KC_NUBS, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT, KC_APFN,
        KC_LCTL, KC_LGUI, KC_LALT,                      KC_SPC,                  KC_RALT, KC_NLCK, KC_RCTRL
    ),
#endif
#endif
#else // ^ SPLIT_RIGHT_SHIFT
#if SPLIT_BACKSPACE
    // ISO, regular right shift, split backspace
    [_BASE] = LAYOUT_iso_regular_shift_split_backspace(
        KC_ESC, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, KC_EQL, KC_GRV, KC_BSPC,
        KC_TAB,    KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC, KC_RBRC, KC_ENT,
        KC_CAPS,      KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT, KC_NUHS,
        KC_LSFT, KC_NUBS, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT,
        KC_LCTRL, KC_LGUI, KC_LALT,                      KC_SPC,                  KC_RALT, KC_NLCK, KC_RCTRL
    ),
#else
    // ISO, regular right shift, regular backspace
    [_BASE] = LAYOUT_iso_regular_shift_regular_backspace(
        KC_ESC, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, KC_EQL, KC_BSPC,
        KC_TAB,    KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC, KC_RBRC, KC_ENT,
        KC_CAPS,      KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT, KC_NUHS,
        KC_LSFT, KC_NUBS, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT,
        KC_LCTRL, KC_LGUI, KC_LALT,                      KC_SPC,                  KC_RALT, KC_NLCK, KC_RCTRL
    ),
#endif
#endif
#else // ^ ISO_LAYOUT
#if SPLIT_RIGHT_SHIFT
#if SPLIT_BACKSPACE
    // ANSI, split shift, split backspace
    [_BASE] = LAYOUT_ansi_hhkb_split_shift_split_backspace(
        KC_ESC, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, KC_EQL, KC_BSLS, KC_GRV,
        KC_TAB,    KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC, KC_RBRC, KC_BSPC,
        KC_CAPS,     KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT, KC_ENT,
        KC_LSFT,         KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT, KC_APFN,
        KC_LCTRL, KC_LGUI, KC_LALT,                      KC_SPC,                  KC_RALT, KC_NLCK, KC_RCTRL
    ),
#else
    // ANSI, split shift, regular backspace
    [_BASE] = LAYOUT_ansi_hhkb_split_shift_regular_backspace(
        KC_ESC, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, KC_EQL, KC_BSPC,
        KC_TAB,    KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC, KC_RBRC, KC_BSLS,
        KC_CAPS     KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT, KC_ENT,
        KC_LSFT,         KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT, KC_APFN,
        KC_LCTRL, KC_LGUI, KC_LALT,                      KC_SPC,                  KC_RALT, KC_NLCK, KC_RCTRL
    ),
#endif
#else // ^ SPLIT_RIGHT_SHIFT
#if SPLIT_BACKSPACE
    // ANSI, regular shift, split backspace
    [_BASE] = LAYOUT_ansi_regular_shift_split_backspace(
        KC_ESC, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, KC_EQL, KC_BSLS, KC_GRV,
        KC_TAB,    KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC, KC_RBRC, KC_BSPC,
        KC_CAPS,      KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT, KC_ENT,
        KC_LSFT,         KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT,
        KC_LCTRL, KC_LGUI, KC_LALT,                      KC_SPC,                  KC_RALT, KC_NLCK, KC_RCTRL
    ),
#else // ^ SPLIT_BACKSPACE
    // ANSI, regular shift, regular backspace
    [_BASE] = LAYOUT_ansi_regular_shift_regular_backspace(
        KC_ESC, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, KC_EQL, KC_BSPC,
        KC_TAB,    KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC, KC_RBRC, KC_BSLS,
        KC_CAPS,      KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT, KC_ENT,
        KC_LSFT,         KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT,
        KC_LCTRL, KC_LGUI, KC_LALT,                      KC_SPC,                  KC_RALT, KC_NLCK, KC_RCTRL
    ),
#endif
#endif
#endif // ^ ANSI_LAYOUT
};

#ifdef NON_HHKB_SPLIT_RIGHT_SHIFT
#error "Non-HHKB split right shift not defined for this combination."
#endif
