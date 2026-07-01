/**
 * kk_ps2_host.h: Host-mode PS/2 functions.
 *
 * Copyright (c) 2026 Kimmo Kulovesi, https://arkku.dev/
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
#ifndef KK_PS2_HOST_H
#define KK_PS2_HOST_H

#include "kk_ps2.h"

extern uint8_t ps2_reply_timeout_ms;

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
#define ps2_request_resend() ps2_send(PS2_COMMAND_RESEND, true)

/// Returns a single charater identifying the last error that occurred.
/// This is only valid while `ps2_is_ok` is `false`.
char ps2_last_error(void);

#endif
