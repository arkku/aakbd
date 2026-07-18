/**
 * vial.c: Vial/QMK compatibility layer for AAKBD.
 *
 * This implements the communication with Vial GUI without actually being Vial.
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

#include "vial.h"
#include "vial_keys.h"
#include "dynamic_keymap.h"
#include "qmk_keycodes.h"
#include "qmk_translate.h"
#include "vial_magic.h"
#include "progmem.h"

#define USB_KEYBOARD_ACCESS_STATE 1
#include "keys.h"
#include "usbkbd.h"
#include "aakbd.h"

#include <string.h>
#include "dynamic_storage.h"

// Canary value written to settings_canary in vial_qmk_settings on init.
// If EEPROM layout changes (different LAYER_COUNT, COMBO_COUNT, etc.),
// the canary address shifts and won't match, forcing reinit.
#define VIAL_QMK_SETTINGS_CANARY 0xDEAD

#if ENABLE_PS2_DEVICE
#include "ps2_output.h"
#endif

uint8_t current_10ms_tick_count(void);

// Keyboard-specific Vial configuration.
// Include via path: modelf77/vial_config.h (if exists) overrides vial/vial_config.h
#include "vial_config.h"

#include <stddef.h>

extern const uint8_t keyboard_definition[] PROGMEM;
extern const uint16_t keyboard_definition_size PROGMEM;

// Supported QMK setting IDs (from vial-qmk)
#define QS_ID_GRAVE_ESC_OVERRIDE 1
#define QS_ID_COMBO_TERM         2
#define QS_ID_OSK_TAP_TOGGLE     5
#define QS_ID_OSK_TIMEOUT        6
#define QS_ID_TAPPING_TERM       7
#define QS_ID_AUTO_SHIFT_FLAGS   3
#define QS_ID_AUTO_SHIFT_TIMEOUT 4
// Bit positions match Vial QMK qmk_settings.h precisely.
#define ASF_ENABLE         ((uint8_t) (1u << 0)) // QS_auto_shift_enable
#define ASF_MODIFIERS      ((uint8_t) (1u << 1)) // QS_auto_shift_modifiers
#define ASF_NO_SPECIAL     ((uint8_t) (1u << 2)) // QS_auto_shift_no_auto_shift_special
#define ASF_NO_NUMERIC     ((uint8_t) (1u << 3)) // QS_auto_shift_no_auto_shift_numeric
#define ASF_NO_ALPHA       ((uint8_t) (1u << 4)) // QS_auto_shift_no_auto_shift_alpha
#define ASF_AUTOREPEAT     ((uint8_t) (1u << 5)) // QS_auto_shift_repeat — hold after timeout
#define ASF_NO_AUTO_REPEAT ((uint8_t) (1u << 6)) // QS_auto_shift_no_auto_repeat — suppress hold

// EEPROM layout: Vial data starts after EECONFIG (which may include
// calibration data on xwhatsit keyboards with CAPSENSE_CAL_AUTOSAVE=1).
// EECONFIG_SIZE must be defined by the platform (eeconfig.h).

#ifndef TESTING
#include "matrix.h"
#include "timer.h"
#if HAPTIC_ENABLE
#include "haptic.h"
#include "solenoid.h"
#endif
#else
// Test build: mocks provide these
uint16_t timer_read(void);
uint16_t timer_elapsed(uint16_t last);
#endif

uint16_t vial_tap_hold_timeout_ms = VIAL_TAP_HOLD_TIMEOUT_MS;

#if ENABLE_AUTOSHIFT
#ifndef AUTOSHIFT_TIMEOUT_MS
/// Delay before auto-shift kicks in (ms).
#define AUTOSHIFT_TIMEOUT_MS 150
#endif
#ifndef AUTOSHIFT_ADJUST_INCREMENT_MS
#define AUTOSHIFT_ADJUST_INCREMENT_MS 5
#endif
#ifndef AUTOSHIFT_MAX_TIMEOUT_MS
#define AUTOSHIFT_MAX_TIMEOUT_MS 5000
#endif

uint16_t autoshift_timeout_ms = AUTOSHIFT_TIMEOUT_MS;
uint8_t autoshift_flags = 0;
#define autoshift_enabled (autoshift_flags & ASF_ENABLE)
#endif

#if !defined(VIAL_INSECURE) || !VIAL_INSECURE
// Vial unlock combo — MUST be defined per-device in keymap.c.
// Example: modelf77/keymap.c defines { 4, 2 } rows and { 0, 7 } cols, len=2.
extern const uint8_t PROGMEM vial_unlock_combo_rows[];
extern const uint8_t PROGMEM vial_unlock_combo_cols[];
extern const uint8_t vial_unlock_combo_len;

static uint8_t vial_unlock_in_progress = 0;
static uint16_t vial_unlock_timer = 0;
static uint8_t vial_unlock_counter = 0;
uint8_t vial_unlocked = 0;

bool
vial_is_unlock_in_progress (void) {
    return vial_unlock_in_progress;
}
#endif

#define GRAVE_ESC_OVERRIDE_BIT_ALT   1
#define GRAVE_ESC_OVERRIDE_BIT_CTRL  2
#define GRAVE_ESC_OVERRIDE_BIT_GUI   4
#define GRAVE_ESC_OVERRIDE_BIT_SHIFT 8

static uint8_t
grave_esc_mods_from_qmk (uint8_t qmk_bits) {
    uint8_t mask = 0;
    if (qmk_bits & GRAVE_ESC_OVERRIDE_BIT_ALT) {
        mask |= ALT_BIT | ALTGR_BIT;
    }
    if (qmk_bits & GRAVE_ESC_OVERRIDE_BIT_CTRL) {
        mask |= CTRL_BIT | RIGHT_CTRL_BIT;
    }
    if (qmk_bits & GRAVE_ESC_OVERRIDE_BIT_GUI) {
        mask |= CMD_BIT | RIGHT_CMD_BIT;
    }
    if (qmk_bits & GRAVE_ESC_OVERRIDE_BIT_SHIFT) {
        mask |= SHIFT_BIT | RIGHT_SHIFT_BIT;
    }
    return mask;
}

static uint32_t
via_eeprom_magic (void) {
    uint32_t m = pgm_read_word(&vial_keyboard_uid[2]);
    m |= (uint32_t) (((pgm_read_byte(&vial_keyboard_uid[1]) & 0x0F) << 4)
             | (pgm_read_byte(&vial_keyboard_uid[0]) & 0x0F))
        << 16;
    return m;
}

static bool
via_eeprom_is_valid (void) {
    const uint32_t stored = eeprom_read_word(VIA_EEPROM_MAGIC_ADDR)
        | ((uint32_t) eeprom_read_byte((uint8_t *) VIA_EEPROM_MAGIC_ADDR + 2) << 16);
    return stored == via_eeprom_magic() && eeprom_read_word(VIAL_QMK_SETTINGS_ADDR) == VIAL_QMK_SETTINGS_CANARY;
}

static void
via_eeprom_set_valid (bool valid) {
    const uint32_t magic = valid ? via_eeprom_magic() : 0xFFFFFFU;
    eeprom_update_word(VIA_EEPROM_MAGIC_ADDR, magic & 0xFFFFU);
    eeprom_update_byte((uint8_t *) VIA_EEPROM_MAGIC_ADDR + 2, (magic >> 16) & 0xFF);
    if (valid) {
        eeprom_update_word(VIAL_QMK_SETTINGS_ADDR, VIAL_QMK_SETTINGS_CANARY);
    }
}

void
vial_init (void) {
    if (!eeconfig_is_enabled()) {
        eeconfig_init();
    }
    if (!via_eeprom_is_valid()) {
        eeprom_update_word(VIAL_MAGIC_EEPROM_ADDR, 0);
        eeprom_update_word(VIAL_QMK_SETTINGS_ADDR, VIAL_QMK_SETTINGS_CANARY);
        eeprom_update_byte(VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, tap_toggle),
            ONESHOT_TAP_TOGGLE);
        eeprom_update_byte(VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, timeout_div_20),
            ONESHOT_TIMEOUT_MS / 20);
        eeprom_update_word(VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, combo_term),
            VIAL_COMBO_TIMEOUT_MS);
        eeprom_update_word(VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, tapping_term),
            VIAL_TAP_HOLD_TIMEOUT_MS);
        eeprom_update_byte(
            VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, grave_esc_override), 0);
        grave_esc_override_mask = 0;
#if ENABLE_AUTOSHIFT
        eeprom_update_word(
            VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, autoshift_timeout),
            AUTOSHIFT_TIMEOUT_MS);
        eeprom_update_byte(
            VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, autoshift_flags), 0);
#endif
        dynamic_keymap_set_layout_options(pgm_read_word(&vial_default_layout_options));
        dynamic_keymap_reset();
#if VIAL_MACRO_COUNT > 0
        dynamic_keymap_macro_reset();
#endif
        via_eeprom_set_valid(true);
    }

    uint8_t tap_toggle = eeprom_read_byte(VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, tap_toggle));
    if (tap_toggle > 0 && tap_toggle <= 10) {
        oneshot_tap_toggle = tap_toggle;
    }
    uint8_t timeout_raw = eeprom_read_byte(VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, timeout_div_20));
    if (timeout_raw) {
        oneshot_timeout_ms = (uint16_t) timeout_raw * 20;
        if (oneshot_timeout_ms > ONESHOT_TIMEOUT_MS_MAX) {
            oneshot_timeout_ms = ONESHOT_TIMEOUT_MS_MAX;
        }
    }
#if VIAL_COMBO_COUNT > 0
    const uint16_t combo_term = eeprom_read_word(VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, combo_term));
    if (combo_term) {
        vial_combo_timeout_ms = combo_term;
    }
#endif
    const uint16_t tap_term = eeprom_read_word(VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, tapping_term));
    if (tap_term) {
        vial_tap_hold_timeout_ms = tap_term;
    }
    const uint8_t grave_esc_qmk = eeprom_read_byte(VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, grave_esc_override));
    grave_esc_override_mask = grave_esc_mods_from_qmk(grave_esc_qmk);
#if ENABLE_AUTOSHIFT
    const uint16_t autoshift = eeprom_read_word(
        VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, autoshift_timeout));
    if (autoshift) {
        autoshift_timeout_ms = autoshift;
    }
    autoshift_flags = eeprom_read_byte(VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, autoshift_flags));
#endif

    // Load persistent base layer (PDF)
    const uint8_t saved_base = eeconfig_read_default_layer();
    if (saved_base && saved_base <= vial_total_layer_count()) {
        set_base_layer(saved_base);
    }
}

static void
cmd_get_keyboard_id (uint8_t data[static VIAL_RAW_EPSIZE], uint8_t length) {
    memset(data, 0, length);
    data[0] = VIAL_PROTOCOL_VERSION & 0xFF;
    data[1] = (VIAL_PROTOCOL_VERSION >> 8) & 0xFF;
    data[2] = (VIAL_PROTOCOL_VERSION >> 16) & 0xFF;
    data[3] = (VIAL_PROTOCOL_VERSION >> 24) & 0xFF;
    memcpy_P(&data[4], vial_keyboard_uid, 8);
}

static void
cmd_get_size (uint8_t data[static VIAL_RAW_EPSIZE]) {
    const uint16_t size = pgm_read_word(&keyboard_definition_size);
    data[0] = size & 0xFF;
    data[1] = (size >> 8) & 0xFF;
    data[2] = 0;
    data[3] = 0;
}

static void
cmd_get_def (const uint16_t page, uint8_t data[static VIAL_RAW_EPSIZE]) {
    const uint16_t start = page * VIAL_RAW_EPSIZE;
    uint16_t end = start + VIAL_RAW_EPSIZE;

    const uint16_t size = pgm_read_word(&keyboard_definition_size);
    if (end <= start || start >= size) {
        return;
    }
    if (end > size) {
        end = size;
    }
    memcpy_P(data, &keyboard_definition[start], end - start);
}

void
vial_handle_cmd (uint8_t data[static VIAL_RAW_EPSIZE], uint8_t length) {
    if (length != VIAL_RAW_EPSIZE) {
        return;
    }

    switch (data[1]) {
        case vial_get_keyboard_id:
            cmd_get_keyboard_id(data, length);
            break;

        case vial_get_size:
            cmd_get_size(data);
            break;

        case vial_get_def:
            cmd_get_def((((uint16_t) data[3]) << 8) | data[2], data);
            break;

#if !defined(VIAL_INSECURE) || !VIAL_INSECURE
        case vial_get_unlock_status:
            memset(data, 0xFF, length);
            data[0] = vial_unlocked;
            data[1] = vial_unlock_in_progress;
            for (uint8_t i = 0; i < vial_unlock_combo_len && i < 14; ++i) {
                data[2 + i * 2] = pgm_read_byte(&vial_unlock_combo_rows[i]);
                data[3 + i * 2] = pgm_read_byte(&vial_unlock_combo_cols[i]);
            }
            break;

        case vial_unlock_start:
            vial_unlock_in_progress = 1;
            vial_unlock_counter = 50;
            vial_unlock_timer = timer_read();
            break;

        case vial_unlock_poll:
            if (vial_unlock_in_progress && timer_elapsed(vial_unlock_timer) >= 100) {
                vial_unlock_timer = timer_read();
                bool all_held = true;
                for (uint8_t i = 0; i < vial_unlock_combo_len; ++i) {
                    uint8_t r = pgm_read_byte(&vial_unlock_combo_rows[i]);
                    uint8_t c = pgm_read_byte(&vial_unlock_combo_cols[i]);
                    if (!matrix_is_on(r, c)) {
                        all_held = false;
                        break;
                    }
                }
                if (all_held) {
                    if (vial_unlock_counter == 0 || --vial_unlock_counter == 0) {
                        vial_unlocked = 1;
                        vial_unlock_in_progress = 0;
                    }
                } else {
                    vial_unlock_counter = 50;
                }
            }
            data[0] = vial_unlocked;
            data[1] = vial_unlock_in_progress;
            data[2] = vial_unlock_counter;
            break;

        case vial_lock:
            vial_unlocked = 0;
            vial_unlock_in_progress = 0;
            break;
#else
        case vial_get_unlock_status:
            memset(data, 0xFF, length);
            data[0] = vial_unlocked;
            data[1] = 0;
            break;

        case vial_unlock_start:
        case vial_unlock_poll:
        case vial_lock:
            break;
#endif

#if NUM_ENCODERS > 0
        case vial_get_encoder: {
            uint16_t keycode = dynamic_keymap_get_encoder(data[2], data[3], false);
            data[0] = keycode >> 8;
            data[1] = keycode & 0xFF;
            keycode = dynamic_keymap_get_encoder(data[2], data[3], true);
            data[2] = keycode >> 8;
            data[3] = keycode & 0xFF;
            break;
        }

        case vial_set_encoder: {
            const uint16_t keycode = ((uint16_t) data[5] << 8) | data[6];
            dynamic_keymap_set_encoder(data[2], data[3], data[4], keycode);
            break;
        }
#endif

        case vial_dynamic_entry_op:
            switch (data[2]) {
#if VIAL_TAP_DANCE_COUNT > 0
                case dynamic_vial_tap_dance_get: {
                    uint8_t idx = data[3];
                    vial_tap_dance_entry_t entry = { 0 };
                    data[0] = (uint8_t) dynamic_keymap_get_tap_dance(idx, &entry);
                    memcpy(&data[1], &entry, sizeof(entry));
                    break;
                }

                case dynamic_vial_tap_dance_set: {
                    uint8_t idx = data[3];
                    vial_tap_dance_entry_t entry;
                    memcpy(&entry, &data[4], sizeof(entry));
                    data[0] = (uint8_t) dynamic_keymap_set_tap_dance(idx, &entry);
                    break;
                }
#endif
#if VIAL_COMBO_COUNT > 0
                case dynamic_vial_combo_get: {
                    uint8_t idx = data[3];
                    vial_combo_entry_t entry = { { 0 }, 0 };
                    data[0] = (uint8_t) dynamic_keymap_get_combo(idx, &entry);
                    memcpy(&data[1], &entry, sizeof(entry));
                    break;
                }

                case dynamic_vial_combo_set: {
                    uint8_t idx = data[3];
                    vial_combo_entry_t entry;
                    memcpy(&entry, &data[4], sizeof(entry));
                    data[0] = (uint8_t) dynamic_keymap_set_combo(idx, &entry);
                    break;
                }
#endif
                default: {
                    const bool is_counts = (data[2] == dynamic_vial_get_number_of_entries);
                    memset(data, 0, length);
                    if (is_counts) {
                        data[0] = VIAL_TAP_DANCE_COUNT;
                        data[1] = VIAL_COMBO_COUNT;
                    }
                    break;
                }
            }
            break;

        case vial_qmk_settings_query: {
            // Return setting IDs greater than the filter (data[2-3] = qsid_gt)
            uint16_t qsid_gt = data[2] | ((uint16_t) data[3] << 8);
            uint8_t out_idx = 2; // start after command header
            static const uint16_t supported[] PROGMEM = {
                QS_ID_GRAVE_ESC_OVERRIDE,
                QS_ID_COMBO_TERM,
                QS_ID_OSK_TAP_TOGGLE,
                QS_ID_OSK_TIMEOUT,
                QS_ID_TAPPING_TERM,
#if ENABLE_AUTOSHIFT
                QS_ID_AUTO_SHIFT_FLAGS,
                QS_ID_AUTO_SHIFT_TIMEOUT,
#endif
            };
            for (uint8_t i = 0; i < sizeof(supported) / sizeof(supported[0]); ++i) {
                if (out_idx + 2 > length) {
                    break;
                }
                uint16_t id = pgm_read_word(&supported[i]);
                if (id > qsid_gt) {
                    data[out_idx++] = id & 0xFF;
                    data[out_idx++] = (id >> 8) & 0xFF;
                }
            }
            // Terminator
            if (out_idx + 2 <= length) {
                data[out_idx++] = 0xFF;
                data[out_idx++] = 0xFF;
            }
            break;
        }

        case vial_qmk_settings_get: {
            uint16_t qsid = data[2] | ((uint16_t) data[3] << 8);
            switch (qsid) {
                case QS_ID_GRAVE_ESC_OVERRIDE: {
                    uint8_t val = eeprom_read_byte(VIAL_QMK_SETTINGS_ADDR
                        + offsetof(struct vial_qmk_settings, grave_esc_override));
                    data[0] = 0;   // success
                    data[1] = val; // stored in QMK format already
                    break;
                }
                case QS_ID_OSK_TAP_TOGGLE: {
                    uint8_t val = eeprom_read_byte(
                        VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, tap_toggle));
                    if (val == 0 || val > 10) {
                        val = ONESHOT_TAP_TOGGLE;
                    }
                    data[0] = 0; // success
                    data[1] = val;
                    break;
                }
                case QS_ID_OSK_TIMEOUT: {
                    uint8_t raw = eeprom_read_byte(
                        VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, timeout_div_20));
                    uint16_t val = (uint16_t) raw * 20U;
                    data[0] = 0; // success
                    data[1] = val & 0xFF;
                    data[2] = (val >> 8) & 0xFF;
                    break;
                }
                case QS_ID_COMBO_TERM: {
                    uint16_t val = eeprom_read_word(
                        VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, combo_term));
#if VIAL_COMBO_COUNT > 0
                    if (!val) {
                        val = vial_combo_timeout_ms;
                    }
#endif
                    data[0] = 0;
                    data[1] = val & 0xFF;
                    data[2] = (val >> 8) & 0xFF;
                    break;
                }
                case QS_ID_TAPPING_TERM: {
                    uint16_t val = eeprom_read_word(
                        VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, tapping_term));
                    if (!val) {
                        val = vial_tap_hold_timeout_ms;
                    }
                    data[0] = 0;
                    data[1] = val & 0xFF;
                    data[2] = (val >> 8) & 0xFF;
                    break;
                }
#if ENABLE_AUTOSHIFT
                case QS_ID_AUTO_SHIFT_FLAGS: {
                    data[0] = 0;
                    data[1] = autoshift_flags;
                    break;
                }
                case QS_ID_AUTO_SHIFT_TIMEOUT: {
                    uint16_t val = eeprom_read_word(
                        VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, autoshift_timeout));
                    if (!val) {
                        val = autoshift_timeout_ms;
                    }
                    data[0] = 0;
                    data[1] = val & 0xFF;
                    data[2] = (val >> 8) & 0xFF;
                    break;
                }
#endif
                default:
                    data[0] = 1; // error
                    break;
            }
            break;
        }

        case vial_qmk_settings_set: {
            if (length < 5) {
                break;
            }
            uint16_t qsid = data[2] | ((uint16_t) data[3] << 8);
            switch (qsid) {
                case QS_ID_OSK_TAP_TOGGLE:
                    if (data[4] >= 2 && data[4] <= 10) {
                        eeprom_update_byte(
                            VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, tap_toggle),
                            data[4]);
                        oneshot_tap_toggle = data[4];
                        if (data[4] <= 1) {
                            oneshot_tap_count = 0;
                        }
                        data[0] = 0;
                    } else {
                        data[0] = 1;
                    }
                    break;
                case QS_ID_OSK_TIMEOUT: {
                    if (length < 6) {
                        break;
                    }
                    uint16_t val = data[4] | ((uint16_t) data[5] << 8);
                    if (val >= 20 && val <= 5100) {
                        if (val > ONESHOT_TIMEOUT_MS_MAX) {
                            val = ONESHOT_TIMEOUT_MS_MAX;
                        }
                        eeprom_update_byte(
                            VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, timeout_div_20),
                            val / 20);
                        oneshot_timeout_ms = val;
                    } else if (val == 0) {
                        eeprom_update_byte(
                            VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, timeout_div_20),
                            0);
                        oneshot_timeout_ms = 0;
                    } else {
                        data[0] = 1;
                        break;
                    }
                    data[0] = 0;
                    break;
                }
                case QS_ID_GRAVE_ESC_OVERRIDE: {
                    if (length < 5) {
                        break;
                    }
                    uint8_t qmk_val = data[4] & 0x0F;
                    eeprom_update_byte(
                        VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, grave_esc_override),
                        qmk_val);
                    grave_esc_override_mask = grave_esc_mods_from_qmk(qmk_val);
                    data[0] = 0;
                    break;
                }
                case QS_ID_COMBO_TERM: {
                    if (length < 6) {
                        break;
                    }
                    uint16_t val = data[4] | ((uint16_t) data[5] << 8);
                    eeprom_update_word(
                        VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, combo_term), val);
#if VIAL_COMBO_COUNT > 0
                    vial_combo_timeout_ms = val;
#endif
                    data[0] = 0;
                    break;
                }
                case QS_ID_TAPPING_TERM: {
                    if (length < 6) {
                        break;
                    }
                    uint16_t val = data[4] | ((uint16_t) data[5] << 8);
                    eeprom_update_word(
                        VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, tapping_term), val);
                    vial_tap_hold_timeout_ms = val;
                    data[0] = 0;
                    break;
                }
#if ENABLE_AUTOSHIFT
                case QS_ID_AUTO_SHIFT_FLAGS: {
                    if (length < 5) {
                        break;
                    }
                    autoshift_flags = data[4];
                    eeprom_update_byte(
                        VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, autoshift_flags),
                        autoshift_flags);
                    data[0] = 0;
                    break;
                }
                case QS_ID_AUTO_SHIFT_TIMEOUT: {
                    if (length < 6) {
                        break;
                    }
                    uint16_t val = data[4] | ((uint16_t) data[5] << 8);
                    if (val < AUTOSHIFT_ADJUST_INCREMENT_MS) {
                        val = AUTOSHIFT_TIMEOUT_MS;
                    }
                    if (val > AUTOSHIFT_MAX_TIMEOUT_MS) {
                        val = AUTOSHIFT_MAX_TIMEOUT_MS;
                    }
                    eeprom_update_word(
                        VIAL_QMK_SETTINGS_ADDR + offsetof(struct vial_qmk_settings, autoshift_timeout),
                        val);
                    autoshift_timeout_ms = val;
                    data[0] = 0;
                    break;
                }
#endif
                default:
                    data[0] = 1; // error
                    break;
            }
            break;
        }

        case vial_qmk_settings_reset:
            // Restore defaults by clearing EEPROM bytes and resetting runtime values
            for (uint8_t i = 0; i < VIAL_QMK_SETTINGS_SIZE; ++i) {
                eeprom_update_byte(VIAL_QMK_SETTINGS_ADDR + i, 0);
            }
            oneshot_tap_toggle = ONESHOT_TAP_TOGGLE;
            oneshot_timeout_ms = ONESHOT_TIMEOUT_MS;
            if (oneshot_timeout_ms > ONESHOT_TIMEOUT_MS_MAX) {
                oneshot_timeout_ms = ONESHOT_TIMEOUT_MS_MAX;
            }
#if VIAL_COMBO_COUNT > 0
            vial_combo_timeout_ms = VIAL_COMBO_TIMEOUT_MS;
#endif
            vial_tap_hold_timeout_ms = VIAL_TAP_HOLD_TIMEOUT_MS;
            grave_esc_override_mask = 0;
            break;

        default:
            break;
    }
}

void
eeconfig_init_via (void) {
    via_eeprom_set_valid(false);
    vial_init();
}
