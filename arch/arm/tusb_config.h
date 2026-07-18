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
#include "generic_hid.h"

#define CFG_TUSB_OS               OPT_OS_NONE
#define CFG_TUD_ENABLED           1

/* Number of HID instances: keyboard (always), plus generic HID and/or consumer */
#define CFG_TUD_HID                (1 + ENABLE_GENERIC_HID_ENDPOINT + MEDIA_KEYS_ENDPOINT)
#define CFG_TUD_HID_INTERFACE_COUNT CFG_TUD_HID

/* DFU runtime: follows AAKBD's ENABLE_DFU_INTERFACE flag */
#if ENABLE_DFU_INTERFACE
#define CFG_TUD_DFU_RUNTIME       1
#endif

/* Endpoint sizes */
#define CFG_TUD_ENDPOINT0_SIZE    64

/* HID buffer sizes (must fit largest report across all instances).
 * When generic HID is disabled only the keyboard (8 bytes) matters. */
#if !ENABLE_GENERIC_HID_ENDPOINT
#define CFG_TUD_HID_BUFSIZE       8
#define CFG_TUD_HID_EP_BUFSIZE    8
#elif GENERIC_HID_REPORT_SIZE > GENERIC_HID_FEATURE_SIZE
#define CFG_TUD_HID_BUFSIZE       GENERIC_HID_REPORT_SIZE
#define CFG_TUD_HID_EP_BUFSIZE    GENERIC_HID_REPORT_SIZE
#else
#define CFG_TUD_HID_BUFSIZE       GENERIC_HID_FEATURE_SIZE
#define CFG_TUD_HID_EP_BUFSIZE    GENERIC_HID_FEATURE_SIZE
#endif

#endif
