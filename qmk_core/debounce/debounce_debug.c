/*
 * debounce_debug.c: Debounce statistics collector.
 *
 * Designed for global debounce algorithms (e.g., sym_defer_g): tracks the
 * longest interval between successive raw changes of the same key event.
 * This is the minimum debounce time needed — if the longest bounce gap
 * is 3ms, DEBOUNCE=4 would have been sufficient.
 *
 * Copyright 2026 Kimmo Kulovesi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "debounce_debug.h"
#include "progmem.h"
#include "config.h"
#include "timer.h"
#include "usbkbd.h"

#if DEBOUNCE == 0
#error "DEBOUNCE_DEBUG requires DEBOUNCE > 0"
#endif

#if !defined(DEBOUNCE_DEBUG) || !DEBOUNCE_DEBUG
#error "debounce_debug.c should only be compiled when DEBOUNCE_DEBUG is enabled"
#endif

static uint8_t histogram[DEBOUNCE];

// Per-key: timestamp of the most recent raw change within the current event
static uint16_t last_change[MATRIX_ROWS * MATRIX_COLS];

// Per-key: longest interval (ms) between consecutive raw changes in this event
static uint8_t max_gap[MATRIX_ROWS * MATRIX_COLS];

static matrix_row_t last_raw[MATRIX_ROWS];
static matrix_row_t prev_cooked[MATRIX_ROWS];
static matrix_row_t event_active[MATRIX_ROWS];
static matrix_row_t was_stable[MATRIX_ROWS];

void debounce_debug_update(matrix_row_t raw[], matrix_row_t cooked[], uint8_t rows) {
    uint16_t now = timer_read();

    for (uint8_t r = 0; r < rows; r++) {
        matrix_row_t raw_changed = last_raw[r] ^ raw[r];

        if (raw_changed) {
            for (uint8_t c = 0; c < MATRIX_COLS; c++) {
                matrix_row_t bit = (matrix_row_t)1 << c;
                if (raw_changed & bit) {
                    uint8_t idx = r * MATRIX_COLS + c;

                    if (event_active[r] & bit) {
                        // Subsequent change — measure gap from last change
                        uint16_t gap = TIMER_DIFF_16(now, last_change[idx]);
                        if (gap > max_gap[idx]) {
                            max_gap[idx] = (gap <= 255) ? (uint8_t)gap : 255;
                        }
                    } else {
                        // First change of a new event
                        event_active[r] |= bit;
                        max_gap[idx] = 0;
                    }
                    last_change[idx] = now;
                }
            }
            last_raw[r] = raw[r];
        }

        matrix_row_t cooked_changed = prev_cooked[r] ^ cooked[r];

        if (cooked_changed) {
            for (uint8_t c = 0; c < MATRIX_COLS; c++) {
                matrix_row_t bit = (matrix_row_t)1 << c;
                if (cooked_changed & bit) {
                    uint8_t idx = r * MATRIX_COLS + c;

                    if (max_gap[idx] > 0) {
                        uint8_t need = max_gap[idx] + 1;

                        // Bouncing detected: increment thresholds too short.
                        // If max_gap=3, need=4: thresholds 0,1,2,3 are insufficient.
                        uint8_t max_bucket = (need < DEBOUNCE) ? need : DEBOUNCE;
                        for (uint8_t t = 0; t < max_bucket; t++) {
                            if (histogram[t] < 255) {
                                histogram[t]++;
                            }
                        }
                    }

                    event_active[r] &= ~bit;
                }
            }
        }

        // Suppressed events: key idle two scans in a row without cooked change
        matrix_row_t stable = ~(raw[r] ^ prev_cooked[r]);
        matrix_row_t now_stable = event_active[r] & stable;
        event_active[r] &= ~(was_stable[r] & now_stable);
        was_stable[r] = now_stable;

        prev_cooked[r] = cooked[r];
    }
}

void debounce_debug_print_histogram(void) {
    fprintf_P(usb_kbd_type, PSTR("D:"));
    for (uint8_t i = 0; i < DEBOUNCE; i++) {
        fprintf_P(usb_kbd_type, PSTR(" %u=%u"), i, histogram[i]);
    }

    uint8_t highest = 0;
    for (uint8_t i = 0; i < DEBOUNCE; i++) {
        if (histogram[i] > 0) {
            highest = i;
        }
    }
    uint8_t need = highest + 1;
    if (need > DEBOUNCE) need = DEBOUNCE;
    fprintf_P(usb_kbd_type, PSTR(" need=%u\n"), need);
}
