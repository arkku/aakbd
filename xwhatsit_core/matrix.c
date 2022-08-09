/* Copyright 2020 Purdea Andrei
 * Copyright © 2022 Kimmo Kulovesi
 *
 * Ported from QMK to AAKBD. Any errors are almost certainly due to that.
 * Added support for saving calibration in EEPROM to enable faster startup
 * when keys are held down. Optimised behaviour when calibration bins are
 * unused.
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

#include "quantum.h"
#include "matrix_manipulate.h"
#include <string.h>
#include <stdbool.h>
#include <avr/eeprom.h>

#include <qmk_port.h>

bool keyboard_scan_enabled = true;

#if CAPSENSE_CAL_ENABLED
#define CAPSENSE_CAL_VERSION 1
#define EECONFIG_KEYBOARD ((uint32_t *) 15)
#define EECONFIG_CALIBRATION_DATA ((char *) (EECONFIG_KEYBOARD + 1))

struct calibration_header {
    uint8_t version;
    uint8_t cols;
uint8_t rows;
    uint8_t bins;
    uint16_t keymap_checksum;
};

uint16_t cal_thresholds[CAPSENSE_CAL_BINS] = { 0 };
matrix_row_t assigned_to_threshold[CAPSENSE_CAL_BINS][MATRIX_CAPSENSE_ROWS] = { { 0 } };
uint8_t cal_bin_key_count[CAPSENSE_CAL_BINS] = { 0 };
uint16_t cal_tr_all_zero = 0xFFFFU;
uint16_t cal_tr_all_one = 0x0000U;
uint16_t cal_keymap_checksum;
uint8_t cal_flags = 0;

#if CAPSENSE_CAL_DEBUG
uint16_t cal_time = 0;
#endif
#endif

static matrix_row_t previous_matrix[MATRIX_ROWS] = { 0 };

#if MATRIX_EXTRA_DIRECT_ROWS
static pin_t extra_direct_pins[MATRIX_EXTRA_DIRECT_ROWS][MATRIX_COLS] = MATRIX_EXTRA_DIRECT_PINS;
#endif

#define idelta(a, b) (((a) < (b)) ? (b) - (a) : (a) - (b))

#define scan_col_raw(col, interference) test_single((col), CAPSENSE_HARDCODED_SAMPLE_TIME, (interference))

#ifdef CAPSENSE_CONDUCTIVE_PLASTIC_IS_PULLED_UP_ON_KEYPRESS
#define scan_physical_col(col, interference) (~scan_col_raw((col), (interference)))
#else
#define scan_physical_col(col, interference) scan_col_raw((col), (interference))
#endif

#define SHIFT_BITS (((CAPSENSE_KEYMAP_COL_TO_PHYSICAL_COL(MATRIX_COLS - 1) >= 16) || \
                     (CAPSENSE_KEYMAP_COL_TO_PHYSICAL_COL(0) >= 16)) ? 24 : 16)

#define TRACKING_TEST_TIME 4
// Key 1 is some position that is unused in the matrix
#define TRACKING_KEY_1_COL 8
#define TRACKING_KEY_1_ROW 3
// Key 2 is the always-pressed calibration pad to the far right-bottom of the keyboard. (both on F62 and F77)
#define TRACKING_KEY_2_COL 15
#define TRACKING_KEY_2_ROW 6
// Key 3 is the F key
#define TRACKING_KEY_3_COL 2
#define TRACKING_KEY_3_ROW 5
// Key 4 is the half of the split backspace that is unused if the user has a normal backspace.
#define TRACKING_KEY_4_COL 7
#define TRACKING_KEY_4_ROW 3
// Key 5 is hidden key next to the left shift, which is only used in ISO layouts.
#define TRACKING_KEY_5_COL 0
#define TRACKING_KEY_5_ROW 7

#define TRACKING_REPS 16

static INLINE uint8_t read_rows(void) {
    CAPSENSE_READ_ROWS_LOCAL_VARS;
    asm volatile (CAPSENSE_READ_ROWS_ASM_INSTRUCTIONS : CAPSENSE_READ_ROWS_OUTPUT_CONSTRAINTS : CAPSENSE_READ_ROWS_INPUT_CONSTRAINTS);
    return CAPSENSE_READ_ROWS_VALUE;
}

#if defined(CAPSENSE_DAC_MCP4921)

static void dac_init(void) {
    writePin(CAPSENSE_DAC_NCS, 1);
    setPinOutput(CAPSENSE_DAC_NCS);
    setPinOutput(CAPSENSE_DAC_SCK);
    setPinOutput(CAPSENSE_DAC_SDI);
    writePin(CAPSENSE_DAC_NCS, 1);
    writePin(CAPSENSE_DAC_SCK, 0);
    writePin(CAPSENSE_DAC_SDI, 0);
}

void dac_write_threshold(uint16_t value) {
    const uint16_t buffered = 0;
    #define nSHDN_BIT 12
    value |= 1 << nSHDN_BIT; // nSHDN = 0 -- make sure output is not floating.
    #define MCP_DAC_GAIN_2X 0
    #define MCP_DAC_GAIN_1X 1
#define nGA_BIT 13
    value |= MCP_DAC_GAIN_1X << nGA_BIT;
    #define BUF_BIT 14;
    value |= buffered << BUF_BIT;

    writePin(CAPSENSE_DAC_NCS, 0);
    for (int_fast8_t i = 0; i < 16; ++i)
    {
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

uint8_t test_single(uint8_t col, uint16_t time, uint8_t *interference_ptr) {
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
            sum += (test_single(col, time, NULL) >> row) & 1;
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

#define calibration_measure_all_zero() calibration_measure_all_valid_keys(CAPSENSE_HARDCODED_SAMPLE_TIME, CAPSENSE_CAL_INIT_REPS, true)
#define calibration_measure_all_one() calibration_measure_all_valid_keys(CAPSENSE_HARDCODED_SAMPLE_TIME, CAPSENSE_CAL_INIT_REPS, false)

static uint16_t calibration_measure_all_valid_keys(uint8_t time, int_fast8_t samples, bool looking_for_all_zero) {
    uint16_t min = 0, max = CAPSENSE_DAC_MAX;
    do {
        const uint16_t mid = (min + max + !looking_for_all_zero) / 2;
        dac_write_threshold(mid);
        for (int_fast8_t col = 0; col < MATRIX_COLS; ++col) {
            // Find the rows that are actually mapped in this column
            uint8_t valid_physical_rows = 0;
            for (int_fast8_t row = 0; row < MATRIX_CAPSENSE_ROWS; ++row) {
                if (usb_keycode_for_matrix(col, row)) {
                    valid_physical_rows |= (((matrix_row_t) 1) << CAPSENSE_KEYMAP_ROW_TO_PHYSICAL_ROW(row));
                }
            }

            // Check if this threshold gives a stable desired result
            uint8_t physical_col = CAPSENSE_KEYMAP_COL_TO_PHYSICAL_COL(col);
            const uint8_t desired_result = looking_for_all_zero ? 0 : valid_physical_rows;
            for (int_fast8_t i = samples; i; --i) {
                const uint8_t result = test_single(physical_col, time, NULL) & valid_physical_rows;
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
    } while (min < max);
    return min;
}

void calibrate_matrix(void) {
    uint16_t cal_thresholds_max[CAPSENSE_CAL_BINS];
    uint16_t cal_thresholds_min[CAPSENSE_CAL_BINS];

    // Find the boundaries where all keys read as zero or one
    cal_tr_all_zero = calibration_measure_all_zero();
    cal_tr_all_one = calibration_measure_all_one();

    uint16_t max = (cal_tr_all_zero == 0) ? 0 : (cal_tr_all_zero - 1);
    uint16_t min = cal_tr_all_one + 1;
    if (max < min) {
        max = min;
    }

    const uint16_t delta = max - min;

    for (int_fast8_t bin = 0; bin < CAPSENSE_CAL_BINS; ++bin) {
        for (int_fast8_t row = 0; row < MATRIX_CAPSENSE_ROWS; row++) {
            assigned_to_threshold[bin][row] = 0;
        }
        cal_bin_key_count[bin] = 0;
        cal_thresholds_max[bin] = 0U;
        cal_thresholds_min[bin] = 0xFFFFU;

        // Set up the bins across the range between min and max
        cal_thresholds[bin] = min + ((delta * (2 * bin + 1)) / 2 / CAPSENSE_CAL_BINS);
    }

    // Measure each key and assign it to a bin
    matrix_row_t col_mask = 1;
    for (int_fast8_t col = 0; col < MATRIX_COLS; col++, col_mask <<= 1) {
        const uint8_t physical_col = CAPSENSE_KEYMAP_COL_TO_PHYSICAL_COL(col);
        for (int_fast8_t row = 0; row < MATRIX_CAPSENSE_ROWS; row++) {
            const uint8_t physical_row = CAPSENSE_KEYMAP_ROW_TO_PHYSICAL_ROW(row);

            if (usb_keycode_for_matrix(row, col)) { // Mapped keys only
                uint16_t threshold = measure_middle(physical_col, physical_row, CAPSENSE_HARDCODED_SAMPLE_TIME, CAPSENSE_CAL_EACHKEY_REPS);

                // Find the bin closest to this threshold
                int_fast8_t best_bin = 0;
                const uint16_t best_threshold = cal_thresholds[best_bin];
                uint16_t best_diff = idelta(threshold, best_threshold);

                for (int_fast8_t bin = 1; bin < CAPSENSE_CAL_BINS; ++bin) {
                    const uint16_t this_threshold = cal_thresholds[bin];
                    uint16_t this_diff = idelta(threshold, this_threshold);
                    if (this_diff < best_diff) {
                        best_diff = this_diff;
                        best_bin = bin;
                    }
                }

                // Assign to the best fitting bin
                assigned_to_threshold[best_bin][row] |= col_mask;
                cal_bin_key_count[best_bin] += 1;

                // Track the bin threshold range
                if (cal_thresholds_max[best_bin] < threshold) {
                    cal_thresholds_max[best_bin] = threshold;
                }
                if (cal_thresholds_min[best_bin] > threshold) {
                    cal_thresholds_min[best_bin] = threshold;
                }
            }
        }
    }

    // Assign the thresholds based on the actual keys in each bin
    for (int_fast8_t bin = 0; bin < CAPSENSE_CAL_BINS; ++bin) {
        uint_fast16_t bin_signal_level;

        if (cal_bin_key_count[bin] == 0) {
            bin_signal_level = cal_thresholds[bin];
        } else {
            // Take the average of the bin extremities
            bin_signal_level = cal_thresholds_max[bin] + cal_thresholds_min[bin];
            #ifdef CAPSENSE_CONDUCTIVE_PLASTIC_IS_PUSHED_DOWN_ON_KEYPRESS
            bin_signal_level += 1;
            #endif
            bin_signal_level /= 2;
        }

        // Offset the level so as to be more lenient with the signal
        // Note: I added the checks to use max/min when out of range, not sure
        // if that's a good idea or not. -KK
        #ifdef CAPSENSE_CONDUCTIVE_PLASTIC_IS_PUSHED_DOWN_ON_KEYPRESS
        bin_signal_level += CAPSENSE_CAL_THRESHOLD_OFFSET;
        if (bin_signal_level < cal_thresholds_max[bin]) {
            bin_signal_level = cal_thresholds_max[bin];
        }
        #else
        if (bin_signal_level < CAPSENSE_CAL_THRESHOLD_OFFSET) {
            bin_signal_level = 0;
        } else {
            bin_signal_level -= CAPSENSE_CAL_THRESHOLD_OFFSET;
        }
        if (bin_signal_level > cal_thresholds_min[bin]) {
            bin_signal_level = cal_thresholds_min[bin];
        }
        #endif
        if (bin_signal_level > CAPSENSE_DAC_MAX) {
            bin_signal_level = CAPSENSE_DAC_MAX;
        }
        cal_thresholds[bin] = bin_signal_level;
    }
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
    eeprom_read_block(assigned_to_threshold, p, sizeof(assigned_to_threshold));
    p += sizeof(assigned_to_threshold);
    eeprom_read_block(cal_bin_key_count, p, sizeof(cal_bin_key_count));
    p += sizeof(cal_bin_key_count);
    cal_tr_all_zero = eeprom_read_word((const uint16_t *) p);
    p += sizeof(cal_tr_all_zero);
    cal_tr_all_one = eeprom_read_word((const uint16_t *) p);
    p += sizeof(cal_tr_all_one);
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
    eeprom_update_byte(EECONFIG_CALIBRATION_DATA, 0);
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
    eeprom_update_block(assigned_to_threshold, p, sizeof(assigned_to_threshold));
    p += sizeof(assigned_to_threshold);
    eeprom_update_block(cal_bin_key_count, p, sizeof(cal_bin_key_count));
    p += sizeof(cal_bin_key_count);
    eeprom_update_word((uint16_t *) p, cal_tr_all_zero);
    p += sizeof(cal_tr_all_zero);
    eeprom_update_word((uint16_t *) p, cal_tr_all_one);
    p += sizeof(cal_tr_all_one);
    
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
            uint16_t keycode = usb_keycode_for_matrix(col, row);
            if (keycode) {
                checksum += ~keycode + (((keycode * (col + 1))) ^ (row + 1));
                checksum <<= 3;
                checksum >>= (16 - 3);
            }
        }
    }

    return ~checksum;
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
    for (int_fast8_t bin = 0; bin < CAPSENSE_CAL_BINS; ++bin) {
        uint8_t remaining_keys = cal_bin_key_count[bin];

        if (remaining_keys == 0) {
            goto no_more_keys_in_bin;
        }

        // Set the threshold of this bin and then scan all keys in the bin
        dac_write_threshold(cal_thresholds[bin]);

        matrix_row_t col_mask = 1;
        for (int_fast8_t col = 0; col < MATRIX_COLS; ++col, col_mask <<= 1) {
            const int_fast8_t physical_col = CAPSENSE_KEYMAP_COL_TO_PHYSICAL_COL(col);
            uint8_t col_data, interference;
            bool col_scanned = false;

            for (int_fast8_t row = 0; row < MATRIX_CAPSENSE_ROWS; ++row) {
                if (assigned_to_threshold[bin][row] & col_mask) {
                    if (!col_scanned) {
                        col_scanned = true;
                        col_data = scan_physical_col(physical_col, &interference);
                    }

                    const int_fast8_t physical_row = CAPSENSE_KEYMAP_ROW_TO_PHYSICAL_ROW(row);
                    const uint8_t physical_row_mask = (1 << physical_row);
                    const uint8_t key_is_on = col_data & physical_row_mask;

                    if (key_is_on && !(interference & physical_row_mask)) {
                        current_matrix[row] |= col_mask;
                    }

                    if (--remaining_keys == 0) {
                        goto no_more_keys_in_bin;
                    }
                }
            }
        }
        no_more_keys_in_bin:
            ;
    }
#else // ^ CAPSENSE_CAL_ENABLED
    for (int_fast8_t col = 0; col < MATRIX_COLS; ++col) {
        int_fast8_t real_col = CAPSENSE_KEYMAP_COL_TO_PHYSICAL_COL(col);
        uint8_t interference;
        uint8_t col_data = scan_physical_col(physical_col, &interference);
        for (int_fast8_t row = 0; row < MATRIX_CAPSENSE_ROWS; ++row) {
            current_matrix[CAPSENSE_PHYSICAL_ROW_TO_KEYMAP_ROW(row)] |= (((matrix_row_t)(col_data & 1)) << col);
            col_data >>= 1;
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
    SETUP_UNUSED_PINS();

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

#if CAPSENSE_CAL_ENABLED
#if CAPSENSE_CAL_DEBUG
    cal_time = timer_read();
#endif

    if (load_matrix_calibration()) {
        (void) matrix_scan_custom(raw_matrix);
        for (int_fast8_t row = 0; row < MATRIX_ROWS; ++row) {
            if (raw_matrix[row]) {
                // Keys pressed, skip calibration
                cal_flags |= CAPSENSE_CAL_FLAG_SKIPPED;
            }
            raw_matrix[row] = 0;
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
    if (!calibration_loaded && !calibration_saved) {
        for (int_fast8_t bin = 0; bin < CAPSENSE_CAL_BINS; ++bin) {
            if (cal_bin_key_count[bin] >= 1 && cal_bin_key_count[bin] <= 4) {
                // Suspicious calibration, could be held keys? Don't save.
                return;
            }
        }
        save_matrix_calibration();
    }
#endif
#else // ^ CAPSENSE_CAL_ENABLED
    dac_write_threshold(CAPSENSE_HARDCODED_THRESHOLD);
    dac_write_threshold(CAPSENSE_HARDCODED_THRESHOLD);
    dac_write_threshold(CAPSENSE_HARDCODED_THRESHOLD);
#endif
}
