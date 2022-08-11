/* Copyright 2016-2018 Erez Zukerman, Jack Humbert, Yiancar
 *
 * Almost all content removed for porting keyboards from QMK to AAKBD.
 * Any problem are due to this and not the fault of the original authors.
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
#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

#include "platform_deps.h"
#include "wait.h"
#include "matrix.h"
#include "led.h"

#include "timer.h"
#include "config.h"
#include "config_common.h"
#include "gpio.h"
#include "bitwise.h"
#include "print.h"

#include "keymap.h"
#include "eeconfig.h"

#include <aakbd.h>

extern const uint8_t keymaps[1][MATRIX_ROWS][MATRIX_COLS];

#ifdef XWHATSIT
#include "xwhatsit_port.h"
#endif
