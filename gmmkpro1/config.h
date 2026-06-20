/**
 * config.h: GMMK Pro rev1 device configuration.
 */

#pragma once

#define MATRIX_ROWS 11
#define MATRIX_COLS 8
#define DIODE_DIRECTION COL2ROW
#define MATRIX_ROW_PINS { B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, B10 }
#define MATRIX_COL_PINS { A0, A1, A2, A3, A4, A8, A9, A10 }

#ifndef DEBOUNCE
#define DEBOUNCE 5
#endif

// Rotary encoder: C14 (B), C15 (A)
#define ENCODER_A_PIN C15
#define ENCODER_B_PIN C14

// AW20216S RGB LED driver
#define AW20216S_EN_PIN         C13
#define AW20216S_CS_PIN_1       B13
#define AW20216S_CS_PIN_2       B14
#define AW20216S_CHIP_COUNT     2
