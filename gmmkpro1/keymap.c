/**
 * keymap.c: GMMK Pro default keymap (ISO and ANSI).
 */

#include "progmem.h"
#include "gmmkpro1.h"
#include "usb_keys.h"
#include "keymap.h"

#if ISO_LAYOUT

const uint8_t keymaps[1][MATRIX_ROWS][MATRIX_COLS] PROGMEM = {
    [0] = LAYOUT(
        KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_PSCR,          KC_SLCK,
        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC,          KC_DEL,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC,                   KC_INS,
        KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_NUHS, KC_ENT,           KC_PGUP,
        KC_LSFT, KC_NUBS, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,          KC_RSFT, KC_UP,   KC_PGDN,
        KC_LCTL, KC_LGUI, KC_LALT,                            KC_SPC,                             KC_RALT, KC_RCTL, KC_APFN, KC_LEFT, KC_DOWN, KC_RGHT
    )
};

#else // ANSI:

const uint8_t keymaps[1][MATRIX_ROWS][MATRIX_COLS] PROGMEM = {
    [0] = LAYOUT(
        KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_PSCR,          KC_SLCK,
        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC,          KC_DEL,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS,          KC_INS,
        KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,          KC_ENT,           KC_PGUP,
        KC_LSFT,          KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,          KC_RSFT, KC_UP,   KC_PGDN,
        KC_LCTL, KC_LGUI, KC_LALT,                            KC_SPC,                             KC_RALT, KC_RCTL, KC_APFN, KC_LEFT, KC_DOWN, KC_RGHT
    )
};
#endif

#if VIAL_ENABLE
#include "vial/vial.h"

const uint16_t vial_default_layout_options PROGMEM = 0;

const uint8_t PROGMEM vial_unlock_combo_rows[] = { 1, 10 };
const uint8_t PROGMEM vial_unlock_combo_cols[] = { 3, 4 };
const uint8_t vial_unlock_combo_len = 2;

const uint8_t vial_keyboard_uid[8] PROGMEM = {
    0x01, 0x47, 0x50, 0x41,
#if ISO_LAYOUT
    0x41,
#else
    0x61,
#endif
    0x4B, 0x42, 0x44
};
#endif
