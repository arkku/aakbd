/**
 * vial_config.h: Vial-specific configuration for AAKBD.
 *
 * Copyright (c) 2026 Kimmo Kulovesi, https://arkku.dev/
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

#include <stdint.h>

#if !defined(ENABLE_GENERIC_HID_ENDPOINT) || !ENABLE_GENERIC_HID_ENDPOINT
#define ENABLE_GENERIC_HID_ENDPOINT 1
#endif
#define ENABLE_GENERIC_HID_OUTPUT   1
#define GENERIC_HID_USAGE_PAGE      0xFF60U
#define GENERIC_HID_USAGE           0x61
#define GENERIC_HID_INPUT_USAGE     0x61
#define GENERIC_HID_OUTPUT_USAGE    0x61
#ifndef GENERIC_HID_REPORT_SIZE
#define GENERIC_HID_REPORT_SIZE 32
#endif
#ifndef GENERIC_HID_FEATURE_SIZE
#define GENERIC_HID_FEATURE_SIZE 32
#endif

_Static_assert(GENERIC_HID_REPORT_SIZE == 32, "Vial requires GENERIC_HID_REPORT_SIZE == 32");
_Static_assert(GENERIC_HID_FEATURE_SIZE == 32, "Vial requires GENERIC_HID_FEATURE_SIZE == 32");

#ifndef GENERIC_HID_POLL_INTERVAL_MS
#define GENERIC_HID_POLL_INTERVAL_MS 2
#endif

#include "progmem.h"

extern const uint16_t vial_default_layout_options PROGMEM;
extern const uint8_t vial_keyboard_uid[8] PROGMEM;
