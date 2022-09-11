/**
 * xwhatsit.c: An alternative firmware for xwhatsit-based keyboards, such as
 * the brand new model F keyboards.
 *
 * Copyright (c) 2022 Kimmo Kulovesi, https://arkku.dev/
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

#include <stdbool.h>
#include <stdint.h>

extern bool keyboard_scan_enabled;

#include <generic_hid.h>

#if ENABLE_GENERIC_HID_ENDPOINT
bool
make_generic_hid_report (uint8_t report_id, uint8_t count, uint8_t report[static count]) {
    return false;
}
#endif
