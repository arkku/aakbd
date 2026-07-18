/**
 * via_handler.c: The Via/Vial communications protocol handler.
 *
 * This is basically what makes it possible to use the Vial app to
 * configure AAKBD keyboards.
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

#include "generic_hid.h"

// Vial requires the generic HID endpoint with output enabled.
#if !ENABLE_GENERIC_HID_ENDPOINT || !ENABLE_GENERIC_HID_OUTPUT
#error "VIAL_ENABLE requires ENABLE_GENERIC_HID_ENDPOINT=1 and ENABLE_GENERIC_HID_OUTPUT=1"
#endif

#include "via.h"
#include "vial.h"
#include "dynamic_keymap.h"
#include "aakbd.h"
#include "dynamic_storage.h"
#include <string.h>
#if VIAL_ENABLE_MATRIX_TESTER
#include "matrix.h"
#endif

#include "timer.h"

// Max data payload in a single HID response (32-byte report minus headers)
#define VIA_MAX_PAYLOAD 28

uint8_t
handle_generic_hid_report (uint8_t report_id, uint8_t count, uint8_t report[static count],
    uint8_t response_length[static 1], uint8_t response[static * response_length]) {
    if (count < 1) {
        return RESPONSE_ERROR;
    }

    uint8_t send_len = count < *response_length ? count : *response_length;
    memcpy(response, report, send_len);
    *response_length = send_len;

    uint8_t *const command_id = &response[0];
    uint8_t *const command_data = &response[1];

    const uint16_t offset = ((uint16_t) command_data[0] << 8) | command_data[1];
    const uint8_t size = (command_data[2] > VIA_MAX_PAYLOAD) ? VIA_MAX_PAYLOAD : command_data[2];

#if !defined(VIAL_INSECURE) || !VIAL_INSECURE
    // When unlock is in progress, only Vial-prefixed commands pass through
    // (those go to vial_handle_cmd via id_vial_prefix below).
    if (vial_is_unlock_in_progress() && *command_id != id_vial_prefix) {
        *command_id = id_unhandled;
        return RESPONSE_SEND_REPLY;
    }
#endif

    switch (*command_id) {
        case id_get_protocol_version:
            command_data[0] = (VIA_PROTOCOL_VERSION >> 8) & 0xFF;
            command_data[1] = VIA_PROTOCOL_VERSION & 0xFF;
            break;

        case id_get_keyboard_value:
            switch (command_data[0]) {
                case id_uptime: {
                    uint32_t value = timer_read32();
                    command_data[1] = (value >> 24) & 0xFF;
                    command_data[2] = (value >> 16) & 0xFF;
                    command_data[3] = (value >> 8) & 0xFF;
                    command_data[4] = value & 0xFF;
                    break;
                }
                case id_layout_options: {
                    uint16_t opts = dynamic_keymap_get_layout_options();
                    command_data[1] = 0;
                    command_data[2] = 0;
                    command_data[3] = (opts >> 8) & 0xFF;
                    command_data[4] = opts & 0xFF;
                    break;
                }
                case id_switch_matrix_state:
#if !defined(VIAL_INSECURE) || !VIAL_INSECURE
                    if (!vial_unlocked) {
                        *command_id = id_unhandled;
                        break;
                    }
#endif
#if VIAL_ENABLE_MATRIX_TESTER
                    uint8_t mi = 1;
                    for (uint8_t row = 0; row < MATRIX_ROWS && mi < 29; ++row) {
                        matrix_row_t value = matrix_get_row(row);
#if (MATRIX_COLS > 24)
                        if (mi < 29) {
                            command_data[mi++] = (value >> 24) & 0xFF;
                        }
#endif
#if (MATRIX_COLS > 16)
                        if (mi < 29) {
                            command_data[mi++] = (value >> 16) & 0xFF;
                        }
#endif
#if (MATRIX_COLS > 8)
                        if (mi < 29) {
                            command_data[mi++] = (value >> 8) & 0xFF;
                        }
#endif
                        if (mi < 29) {
                            command_data[mi++] = value & 0xFF;
                        }
                    }
#endif
                    break;
            }
            break;

        case id_set_keyboard_value:
            switch (command_data[0]) {
                case id_layout_options: {
                    uint16_t old_opts = dynamic_keymap_get_layout_options();
                    uint32_t value = ((uint32_t) command_data[1] << 24)
                        | ((uint32_t) command_data[2] << 16) | ((uint32_t) command_data[3] << 8)
                        | command_data[4];
                    dynamic_keymap_set_layout_options((uint16_t) (value & 0xFFFF));
                    uint16_t new_opts = dynamic_keymap_get_layout_options();
                    if (old_opts != new_opts) {
                        dynamic_keymap_layout_updated(old_opts, new_opts);
                    }
                    break;
                }
            }
            break;

        case id_dynamic_keymap_get_keycode: {
            uint8_t layer = command_data[0];
            uint8_t row = command_data[1];
            uint8_t col = command_data[2];
            uint16_t keycode = dynamic_keymap_get_qmk_keycode(layer, row, col);
            command_data[3] = (keycode >> 8) & 0xFF;
            command_data[4] = keycode & 0xFF;
            break;
        }

        case id_dynamic_keymap_set_keycode: {
            uint8_t layer = command_data[0];
            uint8_t row = command_data[1];
            uint8_t col = command_data[2];
            if (layer >= VIAL_LAYER_COUNT || row >= MATRIX_ROWS || col >= MATRIX_COLS) {
                *command_id = id_unhandled;
                break;
            }
            uint16_t keycode = ((uint16_t) command_data[3] << 8) | command_data[4];
            dynamic_keymap_set_qmk_keycode(layer, row, col, keycode);
            break;
        }

        case id_dynamic_keymap_reset:
            dynamic_keymap_reset();
            break;

        case id_lighting_set_value:
        case id_lighting_get_value:
        case id_lighting_save:
            *command_id = id_unhandled;
            break;

        case id_eeprom_reset:
            eeconfig_init_via();
            break;

        case id_bootloader_jump:
#if !defined(VIAL_INSECURE) || !VIAL_INSECURE
            // Guarded by unlock when security is enabled
            if (!vial_unlocked) {
                *command_id = id_unhandled;
                break;
            }
#endif
            jump_to_bootloader();
            break;

#if VIAL_MACRO_COUNT > 0
        case id_dynamic_keymap_macro_get_count:
            command_data[0] = dynamic_keymap_macro_get_count();
            break;

        case id_dynamic_keymap_macro_get_buffer_size: {
            uint16_t size = dynamic_keymap_macro_get_buffer_size();
            command_data[0] = (size >> 8) & 0xFF;
            command_data[1] = size & 0xFF;
            break;
        }

        case id_dynamic_keymap_macro_get_buffer:
            dynamic_keymap_macro_get_buffer(offset, size, &command_data[3]);
            break;

        case id_dynamic_keymap_macro_set_buffer:
#if !defined(VIAL_INSECURE) || !VIAL_INSECURE
            if (!vial_unlocked) {
                *command_id = id_unhandled;
                break;
            }
#endif
            dynamic_keymap_macro_set_buffer(offset, size, &command_data[3]);
            break;

        case id_dynamic_keymap_macro_reset:
            dynamic_keymap_macro_reset();
            break;
#endif

        case id_dynamic_keymap_get_layer_count:
            command_data[0] = dynamic_keymap_get_layer_count();
            break;

        case id_vial_prefix:
            vial_handle_cmd(response, *response_length);
            break;

        case id_dynamic_keymap_get_buffer:
            dynamic_keymap_get_buffer(offset, size, &command_data[3]);
            break;

        case id_dynamic_keymap_set_buffer:
            if (offset + size <= VIAL_LAYER_COUNT * MATRIX_ROWS * MATRIX_COLS * 2) {
                dynamic_keymap_set_buffer(offset, size, &command_data[3]);
                break;
            }
            // fallthrough

        default:
            *command_id = id_unhandled;
            break;
    }

    return RESPONSE_SEND_REPLY;
}

// make_generic_hid_report is provided by the device layer (xwhatsit.c).
