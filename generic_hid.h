/**
 * generic_hid.h: USB Generic HID endpoint.
 *
 * The idea here is that the code is relatively easy to port to
 * another platform by substituting this header and a few other
 * things.
 *
 * Copyright (c) 2022-2026 Kimmo Kulovesi, https://arkku.dev/
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

#ifndef KK_GENERIC_HID_H
#define KK_GENERIC_HID_H

#if VIAL_ENABLE
#include "vial_config.h"
#endif

#ifndef ENABLE_GENERIC_HID_ENDPOINT
#ifndef ENABLE_GENERIC_HID_OUTPUT
#define ENABLE_GENERIC_HID_OUTPUT 0
#endif
#define ENABLE_GENERIC_HID_ENDPOINT 0
#endif

/// By defining `ENABLE_GENERIC_HID_ENDPOINT` as 1, a device may get an
/// additional HID endpoint for use with things like debugging and
/// interfacing with a configuration software. The device library must then
/// implement `handle_generic_hid_report` and `make_generic_hid_report`.
#if ENABLE_GENERIC_HID_ENDPOINT

#include <stdbool.h>
#include <stdint.h>

#ifndef GENERIC_HID_USAGE_PAGE
#define GENERIC_HID_USAGE_PAGE              0xFFABU
#endif
#ifndef GENERIC_HID_USAGE
#define GENERIC_HID_USAGE                   0x0001
#endif
#ifndef GENERIC_HID_INPUT_USAGE
#define GENERIC_HID_INPUT_USAGE             0x01
#endif
#ifndef GENERIC_HID_OUTPUT_USAGE
#define GENERIC_HID_OUTPUT_USAGE            0x02
#endif
#ifndef GENERIC_HID_REPORT_SIZE
/// The "input" (i.e., from keyboard to computer) report size.
#define GENERIC_HID_REPORT_SIZE             8
#endif
#ifndef GENERIC_HID_FEATURE_SIZE
/// The "output" feature (i.e., from computer to keyboard) report size.
#define GENERIC_HID_FEATURE_SIZE            1
#endif
#ifndef GENERIC_HID_POLL_INTERVAL_MS
#define GENERIC_HID_POLL_INTERVAL_MS        255
#endif
#ifndef GENERIC_HID_UPDATE_IDLE_MS
#define GENERIC_HID_UPDATE_IDLE_MS          0
#endif
#ifndef GENERIC_HID_ENDPOINT_IN_NUM
#define GENERIC_HID_ENDPOINT_IN_NUM         2
#endif
#ifndef ENABLE_GENERIC_HID_OUTPUT
#define ENABLE_GENERIC_HID_OUTPUT           0
#endif
#if ENABLE_GENERIC_HID_OUTPUT
#ifndef GENERIC_HID_ENDPOINT_OUT_NUM
#define GENERIC_HID_ENDPOINT_OUT_NUM        (GENERIC_HID_ENDPOINT_IN_NUM + 1)
#endif
#endif

#define RESPONSE_OK                 (0)
#define RESPONSE_SEND_REPLY         (1)
#define RESPONSE_JUMP_TO_BOOTLOADER (2)
#define RESPONSE_ERROR              (3)

/// Called _from the interrupt handler_ when a report is been received on the
/// generic HID endpoint. Returns one of the responses defined above. If this
/// should cause a response to be sent, the function may return
/// `RESPONSE_SEND_REPLY` and set `*response_length` to the length of the
/// response. The initial value of `*response_length` is the maximum.
///
/// For more complicated generic HID with `ENABLE_GENERIC_HID_OUTPUT` enabled,
/// the input endpoint is polled synchronously and this is not called from an
/// interrupt handler.
uint8_t handle_generic_hid_report(uint8_t report_id, uint8_t count, uint8_t report[static count], uint8_t response_length[static 1], uint8_t response[static *response_length]);

/// Called _from the interrupt handler_ when a report is been requested on the
/// generic HID endpoint. Must fill `report` with `count` bytes. The array
/// `report` is uninitialised! Returns `true` on success, `false` on error.
bool make_generic_hid_report(uint8_t report_id, uint8_t count, uint8_t report[static count]);

/// This can be called to send a report on the generic HID endpoint. Returns
/// `true` on success, `false` on error.
bool send_generic_hid_report(uint8_t report_id, uint8_t count, const uint8_t report[static count]);

/// Call `make_generic_hid_report` and send the report, if one is returned.
bool make_and_send_generic_hid_report(void);

#else
#if !defined(ENABLE_GENERIC_HID_OUTPUT) || ENABLE_GENERIC_HID_OUTPUT
#undef ENABLE_GENERIC_HID_OUTPUT
#define ENABLE_GENERIC_HID_OUTPUT 0
#endif
#endif
#endif
