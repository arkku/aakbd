/* Copyright 2020 Purdea Andrei
 * Copyright © 2022 Kimmo Kulovesi
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

#ifndef MATRIX_MANIPULATE_H
#define MATRIX_MANIPULATE_H

#include "quantum.h"

// Contains stuff used to manipulate the matrix using the util.
// These are defined in matrix.c. This file is not called matrix.h to avoid conflict with qmk-native matrix.h

extern bool keyboard_scan_enabled;

bool matrix_scan_custom(matrix_row_t current_matrix[]);
uint16_t measure_middle_keymap_coords(uint8_t col, uint8_t row, uint8_t time, uint8_t reps);
void shift_data(uint32_t data, int data_idle, int shcp_idle, int stcp_idle);
void dac_write_threshold(uint16_t value);
uint8_t scan_physical_column(uint8_t col, uint16_t time, uint8_t *interference_ptr);

#if ENABLE_SIMULATED_TYPING
void tracking_test(void);
#endif

#if CAPSENSE_CAL_ENABLED
void clear_saved_matrix_calibration(void);
void save_matrix_calibration(void);
void calibrate_matrix(void);

extern matrix_row_t assigned_to_threshold[CAPSENSE_CAL_BINS][MATRIX_CAPSENSE_ROWS + 1];
extern uint16_t cal_thresholds[CAPSENSE_CAL_BINS];
extern uint8_t cal_bin_rows_mask[CAPSENSE_CAL_BINS];
extern uint8_t cal_bin_key_count[CAPSENSE_CAL_BINS];
extern uint16_t cal_threshold_min, cal_threshold_max;
extern uint16_t cal_threshold_offset;
extern uint8_t cal_flags;

#define CAPSENSE_CAL_FLAG_CALIBRATED    (1 << 0)
#define CAPSENSE_CAL_FLAG_UNRELIABLE    (1 << 1)
#define CAPSENSE_CAL_FLAG_SKIPPED       (1 << 2)
#define CAPSENSE_CAL_FLAG_LOADED        (1 << 3)
#define CAPSENSE_CAL_FLAG_SAVED         (1 << 4)

#define calibration_loaded      ((cal_flags & CAPSENSE_CAL_FLAG_LOADED) ? 1 : 0)
#define calibration_saved       ((cal_flags & CAPSENSE_CAL_FLAG_SAVED) ? 1 : 0)
#define calibration_skipped     ((cal_flags & CAPSENSE_CAL_FLAG_SKIPPED) ? 1 : 0)
#define calibration_unreliable  ((cal_flags & CAPSENSE_CAL_FLAG_UNRELIABLE) ? 1 : 0)
#define calibration_done        ((cal_flags & CAPSENSE_CAL_FLAG_CALIBRATED) ? 1 : 0)

#if CAPSENSE_CAL_DEBUG
extern uint16_t cal_time;
#endif
#endif

#endif
