/**
 * usb_hardware.h: USB hardware abstraction layer.
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

#ifndef KK_USB_HARDWARE_H
#define KK_USB_HARDWARE_H

#include <stdbool.h>
#include <stdint.h>

// MARK: - Lifecycle

/// Initialise the USB system.
void usb_init(void);

/// Must be called from the main loop to run the USB system.
void usb_tick(void);

/// De-initialise the USB system.
void usb_deinit(void);

// MARK: - USB State

/// Is USB configured and was the last operation a success?
bool usb_is_ok(void);

/// Is USB configured? (Returns the configuration number.)
uint8_t usb_is_configured(void);

/// Zero if the last USB operation was a success, a non-zero code otherwise.
uint8_t usb_last_error(void);

/// Is USB suspended?
bool usb_is_suspended(void);

/// Is USB host requesting detach (e.g., for firmware update)?
/// If yes, this is the nearest positive 8-bit value of the detach timeout
/// requested (in milliseconds).
uint8_t usb_detach_requested(void);

/// Wake up the USB host.
bool usb_wake_up_host(void);

/// The USB address of the device.
uint8_t usb_address(void);

#endif
