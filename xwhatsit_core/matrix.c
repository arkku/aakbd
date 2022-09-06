/* Copyright 2020 Purdea Andrei
 *
 * Copyright © 2022 Kimmo Kulovesi:
 * - Added support for saving calibration in EEPROM to enable faster startup
 *   when keys are held down.
 * - Optimised speed when some calibration bins are unused, resulting in
 *   several times faster scanning in typical cases.
 * - Optimised scanning and calibration speed in general.
 * - Made calibration a few times faster by running it in parallel.
 * - Added the option to merge nearby calibration bins, resulting in fewer
 *   bins being used (in most cases only one), and thus faster scanning.
 * - Added the option to try to infer the threshold offset from calibration
 *   data (but it is not possible to get the exactly correct value).
 * - Added some heuristics to try to determine whether calibration was done
 *   with a key pressed, and sometimes to recover from that.
 *
 * (However, only properly tested on brand new Model F keyboards for now.)
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

#include <quantum.h>
#include "matrix_manipulate.h"
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <avr/eeprom.h>
#include <avr/power.h>

#include <eeconfig.h>

#define EECONFIG_CALIBRATION_DATA ((char *) (EECONFIG_KEYMAP_UPPER_BYTE + 1))

#ifndef QMK_KEYMAP
// AAKBD firmware, QMK compatibility - https://github.com/arkku/aakbd
#include <qmk_port.h>
#else 
// Backported from AAKBD to QMK - helpers
#define INLINE inline __attribute__((always_inline))
#define usb_keycode_for_matrix(row, col) pgm_read_word(&keymaps[0][(row)][(col)])
#endif

#ifndef CAPSENSE_HARDCODED_THRESHOLD
#define CAPSENSE_HARDCODED_THRESHOLD (CAPSENSE_DAC_MAX / 7)
#endif

#ifndef CAPSENSE_CAL_THRESHOLD_OFFSET_DYNAMIC
#define CAPSENSE_CAL_THRESHOLD_OFFSET_DYNAMIC 0
#endif

#ifndef CAPSENSE_CAL_MERGE_BINS
#define CAPSENSE_CAL_MERGE_BINS 1
#endif

struct calibration_header {
    uint8_t version;
    uint8_t cols;
    uint8_t rows;
    uint8_t bins;
    uint16_t keymap_checksum;
};

bool keyboard_scan_enabled = true;

#if CAPSENSE_CAL_ENABLED
#ifndef CAPSENSE_CAL_VERSION
#define CAPSENSE_CAL_VERSION 4
#endif

// If this this number, or fewer (but non-zero), keys appear to be suspiciously
// close to the threshold values, try to move them to another bin.
#ifndef CAPSENSE_CAL_SUSPICIOUS_KEY_COUNT_MAX
#define CAPSENSE_CAL_SUSPICIOUS_KEY_COUNT_MAX 4
#endif

#define ASSIGNED_KEYMAP_COLS_MASK_INDEX     (MATRIX_CAPSENSE_ROWS)

matrix_row_t assigned_to_threshold[CAPSENSE_CAL_BINS][MATRIX_CAPSENSE_ROWS + 1] = { { 0 } };
uint16_t cal_thresholds[CAPSENSE_CAL_BINS] = { 0 };
uint8_t cal_bin_rows_mask[CAPSENSE_CAL_BINS] = { 0 };
uint8_t cal_bin_key_count[CAPSENSE_CAL_BINS] = { 0 };
uint16_t cal_threshold_max = CAPSENSE_DAC_MAX;
uint16_t cal_threshold_min = 0;
uint16_t cal_threshold_offset = CAPSENSE_CAL_THRESHOLD_OFFSET;
uint16_t cal_keymap_checksum;
uint8_t cal_flags = 0;

#if CAPSENSE_CAL_DEBUG
uint16_t cal_time = 0;
#endif
#endif // ^ CAPSENSE_CAL_ENABLED

static matrix_row_t previous_matrix[MATRIX_ROWS] = { 0 };

#if MATRIX_EXTRA_DIRECT_ROWS
static pin_t extra_direct_pins[MATRIX_EXTRA_DIRECT_ROWS][MATRIX_COLS] = MATRIX_EXTRA_DIRECT_PINS;
#endif

#define ABSDELTA(a, b) (((a) < (b)) ? (b) - (a) : (a) - (b))

#define scan_physical_col_raw(col, interference) scan_physical_column((col), CAPSENSE_HARDCODED_SAMPLE_TIME, (interference))

#ifdef CAPSENSE_CONDUCTIVE_PLASTIC_IS_PULLED_UP_ON_KEYPRESS
#ifdef MATRIX_ROWS_MASK
#define scan_physical_col(col, interference) ((~scan_physical_col_raw((col), (interference))) & MATRIX_ROWS_MASK)
#else
#define scan_physical_col(col, interference) (~scan_physical_col_raw((col), (interference)))
#endif
#else
#define scan_physical_col(col, interference) scan_physical_col_raw((col), (interference))
#endif

#define SHIFT_BITS (((CAPSENSE_KEYMAP_COL_TO_PHYSICAL_COL(MATRIX_COLS - 1) >= 16) || \
                     (CAPSENSE_KEYMAP_COL_TO_PHYSICAL_COL(0) >= 16)) ? 24 : 16)

/// Convert a single bit mask (i.e., exactly one bit is 1) representing a
/// physical row to the corresponding keymap row index (not mask).
static inline int_fast8_t physical_bit_to_keymap_row(const uint8_t bit) {
    switch (bit) {
    default:       return MATRIX_CAPSENSE_ROWS;
    case (1 << 0): return CAPSENSE_PHYSICAL_ROW_TO_KEYMAP_ROW(0);
    case (1 << 1): return CAPSENSE_PHYSICAL_ROW_TO_KEYMAP_ROW(1);
    case (1 << 2): return CAPSENSE_PHYSICAL_ROW_TO_KEYMAP_ROW(2);
    case (1 << 3): return CAPSENSE_PHYSICAL_ROW_TO_KEYMAP_ROW(3);
#if MATRIX_CAPSENSE_ROWS > 4
    case (1 << 4): return CAPSENSE_PHYSICAL_ROW_TO_KEYMAP_ROW(4);
    case (1 << 5): return CAPSENSE_PHYSICAL_ROW_TO_KEYMAP_ROW(5);
    case (1 << 6): return CAPSENSE_PHYSICAL_ROW_TO_KEYMAP_ROW(6);
    case (1 << 7): return CAPSENSE_PHYSICAL_ROW_TO_KEYMAP_ROW(7);
#endif
    }
}

static INLINE uint8_t read_rows(void) {
    CAPSENSE_READ_ROWS_LOCAL_VARS;
    asm volatile (CAPSENSE_READ_ROWS_ASM_INSTRUCTIONS : CAPSENSE_READ_ROWS_OUTPUT_CONSTRAINTS : CAPSENSE_READ_ROWS_INPUT_CONSTRAINTS);
    return CAPSENSE_READ_ROWS_VALUE;
}

/// The current threshold written to the DAC.
static uint16_t current_threshold;

#if defined(CAPSENSE_DAC_MCP4921)
#define nSHDN_BIT 12
#define MCP_DAC_GAIN_2X 0
#define MCP_DAC_GAIN_1X 1
#define nGA_BIT 13
#define BUF_BIT 14

static void dac_init(void) {
    current_threshold = 0;
    writePin(CAPSENSE_DAC_NCS, 1);
    setPinOutput(CAPSENSE_DAC_NCS);
    setPinOutput(CAPSENSE_DAC_SCK);
    setPinOutput(CAPSENSE_DAC_SDI);
    writePin(CAPSENSE_DAC_NCS, 1);
    writePin(CAPSENSE_DAC_SCK, 0);
    writePin(CAPSENSE_DAC_SDI, 0);
}

void dac_write_threshold(uint16_t value) {
    if (current_threshold == value) {
        return;
    }
    current_threshold = value;

    const uint16_t buffered = 0;
    value |= 1 << nSHDN_BIT; // nSHDN = 0 -- make sure output is not floating.
    value |= MCP_DAC_GAIN_1X << nGA_BIT;
    value |= buffered << BUF_BIT;

    writePin(CAPSENSE_DAC_NCS, 0);
    for (int_fast8_t i = 0; i < 16; ++i) {
        writePin(CAPSENSE_DAC_SDI, (value >> 15) & 1);
        value <<= 1;
        writePin(CAPSENSE_DAC_SCK, 1);
        writePin(CAPSENSE_DAC_SCK, 0);
    }
    writePin(CAPSENSE_DAC_NCS, 1);
    wait_us(CAPSENSE_DAC_SETTLE_TIME_US);
}

#else // ^ CAPSENSE_DAC_MCP4921
static void dac_init(void) {
    setPinOutput(CAPSENSE_DAC_SCLK);
    setPinOutput(CAPSENSE_DAC_DIN);
    setPinOutput(CAPSENSE_DAC_SYNC_N);
    writePin(CAPSENSE_DAC_SYNC_N, 1);
    writePin(CAPSENSE_DAC_SCLK, 0);
    writePin(CAPSENSE_DAC_SCLK, 1);
    writePin(CAPSENSE_DAC_SCLK, 0);
}

void dac_write_threshold(uint16_t value) {
    if (current_threshold == value) {
        return;
    }

    current_threshold = value;
    value <<= 2; // The two LSB bits of this DAC are don't care.
    writePin(CAPSENSE_DAC_SYNC_N, 0);
    for (int_fast8_t i = 16; i; --i) {
        writePin(CAPSENSE_DAC_DIN, (value >> 15) & 1);
        value <<= 1;
        writePin(CAPSENSE_DAC_SCLK, 1);
        writePin(CAPSENSE_DAC_SCLK, 0);
    }
    writePin(CAPSENSE_DAC_SYNC_N, 1);
    writePin(CAPSENSE_DAC_SCLK, 1);
    writePin(CAPSENSE_DAC_SCLK, 0);
    wait_us(CAPSENSE_DAC_SETTLE_TIME_US);
}

#endif

static void shift_select_nothing(void) {
    writePin(CAPSENSE_SHIFT_DIN, 0);
    for (int_fast8_t i = SHIFT_BITS; i; --i) {
        writePin(CAPSENSE_SHIFT_SHCP, 1);
        writePin(CAPSENSE_SHIFT_SHCP, 0);
    }
    writePin(CAPSENSE_SHIFT_STCP, 1);
    writePin(CAPSENSE_SHIFT_STCP, 0);
}

void shift_data(uint32_t data, int data_idle, int shcp_idle, int stcp_idle) {
    writePin(CAPSENSE_SHIFT_SHCP, 0);
    writePin(CAPSENSE_SHIFT_STCP, 0);

    for (int_fast8_t i = SHIFT_BITS; i; --i) {
        writePin(CAPSENSE_SHIFT_DIN, (data >> (SHIFT_BITS - 1)) & 1);
        writePin(CAPSENSE_SHIFT_SHCP, 1);
        if (!((i == 1) && (shcp_idle))) {
            writePin(CAPSENSE_SHIFT_SHCP, 0);
        }
        data <<= 1;
    }
    writePin(CAPSENSE_SHIFT_STCP, 1);
    if (!stcp_idle) {
        writePin(CAPSENSE_SHIFT_STCP, 0);
    }
    writePin(CAPSENSE_SHIFT_DIN, !!data_idle);
}

static void shift_select_col_no_strobe(uint8_t col) {
    for (int_fast8_t i = SHIFT_BITS - 1; i >= 0; --i) {
        writePin(CAPSENSE_SHIFT_DIN, !!(col == i));
        writePin(CAPSENSE_SHIFT_SHCP, 1);
        writePin(CAPSENSE_SHIFT_SHCP, 0);
    }
}

static INLINE void shift_select_col(uint8_t col) {
    shift_select_col_no_strobe(col);
    writePin(CAPSENSE_SHIFT_STCP, 1);
    writePin(CAPSENSE_SHIFT_STCP, 0);
}

static void shift_init(void) {
    setPinOutput(CAPSENSE_SHIFT_DIN);
    setPinOutput(CAPSENSE_SHIFT_OE);
    setPinOutput(CAPSENSE_SHIFT_STCP);
    setPinOutput(CAPSENSE_SHIFT_SHCP);
    writePin(CAPSENSE_SHIFT_OE, 0);
    writePin(CAPSENSE_SHIFT_STCP, 0);
    writePin(CAPSENSE_SHIFT_SHCP, 0);
    shift_select_nothing();
    wait_us(CAPSENSE_KEYBOARD_SETTLE_TIME_US);
}

uint8_t scan_physical_column(uint8_t col, uint16_t time, uint8_t *interference_ptr) {
    shift_select_col_no_strobe(col);
    uint16_t index;
    CAPSENSE_READ_ROWS_LOCAL_VARS;
    uint8_t array[CAPSENSE_READ_ROWS_NUMBER_OF_BYTES_PER_SAMPLE + 1]; // one sample before triggering, and one dummy byte
uint8_t *arrayp = array;
    asm volatile (
             "ldi %A[index], 0"                 "\n\t"
             "ldi %B[index], 0"                 "\n\t"
             "cli"                              "\n\t"
             CAPSENSE_READ_ROWS_ASM_INSTRUCTIONS                 "\n\t"
             CAPSENSE_READ_ROWS_STORE_TO_ARRAY_INSTRUCTIONS      "\n\t"
             "sbi %[stcp_regaddr], %[stcp_bit]" "\n\t"
        "1:" CAPSENSE_READ_ROWS_ASM_INSTRUCTIONS                 "\n\t"
             CAPSENSE_READ_ROWS_STORE_TO_ARRAY_INSTRUCTIONS_FAKE "\n\t"
             "adiw %A[index], 0x01"             "\n\t"
             "cp %A[index], %A[time]"           "\n\t"
             "cpc %B[index], %B[time]"          "\n\t"
             "brlo 1b"                          "\n\t"
             "sei"                              "\n\t"
             "cbi %[stcp_regaddr], %[stcp_bit]" "\n\t"
      : [arr] "=e" (arrayp),
        [index] "=&w" (index),
        CAPSENSE_READ_ROWS_OUTPUT_CONSTRAINTS
      : [time] "r" (time + 1),
        [stcp_regaddr] "I" (CAPSENSE_SHIFT_STCP_IO),
        [stcp_bit] "I" (CAPSENSE_SHIFT_STCP_BIT),
        CAPSENSE_READ_ROWS_INPUT_CONSTRAINTS,
        "0" (arrayp)
      : "memory" );
    shift_select_nothing();
    wait_us(CAPSENSE_KEYBOARD_SETTLE_TIME_US);
    uint8_t value_at_time = CAPSENSE_READ_ROWS_VALUE;
    if (interference_ptr) {
        uint16_t p0 = 0;
        CAPSENSE_READ_ROWS_EXTRACT_FROM_ARRAY;
        uint8_t interference = CAPSENSE_READ_ROWS_VALUE;
        *interference_ptr = interference;
    }
    return value_at_time;
}

static uint16_t measure_middle(uint8_t col, uint8_t row, uint8_t time, uint8_t samples) {
    uint8_t samples_div2 = samples / 2;
    uint16_t min = 0, max = CAPSENSE_DAC_MAX;
    while (min < max) {
        uint16_t mid = (min + max) / 2;
        dac_write_threshold(mid);
        uint8_t sum = 0;
        for (int_fast8_t i = samples; i; --i) {
            sum += (scan_physical_column(col, time, NULL) >> row) & 1;
        }
        if (sum < samples_div2) {
            max = mid - 1;
        } else if (sum > samples_div2) {
            min = mid + 1;
        } else {
            return mid;
        }
    }
    return min;
}

uint16_t measure_middle_keymap_coords(uint8_t col, uint8_t row, uint8_t time, uint8_t samples) {
    return measure_middle(CAPSENSE_KEYMAP_COL_TO_PHYSICAL_COL(col), CAPSENSE_KEYMAP_ROW_TO_PHYSICAL_ROW(row), time, samples);
}

#if CAPSENSE_CAL_ENABLED
#define calibration_measure_all_zero(valid) calibration_measure_all(CAPSENSE_HARDCODED_SAMPLE_TIME, CAPSENSE_CAL_INIT_REPS, true, (valid))
#define calibration_measure_all_one(valid) calibration_measure_all(CAPSENSE_HARDCODED_SAMPLE_TIME, CAPSENSE_CAL_INIT_REPS, false, (valid))

#ifdef CAPSENSE_CONDUCTIVE_PLASTIC_IS_PUSHED_DOWN_ON_KEYPRESS
#define row_result row_max
#define first_bin_index 0
#define last_bin_index (CAPSENSE_CAL_BINS - 1)
#else
#define row_result row_min
#define first_bin_index (CAPSENSE_CAL_BINS - 1)
#define last_bin_index 0
#endif

static uint16_t calibration_measure_all(uint8_t time, int_fast8_t samples, bool looking_for_all_zero, bool valid_keys) {
    uint16_t min = 0, max = CAPSENSE_DAC_MAX;
    bool scanned;
    do {
        const uint16_t mid = (min + max + !looking_for_all_zero) / 2;

        dac_write_threshold(mid);

        scanned = false;

        for (int_fast8_t col = 0; col < MATRIX_COLS; ++col) {
            uint8_t valid_physical_rows = 0;
            for (int_fast8_t row = 0; row < MATRIX_CAPSENSE_ROWS; ++row) {
                if ((usb_keycode_for_matrix(row, col) != 0) == valid_keys) {
                    valid_physical_rows |= (1 << CAPSENSE_KEYMAP_ROW_TO_PHYSICAL_ROW(row));
                }
            }

            if (!valid_physical_rows) {
                continue;
            }

            // Check if this threshold gives a stable desired result
            scanned = true;
            uint8_t physical_col = CAPSENSE_KEYMAP_COL_TO_PHYSICAL_COL(col);
            const uint8_t desired_result = looking_for_all_zero ? 0 : valid_physical_rows;
            for (int_fast8_t i = samples; i; --i) {
                const uint8_t result = scan_physical_column(physical_col, time, NULL) & valid_physical_rows;
                if (result != desired_result) {
                    if (looking_for_all_zero) {
                        min = mid + 1;
                    } else {
                        max = mid - 1;
                    }
                    goto next_binary_search;
                }
            }
        }

        // The result was stable, see if we can tighten the threshold
        if (looking_for_all_zero) {
            max = mid;
        } else {
            min = mid;
        }
        next_binary_search:
            ;
    } while (min < max && scanned);
    return looking_for_all_zero ? max : min;
}

void calibrate_matrix(void) {
    uint16_t cal_thresholds_max[CAPSENSE_CAL_BINS];
    uint16_t cal_thresholds_min[CAPSENSE_CAL_BINS];

    _Static_assert(CAPSENSE_KEYMAP_ROW_TO_PHYSICAL_ROW(0) < 8);
    _Static_assert(CAPSENSE_KEYMAP_ROW_TO_PHYSICAL_ROW(1) < 8);
    _Static_assert(CAPSENSE_KEYMAP_ROW_TO_PHYSICAL_ROW(2) < 8);
    _Static_assert(CAPSENSE_KEYMAP_ROW_TO_PHYSICAL_ROW(3) < 8);
#if MATRIX_CAPSENSE_ROWS > 4
    _Static_assert(CAPSENSE_KEYMAP_ROW_TO_PHYSICAL_ROW(4) < 8);
    _Static_assert(CAPSENSE_KEYMAP_ROW_TO_PHYSICAL_ROW(5) < 8);
    _Static_assert(CAPSENSE_KEYMAP_ROW_TO_PHYSICAL_ROW(6) < 8);
    _Static_assert(CAPSENSE_KEYMAP_ROW_TO_PHYSICAL_ROW(7) < 8);
#endif
    _Static_assert(MATRIX_CAPSENSE_ROWS <= 8);

    for (int_fast8_t bin = 0; bin < CAPSENSE_CAL_BINS; ++bin) {
        for (int_fast8_t row = 0; row <= MATRIX_CAPSENSE_ROWS; row++) {
            assigned_to_threshold[bin][row] = 0;
        }
        cal_bin_key_count[bin] = 0;
        cal_thresholds_max[bin] = 0U;
        cal_thresholds_min[bin] = 0xFFFFU;
    }

    // Find the total range by scanning invalid keys, which should include
    // both always and never pressed dummy positions
    uint16_t full_min = calibration_measure_all_zero(false);
    uint16_t full_max = calibration_measure_all_one(false);

    if (full_min > full_max) {
        const uint16_t tmp = full_min;
        full_min = full_max;
        full_max = tmp;
    }

    // Find the range we need to scan for each valid key
    uint16_t min = calibration_measure_all_zero(true);
    uint16_t max = calibration_measure_all_one(true);

    if (min > max) {
        const uint16_t tmp = min;
        min = max;
        max = tmp;
    }

    if (min < full_min) {
        full_min = min;
    }
    if (max > full_max) {
        full_max = max;
    }

#if CAPSENSE_CAL_THRESHOLD_OFFSET_DYNAMIC
    // Magic numbers to try to determine the threshold offset
    // (It is not, in the general case, possible to infer this info from
    // the calibration data alone, since it would need measuring the keys
    // both pressed and non-pressed, which would need user interaction.
    // This arbitrary formula yields results close to the cargo cult
    // hardcoded values.)
    cal_threshold_offset = (full_max - full_min) / 6;
    if (cal_threshold_offset < CAPSENSE_CAL_THRESHOLD_OFFSET / 2) {
        cal_threshold_offset = CAPSENSE_CAL_THRESHOLD_OFFSET / 2;
    } else if (cal_threshold_offset > CAPSENSE_CAL_THRESHOLD_OFFSET * 2) {
        cal_threshold_offset = CAPSENSE_CAL_THRESHOLD_OFFSET;
    }
#endif

    /// Determine the bin size and spacing
    uint16_t bin_max_size = (cal_threshold_offset - (cal_threshold_offset / 4)) + 1;

    uint16_t bin_spacing = (max - min);
#if CAPSENSE_CAL_BINS > 1
    bin_spacing /= (CAPSENSE_CAL_BINS - 1);
#endif
    if (bin_spacing > bin_max_size) {
        bin_spacing = bin_max_size;
    } else if (bin_spacing < cal_threshold_offset / 4) {
        bin_spacing = cal_threshold_offset / 4;
    }

    {
        const uint16_t half_step = bin_spacing / 2;
#ifdef CAPSENSE_CONDUCTIVE_PLASTIC_IS_PUSHED_DOWN_ON_KEYPRESS
        uint16_t threshold = min + half_step;
        for (int_fast8_t bin = first_bin_index; bin <= last_bin_index; ++bin) {
            cal_thresholds[bin] = threshold;
            threshold += bin_spacing;
        }
#else
        uint16_t threshold = max;
        if (threshold >= half_step) {
            threshold -= half_step;
        }
        for (int_fast8_t bin = first_bin_index; bin >= last_bin_index; --bin) {
            cal_thresholds[bin] = threshold;
            if (threshold >= bin_spacing) {
                threshold -= bin_spacing;
            }
        }
#endif
    }

    // Extend the per-key scan range a bit
    {
        const uint16_t range_extend_amount = cal_threshold_offset / 2;

        if (bin_spacing < range_extend_amount) {
            bin_spacing = range_extend_amount;
        }

        max += range_extend_amount;

        if (min >= range_extend_amount) {
            min -= range_extend_amount;
        } else {
            min = 0;
        }
        if (max > CAPSENSE_DAC_MAX){
            max = CAPSENSE_DAC_MAX;
        }
    }

    // These are the actual thresholds seen on individual keys
    cal_threshold_min = CAPSENSE_DAC_MAX;
    cal_threshold_max = 0;

    // Measure each column and assign its rows to bins
    matrix_row_t col_mask = 1;
    for (int_fast8_t col = 0; col < MATRIX_COLS; col++, col_mask <<= 1) {
        const uint8_t physical_col = CAPSENSE_KEYMAP_COL_TO_PHYSICAL_COL(col);

        uint16_t row_min[MATRIX_CAPSENSE_ROWS];
        uint16_t row_max[MATRIX_CAPSENSE_ROWS];
        uint8_t physical_rows_mask = 0;

        for (int_fast8_t row = 0; row < MATRIX_CAPSENSE_ROWS; row++) {
            if (usb_keycode_for_matrix(row, col)) {
                row_min[row] = min;
                row_max[row] = max;
                physical_rows_mask |= (1 << CAPSENSE_KEYMAP_ROW_TO_PHYSICAL_ROW(row));
            } else {
                row_min[row] = CAPSENSE_DAC_MAX + 1;
                row_max[row] = CAPSENSE_DAC_MAX + 1;
            }
        }

        // Binary search the rows in this for the DAC threshold that makes them
        // read about 50/50 zero or one
        uint8_t uncalibrated_rows_mask = physical_rows_mask;
        while (uncalibrated_rows_mask) {
            const uint8_t uncalibrated_row_mask = uncalibrated_rows_mask & -uncalibrated_rows_mask;
            const int_fast8_t uncalibrated_row = physical_bit_to_keymap_row(uncalibrated_row_mask);

            uint16_t lower_bound = row_min[uncalibrated_row];
            uint16_t upper_bound = row_max[uncalibrated_row];

            if (lower_bound < upper_bound) {
                // The search has not yet completed for this row
                uint16_t mid = lower_bound + upper_bound;
                #ifdef CAPSENSE_CONDUCTIVE_PLASTIC_IS_PUSHED_DOWN_ON_KEYPRESS
                    mid -= 1;
                #else
                    mid += 1;
                #endif
                mid /= 2;

                dac_write_threshold(mid);

                // Sample all rows in this column in parallel
                uint8_t seen_rows = 0;
                for (int_fast8_t samples = CAPSENSE_CAL_EACHKEY_REPS; samples; --samples) {
                    seen_rows |= scan_physical_col(physical_col, NULL) & physical_rows_mask;
                    if (seen_rows == physical_rows_mask) {
                        // Already seen all rows, result can't change
                        break;
                    }
                }

                // Find the threshold when each key consistently reads 0
                for (int_fast8_t row = 0; row < MATRIX_CAPSENSE_ROWS; ++row) {
                    const uint8_t row_mask = (1 << CAPSENSE_KEYMAP_ROW_TO_PHYSICAL_ROW(row));
                    if (physical_rows_mask & row_mask) {
                        #ifdef CAPSENSE_CONDUCTIVE_PLASTIC_IS_PUSHED_DOWN_ON_KEYPRESS
                            if (seen_rows & row_mask) {
                                row_min[row] = mid + 1; // mid is not correct
                            } else if (row_max[row] > mid) {
                                row_max[row] = mid; // mid might be correct
                            }
                        #else
                            if (seen_rows & row_mask) {
                                row_max[row] = mid - 1;
                            } else if (row_min[row] < mid) {
                                row_min[row] = mid;
                            }
                        #endif
                    }
                }
            } else {
                // The search has been exhausted for this row
                uncalibrated_rows_mask ^= uncalibrated_row_mask;
            }
        }

        // Assign the rows to bins
        uncalibrated_rows_mask = physical_rows_mask;
        while (uncalibrated_rows_mask) {
            const uint8_t uncalibrated_row_mask = uncalibrated_rows_mask & -uncalibrated_rows_mask;
            uncalibrated_rows_mask ^= uncalibrated_row_mask;

            // Find the (next) lowest threshold row that is unassigned
            int_fast8_t this_row = MATRIX_CAPSENSE_ROWS;
            uint16_t threshold = CAPSENSE_DAC_MAX;
            for (int_fast8_t row = 0; row < MATRIX_CAPSENSE_ROWS; ++row) {
                const uint16_t result = row_result[row];
                if (result <= threshold) {
                    threshold = result;
                    this_row = row;
                }
            }

            if (this_row == MATRIX_CAPSENSE_ROWS) {
                break;
            }

            // Track the range of results for debugging
            if (threshold > cal_threshold_max) {
                cal_threshold_max = threshold;
            }
            if (threshold < cal_threshold_min) {
                cal_threshold_min = threshold;
            }

            // Mark as assigned
            row_result[this_row] = CAPSENSE_DAC_MAX + 1;

            uint8_t physical_row_mask = (1 << CAPSENSE_KEYMAP_ROW_TO_PHYSICAL_ROW(this_row));

            // Find the bin closest to this threshold
            int_fast8_t best_bin = first_bin_index;
            uint16_t bin_threshold = 0xFFFFU;
            uint16_t best_diff = 0xFFFFU;

#ifdef CAPSENSE_CONDUCTIVE_PLASTIC_IS_PUSHED_DOWN_ON_KEYPRESS
            for (int_fast8_t bin = first_bin_index; bin <= last_bin_index; ++bin)
#else
            for (int_fast8_t bin = first_bin_index; bin >= last_bin_index; --bin)
#endif
            {
                bin_threshold = cal_thresholds[bin];
                uint16_t this_diff = ABSDELTA(threshold, bin_threshold);
                if (this_diff < best_diff) {
                    best_diff = this_diff;
                    best_bin = bin;
                }
            }

            if (cal_bin_key_count[best_bin] == 0) {
                // First key in this bin, set the true bin threshold now
                cal_thresholds[best_bin] = threshold;
                cal_thresholds_min[best_bin] = threshold;
                cal_thresholds_max[best_bin] = threshold;

                // Reassign the adjacent thresholds based on what ended up here
                int_fast8_t adjacent = best_bin - 1;
                if (adjacent >= 0 && cal_bin_key_count[adjacent] == 0) {
                    cal_thresholds[adjacent] = threshold - bin_spacing;
                }
                adjacent = best_bin + 1;
                if (adjacent < CAPSENSE_CAL_BINS && cal_bin_key_count[adjacent] == 0) {
                    cal_thresholds[adjacent] = threshold + bin_spacing;
                }
            } else {
                // Track the range of thresholds assined to this bin
                if (cal_thresholds_max[best_bin] < threshold) {
                    cal_thresholds_max[best_bin] = threshold;
                }
                if (cal_thresholds_min[best_bin] > threshold) {
                    cal_thresholds_min[best_bin] = threshold;
                }
            }

            cal_bin_key_count[best_bin] += 1;
            assigned_to_threshold[best_bin][this_row] |= col_mask;

            // Combined mask of all columns in this bin
            assigned_to_threshold[best_bin][ASSIGNED_KEYMAP_COLS_MASK_INDEX] |= col_mask;

            // Combined mask of all rows in this bin
            cal_bin_rows_mask[best_bin] |= physical_row_mask;
        }
    }

    // Assign the final thresholds based on the actual keys in each bin
    const uint16_t suspicious_bin_delta = cal_threshold_offset + bin_spacing + 1;
    uint16_t previous_bin_level;
    int_fast8_t previous_bin;
#ifdef CAPSENSE_CONDUCTIVE_PLASTIC_IS_PUSHED_DOWN_ON_KEYPRESS
#define has_previous_bin (previous_bin >= 0)
    previous_bin = -1;
    previous_bin_level = cal_threshold_min + bin_spacing;

    for (int_fast8_t bin = 0; bin < CAPSENSE_CAL_BINS; ++bin)
#else
#define has_previous_bin (previous_bin < CAPSENSE_CAL_BINS)
    previous_bin = CAPSENSE_CAL_BINS;
    previous_bin_level = cal_threshold_max;
    if (previous_bin_level > bin_spacing) {
        previous_bin_level -= bin_spacing;
    }

    for (int_fast8_t bin = CAPSENSE_CAL_BINS - 1; bin >= 0; --bin)
#endif
    {
        if (cal_bin_key_count[bin] == 0) {
            continue;
        }

        uint_fast16_t bin_signal_level;

        // Take the average of the bin extremities
        bin_signal_level = cal_thresholds_max[bin] + cal_thresholds_min[bin];
        #ifdef CAPSENSE_CONDUCTIVE_PLASTIC_IS_PUSHED_DOWN_ON_KEYPRESS
            bin_signal_level += 1;
        #endif
        bin_signal_level /= 2;

        // Offset the level so as to be more lenient with the signal
        #ifdef CAPSENSE_CONDUCTIVE_PLASTIC_IS_PUSHED_DOWN_ON_KEYPRESS
            bin_signal_level += cal_threshold_offset;
            if (bin_signal_level < cal_thresholds_max[bin]) {
                // Wide bin: avoid unreliable keys at the upper end
                bin_signal_level = cal_thresholds_max[bin];
            }
        #else
            if (bin_signal_level < cal_threshold_offset) {
                bin_signal_level = 0;
            } else {
                bin_signal_level -= cal_threshold_offset;
            }
            if (bin_signal_level > cal_thresholds_min[bin]) {
                // Wide bin: avoid unreliable keys at the lower end
                bin_signal_level = cal_thresholds_min[bin];
            }
        #endif

        // Assign the final threshold for this bin
        cal_thresholds[bin] = bin_signal_level;

        // Sanity-check the bin
        const uint16_t bin_delta = ABSDELTA(previous_bin_level, bin_signal_level);

#if CAPSENSE_CAL_MERGE_BINS
        if (has_previous_bin && cal_bin_key_count[previous_bin] != 0 && bin_delta <= cal_threshold_offset / 2) {
            // The previous bin is very close to this one, maybe merge?
            uint16_t min_min = cal_thresholds_min[bin];
            if (cal_thresholds_min[previous_bin] < min_min) {
                min_min = cal_thresholds_min[previous_bin];
            }
            uint16_t max_max = cal_thresholds_max[bin];
            if (cal_thresholds_max[previous_bin] > max_max) {
                max_max = cal_thresholds_max[previous_bin];
            }

            if ((max_max - min_min) <= bin_max_size) {
                // This could have been one bin if the midpoint had been
                // different, probably safe to merge into one
                cal_bin_key_count[previous_bin] += cal_bin_key_count[bin];
                cal_bin_key_count[bin] = 0;
                cal_bin_rows_mask[previous_bin] |= cal_bin_rows_mask[bin];
                cal_bin_rows_mask[bin] = 0;
                for (int_fast8_t row = 0; row <= MATRIX_CAPSENSE_ROWS; ++row) {
                    assigned_to_threshold[previous_bin][row] |= assigned_to_threshold[bin][row];
                    #if !CAPSENSE_CAL_DEBUG
                        assigned_to_threshold[bin][row] = 0;
                    #endif
                }
                cal_thresholds_min[previous_bin] = min_min;
                cal_thresholds_max[previous_bin] = max_max;

                bin_signal_level = (cal_thresholds[previous_bin] + cal_thresholds[bin]);
                #ifdef CAPSENSE_CONDUCTIVE_PLASTIC_IS_PUSHED_DOWN_ON_KEYPRESS
                    bin_signal_level += 1;
                #endif
                bin_signal_level /= 2;
                cal_thresholds[previous_bin] = bin_signal_level;
                previous_bin_level = bin_signal_level;

                continue;
            }
        }
#endif

        if (bin_delta > suspicious_bin_delta) {
            // Suspicously large jump between bins
            cal_flags |= CAPSENSE_CAL_FLAG_UNRELIABLE;

            if (has_previous_bin) {
                // Probably held keys, move them to the previous bin
                const bool previous_was_empty = (cal_bin_key_count[previous_bin] == 0);

                cal_bin_key_count[previous_bin] += cal_bin_key_count[bin];
                cal_bin_key_count[bin] = 0;
                cal_bin_rows_mask[previous_bin] |= cal_bin_rows_mask[bin];
                cal_bin_rows_mask[bin] = 0;

                for (int_fast8_t row = 0; row <= MATRIX_CAPSENSE_ROWS; ++row) {
                    if (previous_was_empty) {
                        assigned_to_threshold[previous_bin][row] = assigned_to_threshold[bin][row];
                    } else {
                        assigned_to_threshold[previous_bin][row] |= assigned_to_threshold[bin][row];
                    }
                    #if !CAPSENSE_CAL_DEBUG
                        assigned_to_threshold[bin][row] = 0;
                    #endif
                }

                previous_bin_level = cal_thresholds[previous_bin];
                continue;
            }
        }

        previous_bin_level = bin_signal_level;
        previous_bin = bin;
    }

    if (has_previous_bin && cal_bin_key_count[previous_bin] <= CAPSENSE_CAL_SUSPICIOUS_KEY_COUNT_MAX && cal_bin_key_count[previous_bin] != 0) {
        // Suspiciously few keys in the last bin
        cal_flags |= CAPSENSE_CAL_FLAG_UNRELIABLE;
    }

#if CAPSENSE_CAL_DEBUG
    if (cal_bin_key_count[0] == 0) {
        cal_thresholds[0] = full_min;
    }
    if (cal_bin_key_count[CAPSENSE_CAL_BINS - 1] == 0) {
        cal_thresholds[CAPSENSE_CAL_BINS - 1] = full_max;
    }
#endif

    cal_flags |= CAPSENSE_CAL_FLAG_CALIBRATED;
}
#endif

static bool load_matrix_calibration(void) {
#if CAPSENSE_CAL_ENABLED
    const char *p = EECONFIG_CALIBRATION_DATA;
    struct calibration_header header;

    // Check that the saved calibration matches this configuration
    eeprom_read_block(&header, p, sizeof(header));
    if (header.cols != MATRIX_COLS || header.rows != MATRIX_CAPSENSE_ROWS || header.bins != CAPSENSE_CAL_BINS || header.version != CAPSENSE_CAL_VERSION || header.keymap_checksum != cal_keymap_checksum) {
        return false;
    }

    p += sizeof(header);
    eeprom_read_block(cal_thresholds, p, sizeof(cal_thresholds));
    p += sizeof(cal_thresholds);
    eeprom_read_block(cal_bin_rows_mask, p, sizeof(cal_bin_rows_mask));
    p += sizeof(cal_bin_rows_mask);
    eeprom_read_block(assigned_to_threshold, p, sizeof(assigned_to_threshold));
    p += sizeof(assigned_to_threshold);
    eeprom_read_block(cal_bin_key_count, p, sizeof(cal_bin_key_count));
    p += sizeof(cal_bin_key_count);
    cal_threshold_max = eeprom_read_word((const uint16_t *) p);
    p += sizeof(cal_threshold_max);
    cal_threshold_min = eeprom_read_word((const uint16_t *) p);
    p += sizeof(cal_threshold_min);
    cal_threshold_offset = eeprom_read_word((const uint16_t *) p);
    p += sizeof(cal_threshold_offset);
    eeprom_read_block(&header, p, sizeof(header));

    if (header.cols != MATRIX_COLS || header.rows != MATRIX_CAPSENSE_ROWS || header.bins != CAPSENSE_CAL_BINS || header.version != CAPSENSE_CAL_VERSION || header.keymap_checksum != cal_keymap_checksum) {
        return false;
    }

    cal_flags |= CAPSENSE_CAL_FLAG_LOADED;
    return true;
#else
    return false;
#endif
}

void clear_saved_matrix_calibration(void) {
    const struct calibration_header header = { .version = 0, .cols = 0, .rows = 0, .bins = 1, .keymap_checksum = 0xDEADU };
    char *p = EECONFIG_CALIBRATION_DATA;
    eeprom_update_block(&header, p, sizeof(header));
}

void save_matrix_calibration(void) {
#if CAPSENSE_CAL_ENABLED
    char *p = EECONFIG_CALIBRATION_DATA;
    const struct calibration_header header = {
      .version = CAPSENSE_CAL_VERSION,
      .cols = MATRIX_COLS,
      .rows = MATRIX_CAPSENSE_ROWS,
      .bins = CAPSENSE_CAL_BINS,
      .keymap_checksum = cal_keymap_checksum
    };

    eeprom_update_block(&header, p, sizeof(header));
    p += sizeof(header);
    eeprom_update_block(cal_thresholds, p, sizeof(cal_thresholds));
    p += sizeof(cal_thresholds);
    eeprom_update_block(cal_bin_rows_mask, p, sizeof(cal_bin_rows_mask));
    p += sizeof(cal_bin_rows_mask);
    eeprom_update_block(assigned_to_threshold, p, sizeof(assigned_to_threshold));
    p += sizeof(assigned_to_threshold);
    eeprom_update_block(cal_bin_key_count, p, sizeof(cal_bin_key_count));
    p += sizeof(cal_bin_key_count);
    eeprom_update_word((uint16_t *) p, cal_threshold_max);
    p += sizeof(cal_threshold_max);
    eeprom_update_word((uint16_t *) p, cal_threshold_min);
    p += sizeof(cal_threshold_min);
    eeprom_update_word((uint16_t *) p, cal_threshold_offset);
    p += sizeof(cal_threshold_offset);
    
    // Duplicate the header at the end in case the write gets interrupted
    eeprom_update_block(&header, p, sizeof(header));
    cal_flags |= CAPSENSE_CAL_FLAG_SAVED;
#endif
}

#if CAPSENSE_CAL_ENABLED
static uint16_t keymap_checksum(void) {
    uint16_t checksum = 0;

    for (int_fast8_t col = 0; col < MATRIX_COLS; ++col) {
        for (int_fast8_t row = 0; row < MATRIX_CAPSENSE_ROWS; ++row) {
            uint16_t keycode = usb_keycode_for_matrix(row, col);
            if (keycode) {
                checksum += ~keycode + (((keycode * (col + 1))) ^ (row + 1));
                checksum <<= 3;
                checksum >>= (16 - 3);
            }
        }
    }

    return ~checksum;
}

#define MASK_TO_ROW_CASE(x) (1 << (x)): row = CAPSENSE_PHYSICAL_ROW_TO_KEYMAP_ROW((x)); break

static inline void scan_bin(const int_fast8_t bin, matrix_row_t current_matrix[]) {
    const uint8_t bin_physical_rows_mask = cal_bin_rows_mask[bin];
    if (bin_physical_rows_mask == 0) {
        // No keys in this bin, we can skip it
        return;
    }

    // Set the threshold of this bin and then scan all keys in the bin
    dac_write_threshold(cal_thresholds[bin]);

    const matrix_row_t bin_columns_mask = assigned_to_threshold[bin][ASSIGNED_KEYMAP_COLS_MASK_INDEX];

    matrix_row_t col_mask = 1;
    for (int_fast8_t col = 0; col < MATRIX_COLS; ++col, col_mask <<= 1) {
        const int_fast8_t physical_col = CAPSENSE_KEYMAP_COL_TO_PHYSICAL_COL(col);

        uint8_t active_rows_in_col = 0, interference;

        if (bin_columns_mask & col_mask) {
            // This column has keys assigned to this bin
            active_rows_in_col = scan_physical_col(physical_col, &interference);
            // Mask out rows that are not in this bin
            active_rows_in_col &= bin_physical_rows_mask;
        }

        // Iterate over each row that's on in this column
        while (active_rows_in_col) {
            // Isolate the lowest 1 bit
            const uint8_t physical_row_mask = active_rows_in_col & -active_rows_in_col;
            // Turn it off (for loop condition)
            active_rows_in_col ^= physical_row_mask;

            const int_fast8_t row = physical_bit_to_keymap_row(physical_row_mask);

            if (assigned_to_threshold[bin][row] & col_mask) {
                // The key is assigned to this bin
                if (!(interference & physical_row_mask)) {
                    current_matrix[row] |= col_mask;
                }
            }
        }
    }
}
#endif

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    bool changed = false;

    for (int_fast8_t i = 0; i < MATRIX_ROWS; ++i) {
        current_matrix[i] = 0;
    }

    if (!keyboard_scan_enabled) {
        goto end_of_scan;
    }

#if CAPSENSE_CAL_ENABLED
    static bool scan_ascending = true;

    if (scan_ascending) {
        for (int_fast8_t bin = 0; bin < CAPSENSE_CAL_BINS; ++bin) {
            scan_bin(bin, current_matrix);
        }
    } else {
        for (int_fast8_t bin = CAPSENSE_CAL_BINS; bin; ) {
            scan_bin(--bin, current_matrix);
        }
    }
    scan_ascending = !scan_ascending;
#else // ^ CAPSENSE_CAL_ENABLED
    for (int_fast8_t col = 0; col < MATRIX_COLS; ++col) {
        int_fast8_t real_col = CAPSENSE_KEYMAP_COL_TO_PHYSICAL_COL(col);
        uint8_t interference;
        uint8_t active_rows_in_col = scan_physical_col(physical_col, &interference);
        for (int_fast8_t row = 0; row < MATRIX_CAPSENSE_ROWS; ++row) {
            current_matrix[CAPSENSE_PHYSICAL_ROW_TO_KEYMAP_ROW(row)] |= (((matrix_row_t)(active_rows_in_col & 1)) << col);
            active_rows_in_col >>= 1;
        }
    }
#endif

#if MATRIX_EXTRA_DIRECT_ROWS
    for (int_fast8_t row = 0; row < MATRIX_EXTRA_DIRECT_ROWS; ++row) {
        for (int_fast8_t col = 0; col < MATRIX_EXTRA_DIRECT_COLS; ++col) {
            const pin_t pin = extra_direct_pins[row][col];
            if (pin != NO_PIN) {
                uint8_t value = readPin(pin);
                #if MATRIX_EXTRA_DIRECT_PINS_ACTIVE_LOW
                    value = !value;
                #endif
                if (value) {
                    current_matrix[MATRIX_CAPSENSE_ROWS + row] |= ((matrix_row_t) 1) << col;
                }
            }
        }
    }
#endif

end_of_scan:
    for (int_fast8_t row = 0; row < MATRIX_ROWS; ++row) {
        if (previous_matrix[row] != current_matrix[row]) {
            changed = true;
        }
        previous_matrix[row] = current_matrix[row];
    }
    return changed;
}

void matrix_init_custom(void) {
#if defined(CONTROLLER_IS_THROUGH_HOLE_BEAMSPRING) || defined(CONTROLLER_IS_THROUGH_HOLE_MODEL_F)
    // Disable on-board leds
    setPinOutput(D5);
    writePin(D5, 1);
    setPinOutput(B0);
    writePin(B0, 1);
#endif

#if MATRIX_EXTRA_DIRECT_ROWS
    for (int_fast8_t row = 0; row < MATRIX_EXTRA_DIRECT_ROWS; ++row) {
        for (int col=0; col<MATRIX_COLS; col++) {
            pin_t pin = extra_direct_pins[row][col];
            if (pin != NO_PIN) {
                #if MATRIX_EXTRA_DIRECT_PINS_NEED_INTERNAL_PULLUP
                    setPinInputHigh(pin);
                #else
                    setPinInput(pin);
                #endif
            }
        }
    }
#endif

    shift_init();
    dac_init();
    SETUP_ROW_GPIOS();
    SETUP_UNUSED_PINS();

    // Power reduction
#if defined(ACSR) && defined(ACD) && !defined(ENABLE_AC)
    ACSR |= (1 << ACD);
#endif
#ifndef ENABLE_USART
    power_usart1_disable();
#endif
    power_timer1_disable();
#ifdef TIMSK2
    power_timer2_disable();
#endif
#ifdef TIMSK3
    power_timer3_disable();
#endif
#ifndef ENABLE_SPI
    power_spi_disable();
#endif
#if defined(TWIE) && !defined(ENABLE_I2C)
    power_twi_disable();
#endif

    keyboard_scan_enabled = true;

#if CAPSENSE_CAL_ENABLED
    cal_flags = 0;
    cal_keymap_checksum = keymap_checksum();
#endif
}

static void clear_matrix(void) {
    bool was_enabled = keyboard_scan_enabled;
    keyboard_scan_enabled = false;
    (void) matrix_scan(); // This clears the matrix when scan is disabled
    keyboard_scan_enabled = was_enabled;
}

extern matrix_row_t raw_matrix[MATRIX_ROWS];

void matrix_init_kb(void) {
    matrix_init_user();

    dac_write_threshold(CAPSENSE_HARDCODED_THRESHOLD);
    current_threshold = 0;
    dac_write_threshold(CAPSENSE_HARDCODED_THRESHOLD);
    current_threshold = 0;
    dac_write_threshold(CAPSENSE_HARDCODED_THRESHOLD);

#ifdef ERASE_CALIBRATION_ON_START
    (void) clear_saved_matrix_calibration();
#endif

#if CAPSENSE_CAL_ENABLED
#if CAPSENSE_CAL_DEBUG
    cal_time = timer_read();
#endif

#ifdef ERASE_CALIBRATION_ON_START
    cal_flags |= CAPSENSE_CAL_FLAG_UNRELIABLE;
#else
    if (!calibration_done) {
        load_matrix_calibration();
    }
#endif

    if (calibration_loaded) {
        (void) matrix_scan_custom(raw_matrix);
        uint8_t active_key_count = 0;
        for (int_fast8_t row = 0; row < MATRIX_CAPSENSE_ROWS; ++row) {
            matrix_row_t columns = raw_matrix[row];
            raw_matrix[row] = 0;
            while (columns) {
                ++active_key_count;
                columns &= columns - 1;
            }
        }
        if (active_key_count != 0) {
            if (active_key_count <= CAPSENSE_CAL_SUSPICIOUS_KEY_COUNT_MAX) {
                // A few keys are down, skip calibration and use saved
                cal_flags |= CAPSENSE_CAL_FLAG_SKIPPED;
            } else {
                // Suspiciously many keys appear to be down, clear the save
                // and recalibrate
                cal_flags |= CAPSENSE_CAL_FLAG_UNRELIABLE;
                clear_saved_matrix_calibration();
            }
        }
    }

    if (calibration_skipped) {
        clear_matrix();
    } else {
        calibrate_matrix();
    }

#if CAPSENSE_CAL_DEBUG
    cal_time = timer_elapsed(cal_time);
#endif

#if CAPSENSE_CAL_AUTOSAVE
    if ((cal_flags & (CAPSENSE_CAL_FLAG_CALIBRATED | CAPSENSE_CAL_FLAG_UNRELIABLE | CAPSENSE_CAL_FLAG_LOADED | CAPSENSE_CAL_FLAG_SAVED)) == CAPSENSE_CAL_FLAG_CALIBRATED) {
        // Calibration was done reliably and isn't already saved, save it
        save_matrix_calibration();
    }
#endif
#endif // ^ CAPSENSE_CAL_ENABLED
}
