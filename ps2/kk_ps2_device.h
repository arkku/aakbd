/**
 * kk_ps2_device.h: Device-mode PS/2 functions.
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
#ifndef KK_PS2_DEVICE_H
#define KK_PS2_DEVICE_H

#include "kk_ps2.h"

/// Initialise the PS/2 pins as inputs with pull-ups for device mode.
/// Must be called before any other `ps2_device_*` functions.
void ps2_device_init(void);

/// Return `true` if a PS/2 host is detected (both CLK and DATA high
/// from host-side pull-ups). (Or, if `PS2_ENABLE_PIN` is configured,
/// that pin, active low, may override the detection to `true`, but not
/// to `false`.) This is meant to be called after `ps2_device_init()`, but
/// before `ps2_device_attach()`.
bool ps2_device_host_detected(void);

/// Must be called after `p2_device_init()` before actually starting to use
/// the PS/2 device (e.g., after `ps2_device_host_detected()` confirms that
/// we can use the PS/2 bus).
void ps2_device_attach(void);

/// Shut down PS/2, set the PS/2 pins to high impedance.
void ps2_device_shutdown(void);

/// Send a byte to the host as a PS/2 device. Blocks until done,
/// generates the clock signal. Return `true` on success.
bool ps2_device_send(uint8_t data);

/// Receive a byte from the host as a PS/2 device. Blocks until
/// received, generates the clock signal. Returns the byte, or
/// `EOF` on error or timeout.
int ps2_device_recv(void);

/// Flush any queued output bytes by sending them to the host.
/// Returns `true` if all queued bytes were successfully sent.
bool ps2_device_flush(void);

/// Is there unsent uotput in the buffer?
bool ps2_device_has_pending_output(void);

/// Resend the last transmitted byte. Does not check for buffer overflow,
/// so in theory might send incorrect bytes, but with regular flushing that
/// should not happen.
void ps2_device_resend(void);

/// Discard any queued output bytes without sending them.
void ps2_device_clear_output(void);

/// Return the last error character (printable). This is only for debugging.
/// Note that the error is _not_ cleared automatically (because otherwise
/// other successes would hide past errors, as this is not meant to be polled
/// repeatedly): call `ps2_device_clear_error()` to clear it.
char ps2_device_last_error(void);

/// Clears the last error.
void ps2_device_clear_error();

#endif
