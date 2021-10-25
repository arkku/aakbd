/**
 * main.h: USB keyboard implementation for ATMEGA32U4.
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

#ifndef KK_USB_KBD_MAIN_H
#define KK_USB_KBD_MAIN_H

#include <stdint.h>

/// Jump to bootloader. Normally `usb_jump_to_bootloader()` should be used
/// instead, as it tears down the USB connection and then calls this.
void jump_to_bootloader(void);

/// Reset the physical keyboard. Also resets the USB keyboard and key processing.
void keyboard_reset(void);

/// The current 10 ms tick count, i.e., a monotonically increasing counter that
/// increments approximately once per 10 milliseconds.
uint8_t current_10ms_tick_count(void);

#endif
