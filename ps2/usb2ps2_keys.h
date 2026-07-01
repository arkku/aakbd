/**
 * usb2ps2_keys.h: USB to PS/2 scancode lookup.
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
#ifndef KK_USB2PS2_KEYS_H
#define KK_USB2PS2_KEYS_H

#include <stdint.h>
#include <stdbool.h>

#include "usbkbd_config.h"

/// Look up the scancode for a USB keycode in the given scancode set.
/// Returns the scancode byte, or 0 if unmapped.
uint8_t ps2_scancode_for_usb_keycode(uint8_t usb_keycode, uint8_t set);

/// Returns true if the key needs an E0 prefix in the given set.
bool is_extended_ps2_key(uint8_t usb_keycode, uint8_t set);

#endif
