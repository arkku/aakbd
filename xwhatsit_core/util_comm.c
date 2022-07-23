/* Copyright 2020 Purdea Andrei
 * Copyright 2021 Kimmo Kulovesi <https://arkku.dev/>
 *
 * Ported from QMK to AAKBD. Any errors are almost certainly due to that.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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
#if ENABLE_GENERIC_HID_ENDPOINT
#include "quantum.h"
#include "util_comm.h"
#include "matrix_manipulate.h"
#include <string.h>
#include "avr/eeprom.h"
#include "generic_hid.h"

#if GENERIC_HID_REPORT_SIZE < 32
#error "GENERIC_HID_REPORT_SIZE is too small, see util_comm.c"
#endif

#define min(x, y) (((x) < (y))?(x):(y))

#define STRIFY(a)           #a
#define STR(a)              STRIFY(a)

#ifdef KEYBOARD_NAME
#define KEYBOARD_FILENAME (STR(KEYBOARD_NAME)".c")
#endif
extern matrix_row_t raw_matrix[MATRIX_ROWS];

static const uint8_t magic[] = UTIL_COMM_MAGIC;

uint8_t handle_generic_hid_report(uint8_t report_id, uint8_t count, uint8_t data[static count], uint8_t response_length[static 1], uint8_t response[static *response_length]) {
    if (0 != memcmp(data, magic, sizeof(magic))) {
        return RESPONSE_ERROR;
    }
    memcpy(response, magic, sizeof(magic));
    response[2] = UTIL_COMM_RESPONSE_ERROR;
    switch (data[2])
    {
        case UTIL_COMM_GET_VERSION:
            response[2] = UTIL_COMM_RESPONSE_OK;
            response[3] = UTIL_COMM_VERSION_MAJOR;
            response[4] = UTIL_COMM_VERSION_MID;
            response[5] = (UTIL_COMM_VERSION_MINOR >> 8) & 0xff;
            response[6] = (UTIL_COMM_VERSION_MINOR >> 0) & 0xff;
            *response_length = 7;
            break;
        case UTIL_COMM_DISABLE_KEYBOARD:
            response[2] = UTIL_COMM_RESPONSE_OK;
            response[3] = (uint8_t) keyboard_scan_enabled;
            keyboard_scan_enabled = 0;
            *response_length = 4;
            break;
        case UTIL_COMM_ENABLE_KEYBOARD:
            response[2] = UTIL_COMM_RESPONSE_OK;
            response[3] = (uint8_t) keyboard_scan_enabled;
            keyboard_scan_enabled = 1;
            *response_length = 4;
            break;
        case UTIL_COMM_ENTER_BOOTLOADER:
            keyboard_scan_enabled = 0;
            return RESPONSE_JUMP_TO_BOOTLOADER;
        case UTIL_COMM_GET_KEYSTATE:
            response[2] = UTIL_COMM_RESPONSE_OK;
            {
                char *current_matrix_ptr = (char *) raw_matrix;
                int offset = 0;
                if ((uint8_t) sizeof(raw_matrix) > *response_length - 3) {
                    offset = data[3];
                    current_matrix_ptr += offset;
                }
                *response_length = min(*response_length - 3, (uint8_t) sizeof(raw_matrix) - offset);
                memcpy(&response[3], current_matrix_ptr, *response_length);
                *response_length += 3;
            }
            break;
        case UTIL_COMM_GET_THRESHOLDS:
            response[2] = UTIL_COMM_RESPONSE_OK;
            #if CAPSENSE_CAL_ENABLED
            response[3] = CAPSENSE_CAL_BINS;
            {
                const uint8_t cal_bin = data[3];
                response[4] = cal_thresholds[cal_bin] & 0xff;
                response[5] = (cal_thresholds[cal_bin] >> 8) & 0xff;
                char *assigned_to_threshold_ptr = (char *)assigned_to_threshold[cal_bin];
                int offset = 0;
                if ((uint8_t) sizeof(assigned_to_threshold[cal_bin]) > *response_length - 6) {
                    offset = data[4];
                    assigned_to_threshold_ptr += offset;
                }
                *response_length = min(*response_length - 6, (uint8_t) sizeof(assigned_to_threshold[cal_bin]) - offset);
                memcpy(&response[6], assigned_to_threshold_ptr, *response_length);
                *response_length += 6;
            }
            #else
            response[3] = 0;
            response[4] = (CAPSENSE_HARDCODED_THRESHOLD) & 0xff;
            response[5] = ((CAPSENSE_HARDCODED_THRESHOLD) >> 8) & 0xff;
            *response_length = 6;
            #endif
            break;
        case UTIL_COMM_GET_KEYBOARD_FILENAME:
            {
                int string_length = sizeof(KEYBOARD_FILENAME);
                response[2] = UTIL_COMM_RESPONSE_OK;
                if (data[3] >= string_length) {
                    response[3] = 0;
                    *response_length = 4;
                } else {
                    const char *substring = KEYBOARD_FILENAME + data[3];
                    string_length -= data[3];
                    *response_length = min(*response_length - 3, string_length);
                    memcpy(&response[3], substring, string_length);
                    *response_length += 3;
                }
                break;
            }
        case UTIL_COMM_ERASE_EEPROM:
            {
                response[2] = UTIL_COMM_RESPONSE_OK;
                for (uint16_t addr = 0; addr < E2END; ++addr) {
                    eeprom_update_byte((uint8_t *)addr, 0xff);
                }
                *response_length = 3;
                break;
            }
        case UTIL_COMM_GET_SIGNAL_VALUE:
            {
                response[2] = UTIL_COMM_RESPONSE_OK;
                uint8_t col = data[3];
                uint8_t row = data[4];
                uint8_t count = data[5];
                if (count > (*response_length - 3) / 2) {
                    count = (*response_length - 3) / 2;
                }
                *response_length = (count * 2) + 3;
                for (int_fast8_t i = 0; i < count; ++i) {
                    uint16_t value = measure_middle_keymap_coords(col, row, CAPSENSE_HARDCODED_SAMPLE_TIME, 8);
                    response[3 + i*2] = value & 0xff;
                    response[3 + i*2 + 1] = (value >> 8) & 0xff;
                    if (++col >= MATRIX_COLS) {
                        col -= MATRIX_COLS;
                        if (++row >= MATRIX_CAPSENSE_ROWS) {
                            break;
                        }
                    }
                }
                break;
            }
        case UTIL_COMM_GET_KEYBOARD_DETAILS:
            {
                response[2] = UTIL_COMM_RESPONSE_OK;
                response[3] = MATRIX_COLS;
                response[4] = MATRIX_ROWS;
                #if defined(CONTROLLER_IS_XWHATSIT_BEAMSPRING_REV_4)
                response[5] = 1;
                #elif defined(CONTROLLER_IS_XWHATSIT_MODEL_F_OR_WCASS_MODEL_F)
                response[5] = 2;
                #elif defined(CONTROLLER_IS_THROUGH_HOLE_BEAMSPRING)
                response[5] = 3;
                #elif defined(CONTROLLER_IS_THROUGH_HOLE_MODEL_F)
                response[5] = 4;
                #else
                response[5] = 0;
                #endif
                response[6] = CAPSENSE_KEYBOARD_SETTLE_TIME_US;
                response[7] = CAPSENSE_DAC_SETTLE_TIME_US;
                response[8] = CAPSENSE_HARDCODED_SAMPLE_TIME;
                response[9] = CAPSENSE_CAL_ENABLED;
                response[10] = CAPSENSE_DAC_MAX & 0xFF;
                response[11] = (CAPSENSE_DAC_MAX >> 8) & 0xFF;
                response[12] = MATRIX_CAPSENSE_ROWS;
                response[13] = 0; // reserved for future
                response[14] = 0; // reserved for future
                response[15] = 0; // reserved for future
                *response_length = 16;
                break;
            }
        case UTIL_COMM_SHIFT_DATA:
            // fallthrough
        case UTIL_COMM_SHIFT_DATA_EXT:
            {
                response[2] = UTIL_COMM_RESPONSE_OK;
                uint32_t shdata = (((uint32_t)(data[3])) << 0) |
                                (((uint32_t)(data[4])) << 8) |
                                (((uint32_t)(data[5])) << 16) |
                                (((uint32_t)(data[6])) << 24);
                int data_idle = 0;
                int shcp_idle = 0;
                int stcp_idle = 0;
                if (data[2] == UTIL_COMM_SHIFT_DATA_EXT) {
                    data_idle = data[7];
                    shcp_idle = data[8];
                    stcp_idle = data[9];
                }
                shift_data(shdata, data_idle, shcp_idle, stcp_idle);
                response[3] = readPin(CAPSENSE_SHIFT_DIN);
                response[4] = readPin(CAPSENSE_SHIFT_SHCP);
                response[5] = readPin(CAPSENSE_SHIFT_STCP);
                *response_length = 6;
                break;
            }
        case UTIL_COMM_SET_DAC_VALUE:
            {
                response[2] = UTIL_COMM_RESPONSE_OK;
                uint16_t value = data[3] | (((uint16_t)data[4]) << 8);
                dac_write_threshold(value);
                *response_length = 3;
                break;
            }
        case UTIL_COMM_GET_ROW_STATE:
            {
                response[2] = UTIL_COMM_RESPONSE_OK;
                response[3] = test_single(255, 0, NULL);
                *response_length = 4;
                break;
            }
        default:
            return RESPONSE_ERROR;
    }
    return RESPONSE_SEND_REPLY;
}
#endif
