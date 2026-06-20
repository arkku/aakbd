/*
 * suspend_core.c: Suspend helpers from QMK, adapted for AAKBD.
 *
 * Copyright 2022 QMK
 * Copyright 2022 Kimmo Kulovesi
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

#include "suspend.h"
#include "matrix.h"
#include <qmk_port.h>

__attribute__((weak)) void matrix_power_up(void) {}
__attribute__((weak)) void matrix_power_down(void) {}

__attribute__((weak)) void suspend_power_down_user(void) {}

__attribute__((weak)) void suspend_power_down_kb(void) {
    suspend_power_down_user();
}

__attribute__((weak)) void suspend_wakeup_init_user(void) {}

__attribute__((weak)) void suspend_wakeup_init_kb(void) {
    suspend_wakeup_init_user();
}

bool suspend_wakeup_condition(void) {
    matrix_power_up();
    matrix_scan();
    matrix_power_down();
    return matrix_has_keys_pressed();
}
