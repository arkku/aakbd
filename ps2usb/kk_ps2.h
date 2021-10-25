/**
 * kk_ps2.h: A PS/2 host library for AVR
 *
 * Copyright (c) 2020-2021 Kimmo Kulovesi, https://arkku.dev/
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

/// The reply timeout for PS/2 commands, in milliseconds. Set to zero
/// to disable the timeout entirely.  The default is 100 ms, which is
/// relatively generous compared to the 20 ms specified.
extern uint8_t ps2_reply_timeout_ms;

/// Successfully received and recognized previous byte.
#define PS2_REPLY_ACK               ((uint8_t) 0xFAU)

/// Error in previous byte (e.g., invalid command/argument).
#define PS2_REPLY_ERROR             ((uint8_t) 0xFCU)

/// Request to resend the previous byte.
#define PS2_REPLY_RESEND            ((uint8_t) 0xFEU)

// The power-on/reset test has passed successfully.
#define PS2_REPLY_TEST_PASSED       ((uint8_t) 0xAAU)

// MARK: - Commands

/// Reset the device.
#define PS2_COMMAND_RESET           ((uint8_t) 0xFFU)

/// Request resend of the previous packet.
#define PS2_COMMAND_RESEND          ((uint8_t) 0xFEU)

/// Request id (mouse replies with one byte, keyboard with two).
#define PS2_COMMAND_ID              ((uint8_t) 0xF2U)

/// Enable reporting.
#define PS2_COMMAND_ENABLE          ((uint8_t) 0xF4U)

/// Disable reporting.
#define PS2_COMMAND_DISABLE         ((uint8_t) 0xF5U)

/// Set the keyboard mode / scan code set. The argument is a single byte
/// The argument is a scan code set number (1, 2 or 3), or 0 in which case the
/// mode is unchanged but the current mode is echoed back.
#define PS2_COMMAND_SET_SCAN_CODES  ((uint8_t) 0xF0U)

/// Sets a mouse to "remote" mode, in which case the mouse no longer sends
/// updates, but rather the status must be requested by the host with the
/// command `PS2_COMMAND_STATUS`.
#define PS2_COMMAND_SET_REMOTE_MODE ((uint8_t) 0xF0U)

/// Sets a mouse to "stream" mode (the default), in which mode the mouse
/// sends status updates automatically.
#define PS2_COMMAND_STREAM_MODE     ((uint8_t) 0xEAU)

/// Request status.
#define PS2_COMMAND_STATUS          ((uint8_t) 0xE9U)

/// Set the reporting rate.
/// For a mouse, the argument is the number of updates per second from 10 to
/// 200.  For a keyboard, takes two argument bytes, the first of which is
/// a repeat rate from 0x00 (fastest) to 0x1F (slowest), and the second is
/// the delay between repeats from 0x00 (shortest) to 0x03 (longest).
#define PS2_COMMAND_SET_RATE        ((uint8_t) 0xF3U)

/// Set the keyboard's LEDs. The argument is a bit mask of LED states,
/// the least significant bits being:
///
/// - bit 0: scroll lock
/// - bit 1: num lock
/// - bit 2: caps lock
#define PS2_COMMAND_SET_LEDS        ((uint8_t) 0xEDU)

/// Scroll lock's bit mask in the argument of `PS2_COMMAND_SET_LEDS`.
#define PS2_LED_SCROLL_LOCK_BIT     ((uint8_t) 1U)

/// Num lock's bit mask in the argument of `PS2_COMMAND_SET_LEDS`.
#define PS2_LED_NUM_LOCK_BIT        ((uint8_t) 2U)

/// Caps lock's bit mask in the argument of `PS2_COMMAND_SET_LEDS`.
#define PS2_LED_CAPS_LOCK_BIT       ((uint8_t) 4U)

/// Enable mouse scaling (acceleration).
#define PS2_COMMAND_DISABLE_SCALING ((uint8_t) 0xE6U)

/// Disable mouse scaling (acceleration).
#define PS2_COMMAND_ENABLE_SCALING  ((uint8_t) 0xE7U)

/// Set the mouse resolution.
#define PS2_COMMAND_SET_RESOLUTION  ((uint8_t) 0xE8U)

#define PS2_RESOLUTION_1_MM         ((uint8_t) 0x00U)
#define PS2_RESOLUTION_2_MM         ((uint8_t) 0x01U)
#define PS2_RESOLUTION_4_MM         ((uint8_t) 0x02U)
#define PS2_RESOLUTION_8_MM         ((uint8_t) 0x03U)

/// Set all keys to their normal make/break/typematic repeat in mode 3.
#define PS2_COMMAND_SET_ALL_KEYS_NORMAL     ((uint8_t) 0xFAU)

/// Set all keys to make only (no break/repeat).
#define PS2_COMMAND_SET_ALL_KEYS_MAKE       ((uint8_t) 0xF9U) 

/// Set all keys to make/break (no repeat).
#define PS2_COMMAND_SET_ALL_KEYS_MAKE_BREAK ((uint8_t) 0xF8U) 

/// Set all keys to make/repeat (no break).
#define PS2_COMMAND_SET_ALL_KEYS_TYPEMATIC  ((uint8_t) 0xF7U) 

/// Followed by set 3 make codes, disable break and repeat
/// for the list of keys to follow. List terminated by an invalid
/// make code, e.g., another command.
#define PS2_COMMAND_SET_KEY_MAKE        ((uint8_t) 0xFDU)

/// Followed by set 3 make codes, disable repeat (but not break) 
/// for the list of keys to follow. List terminated by an invalid
/// make code, e.g., another command.
#define PS2_COMMAND_SET_KEY_MAKE_BREAK  ((uint8_t) 0xFCU)

/// Followed by set 3 make codes, disable break (but not repeat)
/// for the list of keys to follow. List terminated by an invalid
/// make code, e.g., another command.
#define PS2_COMMAND_SET_KEY_TYPEMATIC   ((uint8_t) 0xFBU)

/// Requests a keyboard to echo back the same command. This can be
/// useful for detecting device removal or invalid state. For mice,
/// requests to enter echo mode (aka wrap mode), in which the mouse
/// will ignore commands other than `PS2_COMMAND_CLEAR_ECHO` and
/// `PS2_COMMAND_RESET`, and instead echo back every byte received
/// until either of those commands is given.)
#define PS2_COMMAND_ECHO                ((uint8_t) 0xEEU)

/// Clears the echo mode if set (and supported). For mice only.
#define PS2_COMMAND_CLEAR_ECHO          ((uint8_t) 0xECU)

// MARK: - Errors

/// Parity error
#define PS2_ERROR_PARITY            'P'

/// Write was requested but did not begin
#define PS2_ERROR_WRITE_BEGIN       'W'

/// Write not acknowledged at the end
#define PS2_ERROR_WRITE_END         'w'

/// Incorrect start bit
#define PS2_ERROR_START_BIT         'S'

/// Incorrect stop bit
#define PS2_ERROR_STOP_BIT          's'

/// PS/2 was busy too long
#define PS2_ERROR_BUSY              'B'

/// PS/2 command did not succeed
#define PS2_ERROR_COMMAND           'C'

// MARK: - Functions

/// Enable the PS/2 host. Reading will commence on the PS/2 CLK signal.
/// This must be called before other `ps2_*` functions. It may also be
/// called again to recover from an error state.
void ps2_enable(void);

/// Is PS/2 ok? Returns `false` if there is an error state. Note that the
/// library does not automatically recover from error states. One recovery
/// option is to simply restart by calling `ps2_enable()` again.
bool ps2_is_ok(void);

/// Send a byte of `data` to the PS/2 device. If `flush_input` is true,
/// then also discards any unread input from the buffer so that any further
/// input will have been sent after sending. Return `true` iff successful
/// in sending (the reply is not read, i.e., the device may still indicate
/// a receive error). See also `ps2_command`.
bool ps2_send(const uint8_t data, const bool flush_input);

/// Send a byte of `data` to the PS/2 device. Return `true` iff successful.
#define ps2_send_byte(data) ps2_send((data), false)

/// Send the single-byte `command` to the PS/2 device, and return its
/// reply. This causes any unread input to be flushed from the buffer.
/// The reply is typically one of the `PS2_REPLY_*` values. The command will
/// automatically be retried if requested.
int ps2_command(const uint8_t command);

/// Send the byte `command` and its argument byte `arg` to the PS/2 device, and
/// return its reply. The reply is typically one of the `PS2_REPLY_*` values.
/// The command will automatically be retried if requested.
int ps2_command_arg(const uint8_t command, const uint8_t arg);

/// Send the single-byte `command` to the PS/2 device.
/// Returns `true` iff the reply is `PS2_REPLY_ACK`.
bool ps2_command_ack(const uint8_t command);

/// Send the byte `command` and its argument byte `arg` to the PS/2 device,
/// Returns `true` iff the reply is `PS2_REPLY_ACK`.
bool ps2_command_arg_ack(const uint8_t command, const uint8_t arg);

/// Reads and returns a byte from the PS/2 device. This blocks until a byte
/// is available to read (or until timed out, in which case `EOF` is returned).
int ps2_recv(void);

/// Reads and returns a byte from the PS/2 device. If no byte is available
/// within the specified number of `milliseconds`, returns `EOF`.
int ps2_recv_timeout(const uint8_t milliseconds);

/// Returns the number of bytes available to read from PS/2.
uint8_t ps2_bytes_available(void);

/// Returns the next available byte, or `EOF` if none available.
int ps2_get_byte(void);

/// Returns the next available byte without consuming it.
int ps2_peek_byte(void);

/// Discard any unread bytes from the input buffer.
void ps2_flush_input(void);

/// Send a request to re-send the last byte received.
#define ps2_request_resend()    ps2_send(PS2_COMMAND_RESEND, true)

/// Returns a single charater identifying the last error that occurred.
/// This is only valid while `ps2_is_ok` is `false`.
char ps2_last_error(void);

#endif
