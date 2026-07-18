/**
 * kk_ps2.h: PS/2 protocol constants.
 *
 * Copyright (c) 2020-2026 Kimmo Kulovesi, https://arkku.dev/
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

#ifndef KK_PS2_H
#define KK_PS2_H

#include <stdint.h>
#include <stdbool.h>

// MARK: - Replies

#define PS2_REPLY_ACK              ((uint8_t) 0xFAU)
#define PS2_REPLY_ERROR            ((uint8_t) 0xFCU)
#define PS2_REPLY_INTERNAL_FAILURE ((uint8_t) 0xFDU)
#define PS2_REPLY_RESEND           ((uint8_t) 0xFEU)
#define PS2_REPLY_KEYBOARD_ERROR   ((uint8_t) 0xFFU)
#define PS2_REPLY_TEST_PASSED      ((uint8_t) 0xAAU)

// MARK: - Commands

#define PS2_COMMAND_ID                      ((uint8_t) 0xF2U)
#define PS2_COMMAND_SET_RATE                ((uint8_t) 0xF3U)
#define PS2_COMMAND_ENABLE                  ((uint8_t) 0xF4U)
#define PS2_COMMAND_DISABLE                 ((uint8_t) 0xF5U)
#define PS2_COMMAND_SET_DEFAULTS            ((uint8_t) 0xF6U)
#define PS2_COMMAND_SET_ALL_KEYS_TYPEMATIC  ((uint8_t) 0xF7U)
#define PS2_COMMAND_SET_ALL_KEYS_MAKE_BREAK ((uint8_t) 0xF8U)
#define PS2_COMMAND_SET_ALL_KEYS_MAKE       ((uint8_t) 0xF9U)
#define PS2_COMMAND_SET_ALL_KEYS_NORMAL     ((uint8_t) 0xFAU)
#define PS2_COMMAND_SET_KEY_MAKE            ((uint8_t) 0xFDU)
#define PS2_COMMAND_SET_KEY_MAKE_BREAK      ((uint8_t) 0xFCU)
#define PS2_COMMAND_SET_KEY_TYPEMATIC       ((uint8_t) 0xFBU)
#define PS2_COMMAND_SET_SCAN_CODES          ((uint8_t) 0xF0U)
#define PS2_COMMAND_SET_REMOTE_MODE         ((uint8_t) 0xF0U)
#define PS2_COMMAND_STREAM_MODE             ((uint8_t) 0xEAU)
#define PS2_COMMAND_STATUS                  ((uint8_t) 0xE9U)
#define PS2_COMMAND_SET_LEDS                ((uint8_t) 0xEDU)
#define PS2_COMMAND_ECHO                    ((uint8_t) 0xEEU)
#define PS2_COMMAND_CLEAR_ECHO              ((uint8_t) 0xECU)
#define PS2_COMMAND_RESET                   ((uint8_t) 0xFFU)
#define PS2_COMMAND_RESEND                  ((uint8_t) 0xFEU)

#define PS2_COMMAND_DISABLE_SCALING ((uint8_t) 0xE6U)
#define PS2_COMMAND_ENABLE_SCALING  ((uint8_t) 0xE7U)
#define PS2_COMMAND_SET_RESOLUTION  ((uint8_t) 0xE8U)

#define PS2_COMMAND_RANGE_START ((uint8_t) 0xE6U)

// MARK: - Keyboard

#ifndef PS2_KEYBOARD_DEFAULT_SCANCODE_SET
/// The default scancode set should be 2 according to specs, but if you wish
/// to simulate an XT keyboard you could try changing this to 1 (and
/// also setting the `PS2_DEVICE_ID` to zero).
#define PS2_KEYBOARD_DEFAULT_SCANCODE_SET 2
#endif

#define PS2_KEYBOARD_DEFAULT_REPEAT_RATE ((uint8_t) 0x2BU)
#define PS2_EXT_PREFIX                   ((uint8_t) 0xE0U)
#define PS2_BREAK_PREFIX                 ((uint8_t) 0xF0U)
#define PS2_PAUSE_PREFIX                 ((uint8_t) 0xE1U)
#define PS2_LED_SCROLL_LOCK_BIT          ((uint8_t) 1U)
#define PS2_LED_NUM_LOCK_BIT             ((uint8_t) 2U)
#define PS2_LED_CAPS_LOCK_BIT            ((uint8_t) 4U)

// MARK: - Mouse

#define PS2_RESOLUTION_1_MM ((uint8_t) 0x00U)
#define PS2_RESOLUTION_2_MM ((uint8_t) 0x01U)
#define PS2_RESOLUTION_4_MM ((uint8_t) 0x02U)
#define PS2_RESOLUTION_8_MM ((uint8_t) 0x03U)

// MARK: - Errors

#define PS2_ERROR_PARITY          'P'
#define PS2_ERROR_WRITE_BEGIN     'W'
#define PS2_ERROR_WRITE_END       'w'
#define PS2_ERROR_START_BIT       'S'
#define PS2_ERROR_STOP_BIT        's'
#define PS2_ERROR_BUSY            'B'
#define PS2_ERROR_COMMAND         'C'
#define PS2_ERROR_BUFFER_OVERFLOW 'Q'

#endif
