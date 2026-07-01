/**
 * ps2_output.h: PS/2 output device interface.
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
#ifndef KK_PS2_OUTPUT_H
#define KK_PS2_OUTPUT_H

#include <stdint.h>
#include <stdbool.h>

#include "usbkbd_config.h"
#include "kk_ps2_device.h"
#include "usb2ps2_keys.h"

/// Initialise the PS/2 output (device mode). Must be called before use.
void ps2_output_init(void);

/// Poll for incoming host commands. Call periodically from the main loop.
/// This is safe to call even if PS/2 is not enabled, it will just do nothing.
/// - Note: Other PS/2 commands MUST NOT be called unless in PS/2 mode.
void ps2_output_task(void);

/// Has the PS/2 output been initialized? In general, this can be used as a
/// guard for detecting whether PS/2 functions can be called, with the
/// exception of `ps2_output_task()` which can be called unconditionally.
bool ps2_output_is_initialized(void);

/// Is keyboard scanning (reacting to keypresses) enabled?
/// This being `true` implies `ps2_output_is_initialized()`, so there is no
/// need to check both if checking this. This function is safe to call even
/// if PS/2 is not enabled.
bool ps2_output_is_scanning(void);

/// Is there an active host that we are communicating with?
/// This being `true` implies `ps2_output_is_initialized()`, so there is no
/// need to check both if checking this. This function is safe to call even
/// if PS/2 is not enabled.
bool ps2_output_is_active(void);

/// Returns `true` if the PS/2 output's event queue is clear. This can be
/// used to check whether key actions have actually been sent.
bool ps2_output_queue_is_clear(void);

/// Returns the LED state set by the PS/2 host. This is in the USB LED
/// bit order for convenience, *not* in the PS/2 LED bit order.
uint8_t ps2_host_leds(void);

/// Queue a key press event (USB keycode) to be sent later.
void ps2_press_key(uint8_t usb_keycode);

/// Queue a key release event (USB keycode) to be sent later.
void ps2_release_key(uint8_t usb_keycode);

/// Clear queued key events and key state.
/// - Parameter should_discard_unsent_keys: If `true`, the key event queue
/// is cleared entirely (discard all pending events). If `false`, all events
/// are kept and a sentinel is appended for final state cleanup. The caller
/// must have queued any needed release events before calling with `false`.
void ps2_output_clear_keys(bool should_discard_unsent_keys);

/// Shut down the PS/2 output (the inverse of `ps2_output_init()`).
void ps2_output_shutdown(void);

#endif
