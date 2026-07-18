/**
 * qmk_translate.c: Translation between AAKBD and QMK keycodes.
 *
 * Copyright (c) 2026 Kimmo Kulovesi, https://arkku.dev/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>

#include "usbkbd_config.h"
#include "dynamic_keymap.h"
#include "keycodes.h"
#include "progmem.h"
#include "qmk_keycodes.h"
#include "qmk_translate.h"
#include "usb_keys.h"

static uint16_t
aakbd_media_from_qmk (uint16_t qmk_key) {
#if ENABLE_MEDIA_KEYS
#if MEDIA_KEYS_COUNT <= 8
    switch (qmk_key) {
        case KC_AUDIO_MUTE:
            return USB_KEY_VOLUME_MUTE;
        case KC_AUDIO_VOL_UP:
            return USB_KEY_VOLUME_UP;
        case KC_AUDIO_VOL_DOWN:
            return USB_KEY_VOLUME_DOWN;
        case KC_MEDIA_NEXT_TRACK:
            return USB_KEY_NEXT_TRACK;
        case KC_MEDIA_PREV_TRACK:
            return USB_KEY_PREVIOUS_TRACK;
        case KC_MEDIA_PLAY_PAUSE:
            return USB_KEY_PLAY_PAUSE;
        case KC_MEDIA_FAST_FORWARD:
            return USB_KEY_FAST_FORWARD;
#if MEDIA_KEYS_COUNT >= 8
        case KC_MEDIA_REWIND:
            return USB_KEY_REWIND;
#else
        case KC_MEDIA_REWIND:
            return USB_KEY_PREVIOUS_TRACK;
#endif
        default:
            return EXTENDED(QMK_KEYCODE);
    }
#else // ^ MEDIA_KEYS_COUNT <= 8
    if (qmk_key >= KC_AUDIO_MUTE && qmk_key <= KC_BRIGHTNESS_DOWN) {
        uint8_t idx = qmk_key - KC_AUDIO_MUTE;
        if (idx > 7) {
            --idx;
        }
        if (idx == 7) {
            return EXTENDED(QMK_KEYCODE);
        }
        return (uint16_t) (USB_KEY_VIRTUAL_MEDIA_1 + idx);
    }
#endif
#endif
    return EXTENDED(QMK_KEYCODE);
}

static uint16_t
qmk_media_from_aakbd (uint16_t aakbd_key) {
#if ENABLE_MEDIA_KEYS
#if MEDIA_KEYS_COUNT <= 8
    switch (aakbd_key) {
        case USB_KEY_VOLUME_MUTE:
            return KC_AUDIO_MUTE;
        case USB_KEY_VOLUME_UP:
            return KC_AUDIO_VOL_UP;
        case USB_KEY_VOLUME_DOWN:
            return KC_AUDIO_VOL_DOWN;
        case USB_KEY_NEXT_TRACK:
            return KC_MEDIA_NEXT_TRACK;
        case USB_KEY_PREVIOUS_TRACK:
            return KC_MEDIA_PREV_TRACK;
        case USB_KEY_PLAY_PAUSE:
            return KC_MEDIA_PLAY_PAUSE;
        case USB_KEY_FAST_FORWARD:
            return KC_MEDIA_FAST_FORWARD;
#if MEDIA_KEYS_COUNT >= 8
        case USB_KEY_REWIND:
            return KC_MEDIA_REWIND;
#endif
        default:
            return KC_NO;
    }
#else
    if (aakbd_key >= USB_KEY_VIRTUAL_MEDIA_1 && aakbd_key < USB_KEY_VIRTUAL_MEDIA_1 + MEDIA_KEYS_COUNT) {
        uint8_t idx = aakbd_key - USB_KEY_VIRTUAL_MEDIA_1;
        if (idx >= 7) {
            ++idx;
        }
        return KC_AUDIO_MUTE + idx;
    }
    return KC_NO;
#endif
#else
    return KC_NO;
#endif
}

uint16_t
aakbd_to_qmk (keycode_t aakbd_keycode) {
    uint16_t kc = aakbd_keycode;

    if (kc == PASS) {
        return KC_TRNS;
    }
    if (kc == NONE) {
        return KC_NO;
    }

    if (kc == USB_KEY_VIRTUAL_APPLE_FN) {
        return VIAL_FN_MACRO;
    }

    uint16_t qmk_media = qmk_media_from_aakbd(kc);
    if (qmk_media != KC_NO) {
        return qmk_media;
    }

    if (!is_extended_keycode(kc)) {
        return kc;
    }

    const uint8_t command = COMMAND_OF(kc);
    const uint8_t key = PLAIN_KEY_OF(kc);
    const uint8_t mods = MODS_OF_EXTENDED(kc);

    if (command) {
        if (command == CMD_MODIFIER_OR_KEY) {
            if (!MODIFIERS_OF_EXTENDED(kc)) {
                return OSM(MODIFIERS_TO_MODS(key));
            }
            if (key > PASS && key <= PLAIN_KEY_MAX) {
                return key | MODS_TO_CMD(mods);
            }
            return KC_NO;
        }

        if (command == CMD_LAYER_OR_KEY) {
            if (key > PASS && key <= PLAIN_KEY_MAX) {
                return LT(LAYER_OF_LAYER_OR_KEY(kc) - 1, key);
            }
            return KC_TRNS;
        }

        uint8_t layer = LAYER_OF_COMMAND(kc);

        if (!layer) {
            return KC_NO;
        }
        --layer; // QMK layers start from zero, AAKBD from 1

        const uint8_t action = LAYER_CMD_MODIFIER_OF(kc);

        switch (command) {
            case CMD_LAYER_TOGGLE:
                // QMK doesn't really have exact matches, but
                if (action == ACT_ON_HOLD_KEEP_IF_NO_KEYPRESS) {
                    return TT(layer);
                }
                // fallthrough
            case CMD_LAYER_ENABLE:
                if (action == ACT_ON_HOLD) {
                    if (mods) {
                        return LM(layer, mods);
                    }
                    return MO(layer);
                }
                if (action == ACT_ON_PRESS || action == ACT_ON_RELEASE) {
                    return TG(layer);
                }
                if (action == ACT_ON_HOLD_KEEP_IF_NO_KEYPRESS) {
                    // QMK has no exact match but OSL is approximate
                    return OSL(layer);
                }
                if (action == ACT_ONESHOT) {
                    return OSL(layer);
                }
                break;
            case CMD_LAYER_DISABLE:
                if (action == ACT_ON_PRESS || action == ACT_ON_RELEASE) {
                    return TG(layer);
                }
                if (action == ACT_ONESHOT) {
                    return OSL(layer);
                }
                break;
            case CMD_LAYER_SET_MASK:
                if (action == ACT_ON_PRESS || action == ACT_ON_RELEASE) {
                    return TO(layer);
                }
                break;
            case CMD_LAYER_SET_BASE:
                if (action == ACT_ON_PRESS || action == ACT_ON_RELEASE) {
                    return DF(layer);
                }
                break;
            default:
                break;
        }
        return KC_NO;
    }

    if (mods & 0x0FU) {
        if (key && key <= PLAIN_KEY_MAX) {
            return kc; // QMK happens to have the same keycode for this
        }
        return LM(0, mods);
    }

    if (extended_keycode_is_macro(kc)) {
        const uint8_t macro_num = MACRO_OF_EXTENDED(kc);
        if (macro_num >= VIAL_MACRO_START) {
            return MACRO00 + (macro_num - VIAL_MACRO_START);
        }
        if (macro_num < VIAL_USER_MACRO_COUNT) {
            return VIAL_USER_MACRO_FIRST + macro_num;
        }
        return KC_NO;
    }

    if (extended_keycode_is_exact_modifiers(kc)) {
        return KC_NO;
    }

    switch (key) {
        case EXT_RESET_KEYBOARD:
            return QK_REBOOT;
        case EXT_ENTER_BOOTLOADER:
            return QK_BOOTLOADER;
        case EXT_EEPROM_RESET:
            return QK_CLEAR_EEPROM;
        case EXT_GRAVE_ESCAPE:
            return QK_GRAVE_ESCAPE;
#if ENABLE_TRI_LAYER
        case EXT_LAYER_2_4:
            return FN_MO13;
        case EXT_LAYER_3_4:
            return FN_MO23;
#endif
        case EXT_VIAL_APPLE_FN:
            return VIAL_FN_MACRO;
#if ENABLE_SPACE_CADET
        case EXT_SC_LEFT_CTRL_PARENTHESIS_OPEN:
            return SC_LCPO;
        case EXT_SC_RIGHT_CTRL_PARENTHESIS_CLOSE:
            return SC_RCPC;
        case EXT_SC_LEFT_SHIFT_PARENTHESIS_OPEN:
            return SC_LSPO;
        case EXT_SC_RIGHT_SHIFT_PARENTHESIS_CLOSE:
            return SC_RSPC;
        case EXT_SC_LEFT_ALT_PARENTHESIS_OPEN:
            return SC_LAPO;
        case EXT_SC_RIGHT_ALT_PARENTHESIS_CLOSE:
            return SC_RAPC;
        case EXT_SC_RIGHT_SHIFT_ENTER:
            return SC_SENT;
#endif
        default:
            break;
    }

    return KC_NO;
}

// The maximum QMK layer number supported that survives translation both ways
#define MAX_LAYER 30

keycode_t
qmk_to_aakbd (uint16_t qmk_keycode) {
    uint16_t kc = qmk_keycode;

    if (kc >= QK_CONSUMER_MIN && kc <= QK_CONSUMER_MAX) {
        return aakbd_media_from_qmk(kc);
    } else if (IS_QK_BASIC(kc)) {
        if (kc == KC_NO) {
            return NONE;
        }
        if (kc == KC_TRNS) {
            return PASS;
        }
        if (kc <= PLAIN_KEY_MAX) {
            return kc;
        }
    } else if (IS_QK_MOMENTARY(kc)) {
        uint8_t qmk_layer = QK_MOMENTARY_GET_LAYER(kc);
        if (qmk_layer <= MAX_LAYER) {
            return (uint16_t) LAYER_COMMAND(TOGGLE, ON_HOLD, qmk_layer + 1);
        }
    } else if (IS_QK_MODS(kc)) {
        uint8_t key = QK_MODS_GET_BASIC_KEYCODE(kc);
        uint8_t qmk_mods = ((kc >> 8) & 0x0FU) | ((kc & EXTENDED_KEY_BIT) ? 0x10U : 0);
        if (key >= 1 && key <= PLAIN_KEY_MAX) {
            return MODS_TO_CMD(qmk_mods) | key;
        }
    } else if (IS_QK_MOD_TAP(kc)) {
        uint8_t key = QK_MOD_TAP_GET_TAP_KEYCODE(kc);
        return MOD_OR_KEY(MODS_TO_CMD(QK_MOD_TAP_GET_MODS(kc)) | key);
    } else if (IS_QK_TOGGLE_LAYER(kc)) {
        uint8_t qmk_layer = QK_TOGGLE_LAYER_GET_LAYER(kc);
        if (qmk_layer <= MAX_LAYER) {
            return (uint16_t) LAYER_COMMAND(TOGGLE, ON_PRESS, qmk_layer + 1);
        }
    } else if (IS_QK_LAYER_MOD(kc)) {
        uint8_t raw_mods = QK_LAYER_MOD_GET_MODS(kc);
        uint8_t qmk_layer = QK_LAYER_MOD_GET_LAYER(kc);
        if (raw_mods && qmk_layer <= MAX_LAYER) {
            return (uint16_t) (LAYER_COMMAND(ENABLE, ON_HOLD, qmk_layer + 1) | MODS_TO_CMD(raw_mods));
        }
        if (raw_mods) {
            return MODS_TO_CMD(raw_mods);
        }
    } else if (kc == VIAL_FN_MACRO) {
        return EXTENDED(VIAL_APPLE_FN);
    } else if (kc >= VIAL_USER_MACRO_FIRST && kc <= VIAL_USER_MACRO_LAST) {
        return MACRO(kc - VIAL_USER_MACRO_FIRST);
    } else if (IS_QK_ONE_SHOT_LAYER(kc)) {
        uint8_t qmk_layer = QK_ONE_SHOT_LAYER_GET_LAYER(kc);
        if (qmk_layer <= MAX_LAYER) {
            return (uint16_t) LAYER_COMMAND(ENABLE, ONESHOT, qmk_layer + 1);
        }
    } else if (IS_QK_ONE_SHOT_MOD(kc)) {
        uint8_t qmk_mods = QK_ONE_SHOT_MOD_GET_MODS(kc);
        uint8_t mods = MODS_TO_MODIFIERS(qmk_mods & (QK_OSM_LEFT_MASK | QK_OSM_RIGHT_BIT));
        if (mods) {
            return (uint16_t) ((CMD_MODIFIER_OR_KEY << 13) | mods);
        }
    } else if (IS_QK_DEF_LAYER(kc)) {
        uint8_t qmk_layer = kc & 0x1F;
        if (qmk_layer <= MAX_LAYER) {
            return (uint16_t) LAYER_COMMAND(SET_BASE, ON_PRESS, qmk_layer + 1);
        }
    } else if (IS_QK_PDF(kc)) {
        // PDF (persistent default layer) falls through to EXTENDED(QMK_KEYCODE)
        // and is handled by vial_process_qmk_keycode with EEPROM persistence.
    } else if (IS_QK_TO(kc)) {
        uint8_t qmk_layer = QK_TO_GET_LAYER(kc);
        if (qmk_layer <= MAX_LAYER) {
            return (uint16_t) LAYER_COMMAND(SET_MASK, ON_PRESS, qmk_layer + 1);
        }
    } else if (IS_QK_LAYER_TAP_TOGGLE(kc)) {
        uint8_t qmk_layer = QK_LAYER_TAP_TOGGLE_GET_LAYER(kc);
        if (qmk_layer <= MAX_LAYER) {
            return (uint16_t) LAYER_COMMAND(TOGGLE, ON_HOLD_KEEP_IF_NO_KEYPRESS, qmk_layer + 1);
        }
    } else if (IS_QK_LAYER_TAP(kc)) {
        uint8_t key = QK_LAYER_TAP_GET_TAP_KEYCODE(kc);
        uint8_t qmk_layer = QK_LAYER_TAP_GET_LAYER(kc);
        if (qmk_layer <= MAX_LAYER && key >= 1 && key <= PLAIN_KEY_MAX) {
            return (uint16_t) LAYER_OR_PLAIN_KEY(qmk_layer + 1, key);
        }
    }
#if VIAL_MACRO_COUNT > 0
    else if (IS_QK_MACRO(kc)) {
        uint8_t n = kc - MACRO00;
        if (n + VIAL_MACRO_START <= MAX_AAKBD_MACRO) {
            return MACRO(n + VIAL_MACRO_START);
        }
    }
#endif
#if ENABLE_SPACE_CADET
    else if (IS_QK_SPACE_CADET(kc)) {
        switch (kc) {
            case SC_LCPO:
                return EXTENDED(SC_LEFT_CTRL_PARENTHESIS_OPEN);
            case SC_RCPC:
                return EXTENDED(SC_RIGHT_CTRL_PARENTHESIS_CLOSE);
            case SC_LSPO:
                return EXTENDED(SC_LEFT_SHIFT_PARENTHESIS_OPEN);
            case SC_RSPC:
                return EXTENDED(SC_RIGHT_SHIFT_PARENTHESIS_CLOSE);
            case SC_LAPO:
                return EXTENDED(SC_LEFT_ALT_PARENTHESIS_OPEN);
            case SC_RAPC:
                return EXTENDED(SC_RIGHT_ALT_PARENTHESIS_CLOSE);
            case SC_SENT:
                return EXTENDED(SC_RIGHT_SHIFT_ENTER);
        }
    }
#endif

    switch (kc) {
        case QK_BOOTLOADER:
            return EXTENDED(ENTER_BOOTLOADER);
        case QK_REBOOT:
            return EXTENDED(RESET_KEYBOARD);
        case QK_CLEAR_EEPROM:
            return EXTENDED(EEPROM_RESET);
        case QK_GRAVE_ESCAPE:
            return KEY_EXT_GRAVE_ESCAPE;
#if ENABLE_TRI_LAYER
        case FN_MO13:
            return EXTENDED(LAYER_2_4);
        case FN_MO23:
            return EXTENDED(LAYER_3_4);
#endif
        default:
            break;
    }

    return EXTENDED(QMK_KEYCODE);
}
