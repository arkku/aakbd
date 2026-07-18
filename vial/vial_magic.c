/**
 * vial_magic.c: "MAGIC" swaps (like Ctrl/Caps) AAKBD reimplementation.
 *
 * Natively AAKBD is supposed to implement key swaps with layers, e.g., if
 * you want to swap Alt and GUI on macOS but not Windows, make a layer that
 * swaps them and passes through all other keys, and have it auto-activate
 * when plugged in to a Mac. But for sake of completeness (and because it is
 * easy to do), this file implements support for the Vial/QMK magic swaps.
 * They are used by mapping the keys (from "Quantum" tab) and then pressing
 * those keys (which TBH, seems more effort than just doing the layer).
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

#include "vial_magic.h"
#include "dynamic_keymap.h"
#include "qmk_keycodes.h"
#include "progmem.h"
#include "usb_keys.h"
#include "usbkbd.h"

#include "dynamic_storage.h"
#include "dynamic_keymap.h"

static inline uint16_t
load_mask (void) {
    return eeprom_read_word(VIAL_MAGIC_EEPROM_ADDR);
}

static inline void
save_mask (uint16_t mask) {
    eeprom_update_word(VIAL_MAGIC_EEPROM_ADDR, mask);
}

uint16_t
vial_magic_load (void) {
    return load_mask();
}

void
vial_magic_save (uint16_t mask) {
    save_mask(mask);
}

bool
vial_magic_process_keycode (uint16_t qmk_keycode) {
    uint16_t mask = load_mask();
    bool changed = true;

    // Uses actual QMK keycode constants from qmk_keycodes.h
    switch (qmk_keycode) {
        case QK_MAGIC_SWAP_CONTROL_CAPS_LOCK:
            mask |= (1U << VIAL_MAGIC_BIT_SWAP_CTRL_CAPS);
            break;
        case QK_MAGIC_UNSWAP_CONTROL_CAPS_LOCK:
            mask &= ~(1U << VIAL_MAGIC_BIT_SWAP_CTRL_CAPS);
            break;
        case QK_MAGIC_TOGGLE_CONTROL_CAPS_LOCK:
            mask ^= (1U << VIAL_MAGIC_BIT_SWAP_CTRL_CAPS);
            break;

        case QK_MAGIC_CAPS_LOCK_AS_CONTROL_ON:
            mask |= (1U << VIAL_MAGIC_BIT_CAPS_TO_CTRL);
            break;
        case QK_MAGIC_CAPS_LOCK_AS_CONTROL_OFF:
            mask &= ~(1U << VIAL_MAGIC_BIT_CAPS_TO_CTRL);
            break;

        case QK_MAGIC_SWAP_LALT_LGUI:
            mask |= (1U << VIAL_MAGIC_BIT_SWAP_LALT_LGUI);
            break;
        case QK_MAGIC_UNSWAP_LALT_LGUI:
            mask &= ~(1U << VIAL_MAGIC_BIT_SWAP_LALT_LGUI);
            break;

        case QK_MAGIC_SWAP_RALT_RGUI:
            mask |= (1U << VIAL_MAGIC_BIT_SWAP_RALT_RGUI);
            break;
        case QK_MAGIC_UNSWAP_RALT_RGUI:
            mask &= ~(1U << VIAL_MAGIC_BIT_SWAP_RALT_RGUI);
            break;

        case QK_MAGIC_SWAP_ALT_GUI:
            mask |= (1U << VIAL_MAGIC_BIT_SWAP_LALT_LGUI) | (1U << VIAL_MAGIC_BIT_SWAP_RALT_RGUI);
            break;
        case QK_MAGIC_UNSWAP_ALT_GUI:
            mask &= ~((1U << VIAL_MAGIC_BIT_SWAP_LALT_LGUI) | (1U << VIAL_MAGIC_BIT_SWAP_RALT_RGUI));
            break;
        case QK_MAGIC_TOGGLE_ALT_GUI:
            // QMK: toggle LALT_LGUI, set RALT_RGUI to match
            mask ^= (1U << VIAL_MAGIC_BIT_SWAP_LALT_LGUI);
            if (mask & (1U << VIAL_MAGIC_BIT_SWAP_LALT_LGUI)) {
                mask |= (1U << VIAL_MAGIC_BIT_SWAP_RALT_RGUI);
            } else {
                mask &= ~(1U << VIAL_MAGIC_BIT_SWAP_RALT_RGUI);
            }
            break;

        case QK_MAGIC_SWAP_GRAVE_ESC:
            mask |= (1U << VIAL_MAGIC_BIT_SWAP_GRAVE_ESC);
            break;
        case QK_MAGIC_UNSWAP_GRAVE_ESC:
            mask &= ~(1U << VIAL_MAGIC_BIT_SWAP_GRAVE_ESC);
            break;

        case QK_MAGIC_SWAP_BACKSLASH_BACKSPACE:
            mask |= (1U << VIAL_MAGIC_BIT_SWAP_BSLASH_BSPACE);
            break;
        case QK_MAGIC_UNSWAP_BACKSLASH_BACKSPACE:
            mask &= ~(1U << VIAL_MAGIC_BIT_SWAP_BSLASH_BSPACE);
            break;
        case QK_MAGIC_TOGGLE_BACKSLASH_BACKSPACE:
            mask ^= (1U << VIAL_MAGIC_BIT_SWAP_BSLASH_BSPACE);
            break;

        case QK_MAGIC_GUI_OFF:
            mask |= (1U << VIAL_MAGIC_BIT_NO_GUI);
            break;
        case QK_MAGIC_GUI_ON:
            mask &= ~(1U << VIAL_MAGIC_BIT_NO_GUI);
            break;
        case QK_MAGIC_TOGGLE_GUI:
            mask ^= (1U << VIAL_MAGIC_BIT_NO_GUI);
            break;

        case QK_MAGIC_SWAP_LCTL_LGUI:
            mask |= (1U << VIAL_MAGIC_BIT_SWAP_LCTL_LGUI);
            break;
        case QK_MAGIC_UNSWAP_LCTL_LGUI:
            mask &= ~(1U << VIAL_MAGIC_BIT_SWAP_LCTL_LGUI);
            break;

        case QK_MAGIC_SWAP_RCTL_RGUI:
            mask |= (1U << VIAL_MAGIC_BIT_SWAP_RCTL_RGUI);
            break;
        case QK_MAGIC_UNSWAP_RCTL_RGUI:
            mask &= ~(1U << VIAL_MAGIC_BIT_SWAP_RCTL_RGUI);
            break;

        case QK_MAGIC_SWAP_CTL_GUI:
            mask |= (1U << VIAL_MAGIC_BIT_SWAP_LCTL_LGUI) | (1U << VIAL_MAGIC_BIT_SWAP_RCTL_RGUI);
            break;
        case QK_MAGIC_UNSWAP_CTL_GUI:
            mask &= ~((1U << VIAL_MAGIC_BIT_SWAP_LCTL_LGUI) | (1U << VIAL_MAGIC_BIT_SWAP_RCTL_RGUI));
            break;
        case QK_MAGIC_TOGGLE_CTL_GUI:
            // QMK: toggle LCTL_LGUI, set RCTL_RGUI to match
            mask ^= (1U << VIAL_MAGIC_BIT_SWAP_LCTL_LGUI);
            if (mask & (1U << VIAL_MAGIC_BIT_SWAP_LCTL_LGUI)) {
                mask |= (1U << VIAL_MAGIC_BIT_SWAP_RCTL_RGUI);
            } else {
                mask &= ~(1U << VIAL_MAGIC_BIT_SWAP_RCTL_RGUI);
            }
            break;

        case QK_MAGIC_SWAP_ESCAPE_CAPS_LOCK:
            mask |= (1U << VIAL_MAGIC_BIT_SWAP_ESC_CAPS);
            break;
        case QK_MAGIC_UNSWAP_ESCAPE_CAPS_LOCK:
            mask &= ~(1U << VIAL_MAGIC_BIT_SWAP_ESC_CAPS);
            break;
        case QK_MAGIC_TOGGLE_ESCAPE_CAPS_LOCK:
            mask ^= (1U << VIAL_MAGIC_BIT_SWAP_ESC_CAPS);
            break;

        case QK_MAGIC_TOGGLE_NKRO:
            // Toggle USB boot protocol (closest AAKBD equivalent to NKRO)
            changed = false; // handled via clear_keyboard + toggle below
            break;

        default:
            changed = false;
            break;
    }

    if (changed) {
        save_mask(mask);
        return true;
    }

    if (qmk_keycode == QK_MAGIC_TOGGLE_NKRO) {
        usb_keyboard_toggle_boot_protocol();
        return true;
    }

    return false;
}

uint8_t
vial_magic_remap_key (uint8_t physical_key) {
    uint16_t mask = vial_magic_load();

    if (!mask) {
        return physical_key;
    }

    // Two-way swaps: bit, key1 ↔ key2
    static const struct {
        uint8_t bit;
        uint8_t key1, key2;
    } swaps[] PROGMEM = {
        { VIAL_MAGIC_BIT_SWAP_CTRL_CAPS, USB_KEY_LEFT_CTRL, USB_KEY_CAPS_LOCK },
        { VIAL_MAGIC_BIT_SWAP_ESC_CAPS, USB_KEY_ESC, USB_KEY_CAPS_LOCK },
        { VIAL_MAGIC_BIT_SWAP_LALT_LGUI, USB_KEY_LEFT_ALT, USB_KEY_LEFT_GUI },
        { VIAL_MAGIC_BIT_SWAP_RALT_RGUI, USB_KEY_RIGHT_ALT, USB_KEY_RIGHT_GUI },
        { VIAL_MAGIC_BIT_SWAP_GRAVE_ESC, USB_KEY_GRAVE, USB_KEY_ESC },
        { VIAL_MAGIC_BIT_SWAP_BSLASH_BSPACE, USB_KEY_BACKSLASH, USB_KEY_BACKSPACE },
        { VIAL_MAGIC_BIT_SWAP_LCTL_LGUI, USB_KEY_LEFT_CTRL, USB_KEY_LEFT_GUI },
        { VIAL_MAGIC_BIT_SWAP_RCTL_RGUI, USB_KEY_RIGHT_CTRL, USB_KEY_RIGHT_GUI },
    };
    for (uint8_t i = 0; i < sizeof(swaps) / sizeof(swaps[0]); ++i) {
        if (mask & (1U << (uint16_t) pgm_read_byte(&swaps[i].bit))) {
            const uint8_t key1 = pgm_read_byte(&swaps[i].key1);
            const uint8_t key2 = pgm_read_byte(&swaps[i].key2);
            if (physical_key == key1) {
                return key2;
            }
            if (physical_key == key2) {
                return key1;
            }
        }
    }

    // One-way: Caps Lock → Left Ctrl
    if ((mask & (1U << VIAL_MAGIC_BIT_CAPS_TO_CTRL)) && physical_key == USB_KEY_CAPS_LOCK) {
        return USB_KEY_LEFT_CTRL;
    }

    // No GUI: block both GUI keys
    if ((mask & (1U << VIAL_MAGIC_BIT_NO_GUI))
        && (physical_key == USB_KEY_LEFT_GUI || physical_key == USB_KEY_RIGHT_GUI)) {
        return 0;
    }

    return physical_key;
}
