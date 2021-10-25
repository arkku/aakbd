/**
 * ps2usb_keys.h: PS/2 set 3 keycode to USB mapping.
 *
 * Copyright (c) 2021 Kimmo Kulovesi, https://arkku.dev/
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

#ifndef KK_PS2_USB_KEYCODES_H
#define KK_PS2_USB_KEYCODES_H

#include <stdint.h>

/// Return the USB keycode corresponding to the same physical key in a
/// typical USB keyboard as the PS/2 key given as argument. This should _not_
/// be used for remapping - rather see `process_key` in `keys.h`.
uint8_t usb_keycode_for_ps2_keycode(const uint8_t ps2_code);

#endif
