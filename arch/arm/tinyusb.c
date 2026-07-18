/*
 * tinyusb.c: USB implementation using TinyUSB for ARM.
 *
 * This is the ARM/TinyUSB implementation of the USB hardware interface.
 * stack. It implements the usb_hardware.h interface and provides
 * usb_keyboard_send_report() via TinyUSB's HID device class driver.
 *
 * Descriptors are the existing ones from usbkbd_descriptors.c, looked
 * up via usb_descriptor_length_and_data() in the TinyUSB callbacks.
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

#include "platform_deps.h"
#define USB_KEYBOARD_ACCESS_STATE 1
#include "tinyusb.h"
#include "usbkbd.h"
#include "timer.h"

/* Keyboard HID report header + key state */

/* Keyboard LEDs: Num Lock=1, Caps Lock=2, Scroll Lock=4 */
#define LED_MASK_ALL            7

/* HID idle rate is in units of 4 ms (USB frame × 4, per HID spec) */
#define HID_IDLE_RATE_MS        4

/* Timeout for remote wakeup to complete, in ms (50 USB frames) */
#define WAKEUP_TIMEOUT_MS       50

/* Timeout for sending a report via tud_hid_n_report, in ms */
#define SEND_TIMEOUT_MS         100

/* Flag to jump to bootloader on next bus reset */
#define USB_STATUS_JUMP_TO_BOOTLOADER   (1 << 7)

/* DFU detach: delay 250ms before jumping to bootloader */
#define DFU_DETACH_DELAY_MS     250

#include "usbkbd_config.h"

#include "usb_hardware.h"
#include "usbkbd_descriptors.h"
#include "usbkbd_descriptors.h"
#include "aakbd.h"
#include "keys.h"
#include "generic_hid.h"
#if ENABLE_HOST_FINGERPRINT
#include "host_fingerprint.h"
#endif

// MARK: - TinyUSB Configuration

#ifndef TINYUSB_CFG_RHPORT
#define TINYUSB_CFG_RHPORT 0
#endif

// MARK: - USB State

static volatile uint8_t local_configuration = 0;
static volatile uint8_t usb_error = 0;
static volatile uint8_t usb_status = 0;
static volatile bool usb_suspended = false;

#if ENABLE_KEYBOARD_ENDPOINT
static uint32_t keyboard_last_report = 0;
static uint8_t keyboard_idle_rate = 0;
#endif

#if ENABLE_GENERIC_HID_ENDPOINT
static uint32_t generic_last_report = 0;
static uint8_t generic_idle_rate = 0;
#endif

#if MEDIA_KEYS_ENDPOINT
#define CONSUMER_HID_INSTANCE (1 + ENABLE_GENERIC_HID_ENDPOINT)
#endif

#if ENABLE_DFU_INTERFACE
static volatile uint8_t usb_request_detach = 0;
#ifndef DFU_DETACH_MAGIC
#define DFU_DETACH_MAGIC 0xAA
#endif
#endif

// MARK: - Descriptor Lookup

static uint8_t const *
find_descriptor (uint16_t value, uint16_t index) {
    const char *data = NULL;
    if (usb_descriptor_length_and_data(value, index, &data)) {
        return (uint8_t const *) data;
    }
    return NULL;
}

// MARK: - usb_hardware.h API

static void
update_virtual_leds (void) {
#if ENABLE_VIRTUAL_LEDS
    uint8_t v = 0;
    if (usb_keyboard_is_in_boot_protocol) {
        v |= LED_VIRTUAL_BOOT_PROTOCOL_BIT;
    }
    if (usb_is_configured() && !usb_suspended) {
        v |= LED_VIRTUAL_USB_ACTIVE_BIT;
    }
    if (!usb_is_ok()) {
        v |= LED_VIRTUAL_USB_ERROR_BIT;
    }
    usb_virtual_leds = v;
#endif
}

static void
usb_devices_reset (void) {
    usb_keyboard_reset();
}

void
usb_init (void) {
    tinyusb_hardware_init();

    tusb_rhport_init_t rh_init = {
        .role = TUSB_ROLE_DEVICE,
        .speed = TUSB_SPEED_FULL,
    };
    tusb_init(TINYUSB_CFG_RHPORT, &rh_init);

    usb_descriptors_init();
    usb_devices_reset();

    local_configuration = 0;
    usb_error = 0;
    usb_status = 0;
    usb_suspended = false;

#if ENABLE_DFU_INTERFACE
    usb_request_detach = 0;
#endif
}

void
usb_bus_attach (void) {
    // Deferred — tud_connect() happens on first usb_tick() call.
}

void
usb_bus_detach (void) {
    if (tusb_inited()) {
        tud_disconnect();
    }
}

#if ENABLE_GENERIC_HID_ENDPOINT
static uint8_t generic_vial_response[GENERIC_HID_REPORT_SIZE];
static uint8_t generic_vial_report_id;
static bool generic_vial_pending;
#endif

void
usb_tick (void) {
    tud_task();
#if ENABLE_GENERIC_HID_ENDPOINT
    if (generic_vial_pending) {
        if (tud_hid_n_report(1, generic_vial_report_id, generic_vial_response, GENERIC_HID_REPORT_SIZE)) {
            generic_vial_pending = false;
        }
    }
    make_and_send_generic_hid_report();
#endif

    // First call: connect to USB bus now that all init is complete.
    static bool connected = false;
    if (!connected) {
        connected = true;
        tud_connect();
    }

    update_virtual_leds();

    if (local_configuration == 0 && (usb_status & USB_STATUS_JUMP_TO_BOOTLOADER)) {
        jump_to_bootloader();
    }

    // HID idle rate: periodic report at idle_rate × 4 ms intervals (HID spec)
#if ENABLE_KEYBOARD_ENDPOINT
    if (keyboard_idle_rate && tud_ready()) {
        uint32_t now = timer_read32();
        if ((uint32_t)(now - keyboard_last_report) >= (uint32_t)keyboard_idle_rate * HID_IDLE_RATE_MS) {
            keyboard_last_report = now;
            usb_keyboard_send_report();
        }
    }
#endif
#if ENABLE_GENERIC_HID_ENDPOINT
    if (generic_idle_rate && tud_ready()) {
        uint32_t now = timer_read32();
        if ((uint32_t)(now - generic_last_report) >= (uint32_t)generic_idle_rate * HID_IDLE_RATE_MS) {
            generic_last_report = now;
            make_and_send_generic_hid_report();
        }
    }
#endif

#if ENABLE_GENERIC_HID_ENDPOINT
    if (tud_ready()) {
        make_and_send_generic_hid_report();
    }
#endif

#if ENABLE_DFU_INTERFACE
    if (usb_request_detach && local_configuration) {
        if (usb_request_detach < 0xFF) {
            --usb_request_detach;
        }
        if (usb_request_detach == 0) {
            local_configuration = 0;
            usb_status |= USB_STATUS_JUMP_TO_BOOTLOADER;
        }
    }
#endif
#if ENABLE_HOST_FINGERPRINT
    host_fingerprint_notify_if_needed();
#endif
}

void
usb_deinit (void) {
    usb_keyboard_leds = LED_MASK_ALL;
    usb_keyboard_release_all_keys();
    (void) usb_keyboard_send_report();

    tusb_deinit(TINYUSB_CFG_RHPORT);

    local_configuration = 0;
    usb_suspended = false;
}

bool
usb_is_ok (void) {
    return (local_configuration != 0) && (usb_error == 0) && tud_ready();
}

uint8_t
usb_is_configured (void) {
    return local_configuration;
}

uint8_t
usb_last_error (void) {
    return usb_error;
}

bool
usb_is_suspended (void) {
    return usb_suspended;
}

uint8_t
usb_detach_requested (void) {
#if ENABLE_DFU_INTERFACE
    return usb_request_detach;
#else
    return 0;
#endif
}

bool
usb_wake_up_host (void) {
    if (!usb_suspended || !(usb_status & USB_STATUS_REMOTE_WAKEUP_ENABLED)) {
        usb_error = 'w';
        return false;
    }

    tud_remote_wakeup();
    uint32_t wakeup_start = timer_read32();
    while ((uint32_t)(timer_read32() - wakeup_start) < WAKEUP_TIMEOUT_MS) {
        tud_task();
        if (!tud_suspended()) {
            usb_suspended = false;
            return true;
        }
    }

    // Clear low-power mode to allow endpoint transmission after wakeup:
    USB->CNTR &= ~(USB_CNTR_FSUSP | USB_CNTR_LPMODE);
    usb_suspended = false;
    return true;
}

uint8_t
usb_address (void) {
    return 0;
}

void __attribute__((weak))
usb_wake_up_interrupt (void) { }

void __attribute__((weak))
usb_suspend_interrupt (void) { }

// MARK: - Keyboard Report

bool
usb_keyboard_send_report (void) {
    if (!tud_ready()) {
        if (!tud_suspended()) {
            usb_error = 'c';
            return false;
        }
        tud_remote_wakeup();
        uint32_t wakeup_start = timer_read32();
        while ((uint32_t)(timer_read32() - wakeup_start) < WAKEUP_TIMEOUT_MS) {
            tud_task();
            if (!tud_suspended()) {
                goto resume_done;
            }
        }
        // Clear low-power mode to allow endpoint transmission after wakeup:
        USB->CNTR &= ~(USB_CNTR_FSUSP | USB_CNTR_LPMODE);
    }
resume_done:

    uint8_t report[CFG_TUD_HID_BUFSIZE];
    uint8_t pos = 0;

    // usb_tx_report_header() — modifier + optional bytes
    report[pos++] = usb_keys_modifier_flags;
#if RESERVE_BOOT_PROTOCOL_RESERVED_BYTE
    report[pos++] = 0;
#endif
#if VIRTUAL_KEY_BYTES_IN_REPORT
    if (!(RESERVE_BOOT_PROTOCOL_RESERVED_BYTE && usb_keyboard_is_in_boot_protocol)) {
        report[pos++] = usb_keys_extended_flags & 0xFFU;
#if VIRTUAL_KEY_BYTES_IN_REPORT > 1
        report[pos++] = (usb_keys_extended_flags >> 8) & 0xFFU;
#endif
    }
#endif

    // usb_tx_keys_state() — build key array
    int_fast8_t count = usb_keyboard_rollover;
    usb_keyboard_updated = false;

#if KEY_IN_RESERVED_BYTE
    report[pos++] = usb_keys_buffer[--count];
#endif
    for (int_fast8_t i = 0; i < count; ++i) {
        report[pos++] = usb_keys_buffer[i];
    }

    uint8_t err = key_error;
    if (err & KEY_ERROR_NEEDS_REPORTING_FLAG) {
        key_error = err + 1;
        err &= ~KEY_ERROR_NEEDS_REPORTING_FLAG;
        if (err) {
            for (int_fast8_t i = KEY_IN_RESERVED_BYTE + 1; i < pos; ++i) {
                report[i] = err;
            }
        }
    }

    usb_error = 0;
    {
        uint32_t send_start = timer_read32();
        bool sent = false;
        while (!sent && (uint32_t)(timer_read32() - send_start) < SEND_TIMEOUT_MS) {
            sent = tud_hid_n_report(0, 0, report, pos);
            if (!sent) {
                tud_task();
            }
        }
        if (sent) {
            return true;
        }
        usb_error = 'T';
    }
    return true;
}

// MARK: - Consumer / Media Keys Endpoint

#if MEDIA_KEYS_ENDPOINT
bool
usb_keyboard_send_consumer (uint16_t usage) {
    if (!tud_ready()) {
        return false;
    }

    uint8_t report[2] = { usage & 0xFF, (usage >> 8) & 0xFF };

    uint32_t send_start = timer_read32();
    bool sent = false;
    while (!sent && (uint32_t)(timer_read32() - send_start) < SEND_TIMEOUT_MS) {
        sent = tud_hid_n_report(CONSUMER_HID_INSTANCE, CONSUMER_REPORT_ID, report, sizeof report);
        if (!sent) {
            tud_task();
        }
    }
    return sent;
}
#endif

// MARK: - Generic HID

#if ENABLE_GENERIC_HID_ENDPOINT

static uint8_t generic_vial_response[GENERIC_HID_REPORT_SIZE];
static uint8_t generic_vial_report_id;
static bool generic_vial_pending;

bool
send_generic_hid_report (uint8_t report_id, uint8_t count,
                         const uint8_t report[static count]) {
    if (!tud_ready()) {
        return false;
    }
    return tud_hid_n_report(1, report_id, report, count);
}

bool
make_and_send_generic_hid_report (void) {
#if GENERIC_HID_REPORT_SIZE
    uint8_t report[GENERIC_HID_REPORT_SIZE];
    if (make_generic_hid_report(0, GENERIC_HID_REPORT_SIZE, report)) {
        return send_generic_hid_report(0, GENERIC_HID_REPORT_SIZE, report);
    }
#endif
    return false;
}

#endif

// MARK: - TinyUSB Device Descriptor Callbacks

uint8_t const *
tud_descriptor_device_cb (void) {
    return find_descriptor(
        BYTES_WORD(0, DESCRIPTOR_TYPE_DEVICE), 0);
}

uint8_t const *
tud_descriptor_configuration_cb (uint8_t index) {
    return find_descriptor(
        BYTES_WORD(index, DESCRIPTOR_TYPE_CONFIGURATION), 0);
}

uint16_t const *
tud_descriptor_string_cb (uint8_t index, uint16_t langid) {
#if USB_STRINGS_STORED_AS_ASCII
    #error "USB_STRINGS_STORED_AS_ASCII is not supported with TinyUSB — set USB_STRINGS_STORED_AS_ASCII=0"
#endif
    return (uint16_t const *) find_descriptor(
        BYTES_WORD(index, DESCRIPTOR_TYPE_STRING), langid);
}

// MARK: - TinyUSB HID Callbacks

uint8_t const *
tud_hid_descriptor_report_cb (uint8_t instance) {
    uint16_t index;
    if (instance == 0) {
        index = KEYBOARD_INTERFACE_INDEX;
    }
#if ENABLE_GENERIC_HID_ENDPOINT
    else if (instance == 1) {
        index = GENERIC_INTERFACE_INDEX;
    }
#endif
#if MEDIA_KEYS_ENDPOINT
    else if (instance == CONSUMER_HID_INSTANCE) {
        index = CONSUMER_INTERFACE_INDEX;
    }
#endif
    else {
        return NULL;
    }
    return find_descriptor(
        BYTES_WORD(0, HID_DESCRIPTOR_TYPE_REPORT), index);
}

uint16_t
tud_hid_get_report_cb (uint8_t instance, uint8_t report_id,
                       hid_report_type_t report_type,
                       uint8_t *buffer, uint16_t reqlen) {
    (void) report_id;
    (void) report_type;

    if (instance != 0) {
#if ENABLE_GENERIC_HID_ENDPOINT
        if (instance == 1) {
            if (reqlen > GENERIC_HID_REPORT_SIZE) {
                reqlen = GENERIC_HID_REPORT_SIZE;
            }
            make_generic_hid_report(report_id, reqlen, buffer);
            return reqlen;
        }
#endif
#if MEDIA_KEYS_ENDPOINT
        if (instance == CONSUMER_HID_INSTANCE) {
            // Consumer endpoint is event-driven; no state to report on GET.
            return 0;
        }
#endif
        return 0;
    }

    uint8_t pos = 0;
    buffer[pos++] = usb_keys_modifier_flags;
#if RESERVE_BOOT_PROTOCOL_RESERVED_BYTE
    buffer[pos++] = 0;
#endif
#if VIRTUAL_KEY_BYTES_IN_REPORT
    if (!(RESERVE_BOOT_PROTOCOL_RESERVED_BYTE && usb_keyboard_is_in_boot_protocol)) {
        buffer[pos++] = usb_keys_extended_flags & 0xFFU;
#if VIRTUAL_KEY_BYTES_IN_REPORT > 1
        buffer[pos++] = (usb_keys_extended_flags >> 8) & 0xFFU;
#endif
    }
#endif

    int_fast8_t count = usb_keyboard_rollover;
#if KEY_IN_RESERVED_BYTE
    buffer[pos++] = usb_keys_buffer[--count];
#endif
    for (int_fast8_t i = 0; i < count; ++i) {
        buffer[pos++] = usb_keys_buffer[i];
    }

    return (pos <= reqlen) ? pos : reqlen;
}

void
tud_hid_set_report_cb (uint8_t instance, uint8_t report_id,
                       hid_report_type_t report_type,
                       uint8_t const *buffer, uint16_t bufsize) {
    (void) report_id;

    if (instance == 0 &&
        report_type == HID_REPORT_TYPE_OUTPUT && bufsize >= 1) {
        usb_keyboard_leds = buffer[0];
    }

#if ENABLE_GENERIC_HID_ENDPOINT
    if (instance == 1 && bufsize > 0 &&
        (report_type == HID_REPORT_TYPE_OUTPUT || report_type == HID_REPORT_TYPE_FEATURE)) {
        uint8_t response_length = sizeof(generic_vial_response);
        uint8_t result = handle_generic_hid_report(
            report_id, bufsize, (uint8_t *) buffer,
            &response_length, generic_vial_response);
        generic_vial_report_id = report_id;
        generic_vial_pending = (result == RESPONSE_SEND_REPLY && response_length);
        if (result == RESPONSE_JUMP_TO_BOOTLOADER) {
            jump_to_bootloader();
        }
    }
#endif

#if ENABLE_DFU_INTERFACE
    if (instance == 0 &&
        report_type == HID_REPORT_TYPE_FEATURE && bufsize >= 1 &&
        buffer[0] == DFU_DETACH_MAGIC) {
        usb_request_detach = DFU_DETACH_DELAY_MS;
    }
#endif
}

void
tud_hid_set_protocol_cb (uint8_t instance, uint8_t protocol) {
    if (instance == 0) {
        usb_keyboard_protocol = protocol;
    }
}

bool
tud_hid_set_idle_cb (uint8_t instance, uint8_t idle_rate) {
#if ENABLE_KEYBOARD_ENDPOINT
    if (instance == 0) {
        keyboard_idle_rate = idle_rate;
        keyboard_last_report = timer_read32();
    }
#endif
#if ENABLE_GENERIC_HID_ENDPOINT
    if (instance == 1) {
        generic_idle_rate = idle_rate;
        generic_last_report = timer_read32();
    }
#endif
    return true;
}

void
tud_hid_report_complete_cb (uint8_t instance, uint8_t const *report,
                            uint16_t len) {
    (void) instance;
    (void) report;
    (void) len;
}

void
tud_hid_report_failed_cb (uint8_t instance, hid_report_type_t report_type,
                          uint8_t const *report, uint16_t xferred_bytes) {
    (void) instance;
    (void) report_type;
    (void) report;
    (void) xferred_bytes;
}

// MARK: - TinyUSB Device Event Callbacks

void
tud_mount_cb (void) {
    local_configuration = 1;
    usb_suspended = false;
}

void
tud_umount_cb (void) {
    local_configuration = 0;
    usb_keyboard_reset();
}

void
tud_suspend_cb (bool remote_wakeup_en) {
    usb_suspended = true;
    if (remote_wakeup_en) {
        usb_status |= USB_STATUS_REMOTE_WAKEUP_ENABLED;
    } else {
        usb_status &= ~USB_STATUS_REMOTE_WAKEUP_ENABLED;
    }
    usb_suspend_interrupt();
}

void
tud_resume_cb (void) {
    usb_suspended = false;
    usb_wake_up_interrupt();
}

// MARK: - TinyUSB DFU Runtime Callback

#if CFG_TUD_DFU_RUNTIME
void tud_dfu_runtime_reboot_to_dfu_cb (void) {
    jump_to_bootloader();
}
#endif
