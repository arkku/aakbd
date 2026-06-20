#pragma once

#include <stdint.h>
#include "matrix.h"

void debounce_debug_update(matrix_row_t raw[], matrix_row_t cooked[], uint8_t rows);

void debounce_debug_print_histogram(void);
