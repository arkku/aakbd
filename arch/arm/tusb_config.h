/**
 * tusb_config.h: TinyUSB configuration for ARM AAKBD.
 *
 * Copyright (c) 2026 Kimmo Kulovesi
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

#ifndef TUSB_CONFIG_H
#define TUSB_CONFIG_H

#include "usbkbd_config.h"

#define CFG_TUSB_OS               OPT_OS_NONE
#define CFG_TUD_ENABLED           1

#ifndef CFG_TUSB_DEBUG
#define CFG_TUSB_DEBUG            0
#endif

/* Number of HID instances: keyboard (always), plus one if generic HID is enabled */
#if ENABLE_GENERIC_HID_ENDPOINT
#define CFG_TUD_HID               2
#define CFG_TUD_HID_INTERFACE_COUNT 2
#else
#define CFG_TUD_HID               1
#define CFG_TUD_HID_INTERFACE_COUNT 1
#endif

/* DFU runtime: follows AAKBD's ENABLE_DFU_INTERFACE flag */
#if ENABLE_DFU_INTERFACE
#define CFG_TUD_DFU_RUNTIME       1
#endif

/* Endpoint sizes */
#define CFG_TUD_ENDPOINT0_SIZE    64

/* HID buffer sizes */
#define CFG_TUD_HID_BUFSIZE       64
#define CFG_TUD_HID_EP_BUFSIZE    64

#endif
