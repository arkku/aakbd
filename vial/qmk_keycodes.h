// Copyright 2025 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdint.h>
#include <keymap.h>

// These must match the Vial protocol specification used by the GUI.
// Always use these macros, never hardcode the values.

#define QK_BASIC                            0x0000
#define QK_BASIC_MAX                        0x00FF
#define QK_MODS                             0x0100
#define QK_MODS_MAX                         0x1FFF
#define QK_MOD_TAP                          0x2000
#define QK_MOD_TAP_MAX                      0x3FFF
#define QK_LAYER_TAP                        0x4000
#define QK_LAYER_TAP_MAX                    0x4FFF
#define QK_LAYER_MOD                        0x5000
#define QK_LAYER_MOD_MAX                    0x51FF
#define QK_TO                               0x5200
#define QK_TO_MAX                           0x521F
#define QK_MOMENTARY                        0x5220
#define QK_MOMENTARY_MAX                    0x523F
#define QK_DEF_LAYER                        0x5240
#define QK_DEF_LAYER_MAX                    0x525F
#define QK_TOGGLE_LAYER                     0x5260
#define QK_TOGGLE_LAYER_MAX                 0x527F
#define QK_ONE_SHOT_LAYER                   0x5280
#define QK_ONE_SHOT_LAYER_MAX               0x529F
#define QK_ONE_SHOT_MOD                     0x52A0
#define QK_ONE_SHOT_MOD_MAX                 0x52BF
#define QK_PERSISTENT_DEF_LAYER             0x52E0
#define QK_PERSISTENT_DEF_LAYER_MAX         0x52FF
#define QK_LAYER_TAP_TOGGLE                 0x52C0
#define QK_LAYER_TAP_TOGGLE_MAX             0x52DF
#define QK_SWAP_HANDS                       0x5600
#define QK_SWAP_HANDS_MAX                   0x56FF
#define QK_TAP_DANCE                        0x5700
#define QK_TAP_DANCE_MAX                    0x57FF
#define QK_MAGIC                            0x7000
#define QK_MAGIC_MAX                        0x70FF
#define QK_MAGIC_SWAP_CONTROL_CAPS_LOCK     0x7000
#define QK_MAGIC_UNSWAP_CONTROL_CAPS_LOCK   0x7001
#define QK_MAGIC_TOGGLE_CONTROL_CAPS_LOCK   0x7002
#define QK_MAGIC_CAPS_LOCK_AS_CONTROL_OFF   0x7003
#define QK_MAGIC_CAPS_LOCK_AS_CONTROL_ON    0x7004
#define QK_MAGIC_SWAP_LALT_LGUI             0x7005
#define QK_MAGIC_UNSWAP_LALT_LGUI           0x7006
#define QK_MAGIC_SWAP_RALT_RGUI             0x7007
#define QK_MAGIC_UNSWAP_RALT_RGUI           0x7008
#define QK_MAGIC_GUI_ON                     0x7009
#define QK_MAGIC_GUI_OFF                    0x700A
#define QK_MAGIC_TOGGLE_GUI                 0x700B
#define QK_MAGIC_SWAP_GRAVE_ESC             0x700C
#define QK_MAGIC_UNSWAP_GRAVE_ESC           0x700D
#define QK_MAGIC_SWAP_BACKSLASH_BACKSPACE   0x700E
#define QK_MAGIC_UNSWAP_BACKSLASH_BACKSPACE 0x700F
#define QK_MAGIC_TOGGLE_BACKSLASH_BACKSPACE 0x7010
#define QK_MAGIC_NKRO_ON                    0x7011
#define QK_MAGIC_NKRO_OFF                   0x7012
#define QK_MAGIC_TOGGLE_NKRO                0x7013
#define QK_MAGIC_SWAP_ALT_GUI               0x7014
#define QK_MAGIC_UNSWAP_ALT_GUI             0x7015
#define QK_MAGIC_TOGGLE_ALT_GUI             0x7016
#define QK_MAGIC_SWAP_LCTL_LGUI             0x7017
#define QK_MAGIC_UNSWAP_LCTL_LGUI           0x7018
#define QK_MAGIC_SWAP_RCTL_RGUI             0x7019
#define QK_MAGIC_UNSWAP_RCTL_RGUI           0x701A
#define QK_MAGIC_SWAP_CTL_GUI               0x701B
#define QK_MAGIC_UNSWAP_CTL_GUI             0x701C
#define QK_MAGIC_TOGGLE_CTL_GUI             0x701D
#define QK_MAGIC_EE_HANDS_LEFT              0x701E
#define QK_MAGIC_EE_HANDS_RIGHT             0x701F
#define QK_MAGIC_SWAP_ESCAPE_CAPS_LOCK      0x7020
#define QK_MAGIC_UNSWAP_ESCAPE_CAPS_LOCK    0x7021
#define QK_MAGIC_TOGGLE_ESCAPE_CAPS_LOCK    0x7022

// Auto-shift keycodes
#define QK_AUTO_SHIFT_DOWN          0x7C10
#define QK_AUTO_SHIFT_UP            0x7C11
#define QK_AUTO_SHIFT_REPORT        0x7C12
#define QK_AUTO_SHIFT_ON            0x7C13
#define QK_AUTO_SHIFT_OFF           0x7C14
#define QK_AUTO_SHIFT_TOGGLE        0x7C15
#define QK_MACRO                    0x7700
#define QK_MACRO_MAX                0x777F
#define QK_KB                       0x7E00
#define QK_KB_MAX                   0x7E3F
#define QK_USER                     0x7E40
#define QK_USER_MAX                 0x7FFF
#define QK_HAPTIC_ON                0x7C40
#define QK_HAPTIC_OFF               0x7C41
#define QK_HAPTIC_TOGGLE            0x7C42
#define QK_HAPTIC_RESET             0x7C43
#define QK_HAPTIC_FEEDBACK_TOGGLE   0x7C44
#define QK_HAPTIC_BUZZ_TOGGLE       0x7C45
#define QK_HAPTIC_MODE_NEXT         0x7C46
#define QK_HAPTIC_MODE_PREVIOUS     0x7C47
#define QK_HAPTIC_CONTINUOUS_TOGGLE 0x7C48
#define QK_HAPTIC_CONTINUOUS_UP     0x7C49
#define QK_HAPTIC_CONTINUOUS_DOWN   0x7C4A
#define QK_HAPTIC_DWELL_UP          0x7C4B
#define QK_HAPTIC_DWELL_DOWN        0x7C4C

#define IS_QK_HAPTIC(code) ((code) >= QK_HAPTIC_ON && (code) <= QK_HAPTIC_DWELL_DOWN)

#define QK_COMBO_ON     0x7C50
#define QK_COMBO_OFF    0x7C51
#define QK_COMBO_TOGGLE 0x7C52

#define IS_QK_COMBO(code) ((code) >= QK_COMBO_ON && (code) <= QK_COMBO_TOGGLE)

// Modifier combos — these are just QK_MODS / QK_MOD_TAP with specific modifiers
// Prefixed with QK_ to avoid collision with AAKBD's MEH/HYPER/etc. macros.
#define QK_MEH(kc)  (QK_LCTL | QK_LSFT | QK_LALT | (kc))
#define QK_HYPR(kc) (QK_LCTL | QK_LSFT | QK_LALT | QK_LGUI | (kc))
#define QK_LCAG(kc) (QK_LCTL | QK_LALT | QK_LGUI | (kc))

#define QK_MEH_T(kc)  MT(MOD_LCTL | MOD_LSFT | MOD_LALT, kc)
#define QK_HYPR_T(kc) MT(MOD_LCTL | MOD_LSFT | MOD_LALT | MOD_LGUI, kc)
#define QK_ALL_T(kc)  QK_HYPR_T(kc)
#define QK_LCAG_T(kc) MT(MOD_LCTL | MOD_LALT | MOD_LGUI, kc)
#define QK_RCAG_T(kc) MT(MOD_RCTL | MOD_RALT | MOD_RGUI, kc)

#define QK_SPACE_CADET_LEFT_CTRL_PARENTHESIS_OPEN    0x7C18
#define QK_SPACE_CADET_RIGHT_CTRL_PARENTHESIS_CLOSE  0x7C19
#define QK_SPACE_CADET_LEFT_SHIFT_PARENTHESIS_OPEN   0x7C1A
#define QK_SPACE_CADET_RIGHT_SHIFT_PARENTHESIS_CLOSE 0x7C1B
#define QK_SPACE_CADET_LEFT_ALT_PARENTHESIS_OPEN     0x7C1C
#define QK_SPACE_CADET_RIGHT_ALT_PARENTHESIS_CLOSE   0x7C1D
#define QK_SPACE_CADET_RIGHT_SHIFT_ENTER             0x7C1E

#define SC_LCPO QK_SPACE_CADET_LEFT_CTRL_PARENTHESIS_OPEN
#define SC_RCPC QK_SPACE_CADET_RIGHT_CTRL_PARENTHESIS_CLOSE
#define SC_LSPO QK_SPACE_CADET_LEFT_SHIFT_PARENTHESIS_OPEN
#define SC_RSPC QK_SPACE_CADET_RIGHT_SHIFT_PARENTHESIS_CLOSE
#define SC_LAPO QK_SPACE_CADET_LEFT_ALT_PARENTHESIS_OPEN
#define SC_RAPC QK_SPACE_CADET_RIGHT_ALT_PARENTHESIS_CLOSE
#define SC_SENT QK_SPACE_CADET_RIGHT_SHIFT_ENTER

#define QK_UNICODE     0x8000
#define QK_UNICODE_MAX 0xFFFF

// QMK modifier bit positions (same in v5 and v6)
#define QK_LCTL 0x0100
#define QK_LSFT 0x0200
#define QK_LALT 0x0400
#define QK_LGUI 0x0800
#define QK_RCTL 0x1100
#define QK_RSFT 0x1200
#define QK_RALT 0x1400
#define QK_RGUI 0x1800

// QMK keycode forming macros (from quantum_keycodes.h)
#define LCTL(kc)       (QK_LCTL | (kc))
#define LSFT(kc)       (QK_LSFT | (kc))
#define LALT(kc)       (QK_LALT | (kc))
#define LGUI(kc)       (QK_LGUI | (kc))
#define RCTL(kc)       (QK_RCTL | (kc))
#define RSFT(kc)       (QK_RSFT | (kc))
#define RALT(kc)       (QK_RALT | (kc))
#define RGUI(kc)       (QK_RGUI | (kc))
#define TO(layer)      (QK_TO | ((layer) & 0x1F))
#define MO(layer)      (QK_MOMENTARY | ((layer) & 0x1F))
#define DF(layer)      (QK_DEF_LAYER | ((layer) & 0x1F))
#define PDF(layer)     (QK_PERSISTENT_DEF_LAYER | ((layer) & 0x1F))
#define TG(layer)      (QK_TOGGLE_LAYER | ((layer) & 0x1F))
#define OSL(layer)     (QK_ONE_SHOT_LAYER | ((layer) & 0x1F))
#define OSM(mod)       (QK_ONE_SHOT_MOD | ((mod) & 0x1F))
#define TT(layer)      (QK_LAYER_TAP_TOGGLE | ((layer) & 0x1F))
#define LT(layer, kc)  (QK_LAYER_TAP | (((layer) & 0xF) << 8) | ((kc) & 0xFF))
#define MT(mod, kc)    (QK_MOD_TAP | (((mod) & 0x1F) << 8) | ((kc) & 0xFF))
#define LM(layer, mod) (QK_LAYER_MOD | (((layer) & 0xF) << 5) | ((mod) & 0x1F))

// Generic decoding for the whole QK_MODS range
#define QK_MODS_GET_MODS(kc)          (((kc) >> 8) & 0x1F)
#define QK_MODS_GET_BASIC_KEYCODE(kc) ((kc) & 0xFF)

// Getters for layer/tap/mod values from QMK keycode ranges
#define QK_TO_GET_LAYER(kc)               ((kc) & 0x1F)
#define QK_MOMENTARY_GET_LAYER(kc)        ((kc) & 0x1F)
#define QK_DEF_LAYER_GET_LAYER(kc)        ((kc) & 0x1F)
#define QK_TOGGLE_LAYER_GET_LAYER(kc)     ((kc) & 0x1F)
#define QK_ONE_SHOT_LAYER_GET_LAYER(kc)   ((kc) & 0x1F)
#define QK_ONE_SHOT_MOD_GET_MODS(kc)      ((kc) & 0x1F)
#define QK_LAYER_TAP_TOGGLE_GET_LAYER(kc) ((kc) & 0x1F)
#define QK_LAYER_TAP_GET_LAYER(kc)        (((kc) >> 8) & 0xF)
#define QK_LAYER_TAP_GET_TAP_KEYCODE(kc)  ((kc) & 0xFF)
#define QK_MOD_TAP_GET_MODS(kc)           (((kc) >> 8) & 0x1F)
#define QK_MOD_TAP_GET_TAP_KEYCODE(kc)    ((kc) & 0xFF)
#define QK_LAYER_MOD_GET_LAYER(kc)        (((kc) >> 5) & 0xF)
#define QK_LAYER_MOD_GET_MODS(kc)         ((kc) & 0x1F)

// QMK mod tap shortcuts (v6: QK_MOD_TAP + mod_bits << 8)
#define QK_MOD_TAP_LCTL 0x2100
#define QK_MOD_TAP_LSFT 0x2200
#define QK_MOD_TAP_LALT 0x2400
#define QK_MOD_TAP_LGUI 0x2800
#define QK_MOD_TAP_RCTL 0x3100
#define QK_MOD_TAP_RSFT 0x3200
#define QK_MOD_TAP_RALT 0x3400
#define QK_MOD_TAP_RGUI 0x3800

// QMK OSM modifier encoding (5-bit: bits 0-3 = left mod, bit 4 = right marker)
#define MOD_LCTL 0x01
#define MOD_LSFT 0x02
#define MOD_LALT 0x04
#define MOD_LGUI 0x08
#define MOD_RCTL 0x11
#define MOD_RSFT 0x12
#define MOD_RALT 0x14
#define MOD_RGUI 0x18

// OSM format helpers
#define QK_OSM_RIGHT_BIT 0x10
#define QK_OSM_LEFT_MASK 0x0F

// Range helpers (from vial-qmk-latest/quantum/keycodes.h)
#define IS_QK_BASIC(code)     ((code) <= QK_BASIC_MAX)
#define IS_QK_MODS(code)      ((code) >= QK_MODS && (code) <= QK_MODS_MAX)
#define IS_QK_MOD_TAP(code)   ((code) >= QK_MOD_TAP && (code) <= QK_MOD_TAP_MAX)
#define IS_QK_LAYER_TAP(code) ((code) >= QK_LAYER_TAP && (code) <= QK_LAYER_TAP_MAX)
#define IS_QK_LAYER_MOD(code) ((code) >= QK_LAYER_MOD && (code) <= QK_LAYER_MOD_MAX)
#define IS_QK_SPACE_CADET(code) \
    ((code) >= QK_SPACE_CADET_LEFT_CTRL_PARENTHESIS_OPEN && (code) <= QK_SPACE_CADET_RIGHT_SHIFT_ENTER)
#define IS_QK_TO(code)        ((code) >= QK_TO && (code) <= QK_TO_MAX)
#define IS_QK_MOMENTARY(code) ((code) >= QK_MOMENTARY && (code) <= QK_MOMENTARY_MAX)
#define IS_QK_DEF_LAYER(code) ((code) >= QK_DEF_LAYER && (code) <= QK_DEF_LAYER_MAX)
#define IS_QK_PDF(code) ((code) >= QK_PERSISTENT_DEF_LAYER && (code) <= QK_PERSISTENT_DEF_LAYER_MAX)
#define IS_QK_TOGGLE_LAYER(code)   ((code) >= QK_TOGGLE_LAYER && (code) <= QK_TOGGLE_LAYER_MAX)
#define IS_QK_ONE_SHOT_LAYER(code) ((code) >= QK_ONE_SHOT_LAYER && (code) <= QK_ONE_SHOT_LAYER_MAX)
#define IS_QK_ONE_SHOT_MOD(code)   ((code) >= QK_ONE_SHOT_MOD && (code) <= QK_ONE_SHOT_MOD_MAX)
#define IS_QK_LAYER_TAP_TOGGLE(code) \
    ((code) >= QK_LAYER_TAP_TOGGLE && (code) <= QK_LAYER_TAP_TOGGLE_MAX)
#define IS_QK_MACRO(code) ((code) >= QK_MACRO && (code) <= QK_MACRO_MAX)

// Special QMK keycodes
#define KC_TRNS         0x0001
#define QK_BOOTLOADER   0x7C00
#define QK_REBOOT       0x7C01
#define QK_CLEAR_EEPROM 0x7C03
#define QK_GRAVE_ESCAPE 0x7C16

// VIA keycodes (v6 values from vial-qmk-latest)
#define FN_MO13 0x7C77
#define FN_MO23 0x7C78
#define MACRO00 0x7700
#define MACRO01 0x7701
#define MACRO02 0x7702
#define MACRO03 0x7703
#define MACRO04 0x7704
#define MACRO05 0x7705
#define MACRO06 0x7706
#define MACRO07 0x7707
#define MACRO08 0x7708
#define MACRO09 0x7709
#define MACRO10 0x770A
#define MACRO11 0x770B
#define MACRO12 0x770C
#define MACRO13 0x770D
#define MACRO14 0x770E
#define MACRO15 0x770F

// User keycodes: Apple Fn → USER00 (QK_KB + 0 = 0x7E00)
// Macros → USER01..USER63 (computed up to QK_KB_MAX)
#define USER00                0x7E00
#define VIAL_FN_MACRO         USER00
#define VIAL_USER_MACRO_FIRST (USER00 + 1)
#define VIAL_USER_MACRO_COUNT 63
#define VIAL_USER_MACRO_LAST  (VIAL_USER_MACRO_FIRST + VIAL_USER_MACRO_COUNT - 1)

// QMK consumer keycodes (USB HID usage IDs, same in v5/v6)
#define QK_CONSUMER_MIN       0x00A5
#define QK_CONSUMER_MAX       0x00BE
#define KC_AUDIO_MUTE         0x00A8
#define KC_AUDIO_VOL_UP       0x00A9
#define KC_AUDIO_VOL_DOWN     0x00AA
#define KC_MEDIA_NEXT_TRACK   0x00AB
#define KC_MEDIA_PREV_TRACK   0x00AC
#define KC_MEDIA_STOP         0x00AD
#define KC_MEDIA_PLAY_PAUSE   0x00AE
#define KC_MEDIA_FAST_FORWARD 0x00BB
#define KC_MEDIA_REWIND       0x00BC
#define KC_BRIGHTNESS_UP      0x00BD
#define KC_BRIGHTNESS_DOWN    0x00BE
#define KC_MEDIA_SELECT       0x00AF
#define KC_MEDIA_EJECT        0x00B0
#define KC_MAIL               0x00B1
#define KC_CALCULATOR         0x00B2
#define KC_MY_COMPUTER        0x00B3
#define KC_WWW_SEARCH         0x00B4
#define KC_WWW_HOME           0x00B5
#define KC_WWW_BACK           0x00B6
#define KC_WWW_FORWARD        0x00B7
#define KC_WWW_STOP           0x00B8
#define KC_WWW_REFRESH        0x00B9
#define KC_WWW_FAVORITES      0x00BA

// RGB matrix control
#define QK_RGB_MATRIX_ON     0x7840
#define QK_RGB_MATRIX_OFF    0x7841
#define QK_RGB_MATRIX_TOGGLE 0x7842
