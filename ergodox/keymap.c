/**
 * keymap.c: Base keymap for Ergodox Ez.
 *
 * Each position must have a unique plain USB HID keycode. Dual-role and
 * layer behaviour is configured in `layers.c` / `layers_vial.c` (or via
 * Vial GUI). The physical-to-USB mapping can be reconfigured by editing
 * `config.h` (KC_R{row}C{col}{hand} aliases) or by defining CUSTOM_KEYMAP.
 */
#include "progmem.h"
#include "config.h"
#if VIAL_ENABLE
#include "vial_config.h"
#include "vial.h"
#endif
#include "ergodox_ez.h"
#include "device_keymap.h"

const uint8_t keymaps[1][MATRIX_ROWS][MATRIX_COLS] PROGMEM = { [0] = LAYOUT_ergodox_pretty(
  /* Left hand ***********************************************       Right hand *************************************************/
  KC_R1C1L, KC_R1C2L, KC_R1C3L, KC_R1C4L, KC_R1C5L, KC_R1C6L, KC_R1C7L,  /**/ KC_R1C1R, KC_R1C2R, KC_R1C3R, KC_R1C4R, KC_R1C5R, KC_R1C6R, KC_R1C7R,
  KC_R2C1L, KC_R2C2L, KC_R2C3L, KC_R2C4L, KC_R2C5L, KC_R2C6L, KC_R2C7L,  /**/ KC_R2C1R, KC_R2C2R, KC_R2C3R, KC_R2C4R, KC_R2C5R, KC_R2C6R, KC_R2C7R,
  KC_R3C1L, KC_R3C2L, KC_R3C3L, KC_R3C4L, KC_R3C5L, KC_R3C6L, /* ^ */   /**/ /* ^ */  KC_R3C2R, KC_R3C3R, KC_R3C4R, KC_R3C5R, KC_R3C6R, KC_R3C7R,
  KC_R4C1L, KC_R4C2L, KC_R4C3L, KC_R4C4L, KC_R4C5L, KC_R4C6L, KC_R4C7L,  /**/ KC_R4C1R, KC_R4C2R, KC_R4C3R, KC_R4C4R, KC_R4C5R, KC_R4C6R, KC_R4C7R,
  KC_R5C1L, KC_R5C2L, KC_R5C3L, KC_R5C4L, KC_R5C5L, /* ^ */             /**/ /* ^ */           KC_R5C3R, KC_R5C4R, KC_R5C5R, KC_R5C6R, KC_R5C7R,
  /* Left hand ***********************************************       Right hand *************************************************/
                                               KC_R6C1L, KC_R6C2L,  /**/ KC_R6C1R, KC_R6C2R,
                                                          KC_R7C1L,  /**/ KC_R7C1R,
                                          KC_R8C1L, KC_R8C2L, KC_R8C3L,  /**/ KC_R8C1R, KC_R8C2R, KC_R8C3R
) };

#if VIAL_ENABLE

const uint16_t vial_default_layout_options PROGMEM = 0;
const uint8_t vial_keyboard_uid[8] PROGMEM = { 0x01, 0x45, 0x5A, 0x41, 0x41, 0x4B, 0x42, 0x44 };

const uint8_t PROGMEM vial_unlock_combo_rows[] = { 7, 13 };
const uint8_t PROGMEM vial_unlock_combo_cols[] = { 0, 0 };
const uint8_t vial_unlock_combo_len = 2;

#endif
