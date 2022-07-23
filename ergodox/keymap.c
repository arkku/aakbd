#include "progmem.h"
#include "ergodox_ez.h"
#include "device_keymap.h"

/// Note that AAKBD requires every key in the keymap to be unique. Also, the
/// base keymap doesn't support extended keycodes. So, this map defines the
/// keymap closely, but not exactly, matching the Ergodox Ez default layout.
/// See `keymap.h` for macros to easily refer to the Ergodox keys when
/// reassigning them. The `template_layers.c` in this directory also does
/// the key assignments as per Ergodox default.
const uint8_t keymaps[1][MATRIX_ROWS][MATRIX_COLS] PROGMEM = { [0] = LAYOUT_ergodox_pretty(
  /* Left hand ***********************************************       Right hand *************************************************/
  KC_EQL,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,  KC_F1,    /**/ KC_F5,   KC_6,   KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS,
  KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,  KC_LCTL,  /**/ KC_RCTL, KC_Y,   KC_U,    KC_I,    KC_O,    KC_P,    KC_BSLS,
  KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,  /* ^ */   /**/ /* ^ */  KC_H,   KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,
  KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,  KC_APFN,  /**/ KC_INS,  KC_N,   KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,
  KC_GRV,  KC_INT1, KC_LALT, KC_LEFT, KC_RGHT,        /* ^ */   /**/ /* ^ */  KC_UP,  KC_DOWN, KC_LBRC, KC_RBRC, KC_SLCK,
  /* Left hand ***********************************************       Right hand *************************************************/
                                             KC_APP,  KC_LGUI,  /**/ KC_RALT, KC_ESC,
                                    /* _ */  /* _ */  KC_HOME,  /**/ KC_PGUP, /* _ */ /* _ */
                                    KC_SPC,  KC_BSPC, KC_END,   /**/ KC_PGDN, KC_DEL, KC_ENT
) };
