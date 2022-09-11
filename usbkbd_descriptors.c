/*
 * usbkbd_descriptors.c: USB HID keyboard descriptors.
 *
 * Copyright (c) 2021-2022 Kimmo Kulovesi, https://arkku.dev/
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

#include "aakbd.h"
#include "usbkbd_config.h"
#include "usb.h"
#include "usbkbd_descriptors.h"
#include "usb_keys.h"
#include "generic_hid.h"

// MARK: - USB Descriptors

/// Device descriptor.
static const uint8_t PROGMEM device_descriptor[] = {
    DESCRIPTOR_SIZE_DEVICE,             // bLength
    DESCRIPTOR_TYPE_DEVICE,             // bType
    WORD_BYTES(USB_VERSION),            // bcdUSB
    DEVICE_NO_SPECIFIC_CLASS,           // bDeviceClass
    DEVICE_NO_SPECIFIC_SUBCLASS,        // bDeviceSubClass
    DEVICE_NO_SPECIFIC_PROTOCOL,        // bDeviceProtocol
    ENDPOINT_0_SIZE,                    // bMaxPacketSize0
    WORD_BYTES(USB_VENDOR_ID),          // idVendor
    WORD_BYTES(USB_PRODUCT_ID),         // idProduct
    WORD_BYTES(DEVICE_VERSION),         // bcdDevice
    STRING_INDEX_MANUFACTURER,          // iManufacturer
    STRING_INDEX_PRODUCT,               // iProduct
    STRING_INDEX_SERIAL_NUMBER,         // iSerialNumber
    CONFIGURATIONS_COUNT                // bNumConfigurations
};

#if HARDWARE_SUPPORTS_HIGH_SPEED
static const uint8_t PROGMEM device_qualifier_descriptor[DESCRIPTOR_SIZE_DEVICE_QUALIFIER] = {
    DESCRIPTOR_SIZE_DEVICE_QUALIFIER,   // bLength
    DESCRIPTOR_TYPE_DEVICE_QUALIFIER,   // bType
    WORD_BYTES(USB_VERSION),            // bcdUSB
    DEVICE_NO_SPECIFIC_CLASS,           // bDeviceClass
    DEVICE_NO_SPECIFIC_SUBCLASS,        // bDeviceSubClass
    DEVICE_NO_SPECIFIC_PROTOCOL,        // bDeviceProtocol
    ENDPOINT_0_SIZE,                    // bMaxPacketSize0
    CONFIGURATIONS_COUNT,               // bNumConfigurations
    0                                   // bReserved
};
#endif

#define HID_KEYBOARD_REPORT_KEY_ARRAY(rollover)         \

/// HID descriptor for the keyboard (boot protocol compatible).
static const uint8_t PROGMEM kbd_boot_hid_descriptor[] = {
    HID_USAGE_PAGE,         HID_USAGE_PAGE_GENERIC_DESKTOP,
    HID_USAGE,              HID_USAGE_KEYBOARD,
    HID_COLLECTION,         HID_COLLECTION_APPLICATION,

#if USE_MULTIPLE_REPORTS
    HID_REPORT_ID,          KEYBOARD_REPORT_ID,
#endif

    // Modifier keys
    HID_USAGE_PAGE,         HID_USAGE_PAGE_KEYCODES,
    HID_USAGE_MINIMUM,      MODIFIERS_START,
    HID_USAGE_MAXIMUM,      MODIFIERS_END - APPLE_FN_IS_MODIFIER,
    HID_LOGICAL_MINIMUM,    0,
    HID_LOGICAL_MAXIMUM,    1,
    HID_REPORT_SIZE,        1,
    HID_REPORT_COUNT,       MODIFIER_COUNT - APPLE_FN_IS_MODIFIER,
    HID_INPUT,              HID_IO_VARIABLE,

#if RESERVE_BOOT_PROTOCOL_RESERVED_BYTE
    // Reserved byte of the boot protocol
    // Note: In boot protocol mode this descriptor will be ignored, so our
    // actual report does not have to conform to the boot protocol, nor do we
    // need to provide an alternative endpoint that does. The HID 1.11 spec
    // seems quite clear on that (Appendix B):
    //
    // …since the BIOS does not actually read the Report descriptors, these
    // descriptors do not have to be hard-coded into the device if an
    // alternative report descriptor is provided. Instead, descriptors that
    // describe the device reports in a USB-aware operating system should be
    // included (these may or may not be the same).
    HID_REPORT_COUNT,       1,
    HID_REPORT_SIZE,        16 - MODIFIER_COUNT,
    HID_INPUT,              HID_IO_CONSTANT,
#endif

#if ENABLE_APPLE_FN_KEY
    HID_LOGICAL_MINIMUM,    0,
    HID_LOGICAL_MAXIMUM,    1,
#if ENABLE_EXTRA_APPLE_KEYS || APPLE_FN_IS_MODIFIER
    HID_REPORT_SIZE,        1,
#else
    HID_REPORT_SIZE,        8,
#endif
    HID_REPORT_COUNT,       1,
    HID_USAGE_PAGE,         HID_USAGE_PAGE_VENDOR_APPLE_TOP_COVER,
    HID_USAGE,              HID_USAGE_APPLE_FN_KEY,
    HID_INPUT,              HID_IO_VARIABLE,
#if ENABLE_EXTRA_APPLE_KEYS
#if APPLE_FN_IS_MODIFIER
    HID_REPORT_COUNT,       1,
    HID_REPORT_SIZE,        1,
    HID_INPUT,              HID_IO_CONSTANT,
#endif
    HID_LOGICAL_MINIMUM,    0,
    HID_LOGICAL_MAXIMUM,    1,
    HID_REPORT_SIZE,        1,
    HID_REPORT_COUNT,       1,
    HID_USAGE_PAGE_WORD,    WORD_BYTES(HID_USAGE_PAGE_VENDOR_APPLE_KEYBOARD),
    HID_USAGE,              HID_USAGE_APPLE_BRIGHTNESS_UP,
    HID_INPUT,              HID_IO_VARIABLE,
    HID_USAGE,              HID_USAGE_APPLE_BRIGHTNESS_DOWN,
    HID_INPUT,              HID_IO_VARIABLE,
    HID_USAGE,              HID_USAGE_APPLE_SPOTLIGHT,
    HID_INPUT,              HID_IO_VARIABLE,
    HID_USAGE,              HID_USAGE_APPLE_DASHBOARD,
    HID_INPUT,              HID_IO_VARIABLE,
    HID_USAGE,              HID_USAGE_APPLE_LAUNCHPAD,
    HID_INPUT,              HID_IO_VARIABLE,
    HID_USAGE,              HID_USAGE_APPLE_EXPOSE,
    HID_INPUT,              HID_IO_VARIABLE,
    HID_USAGE,              HID_USAGE_APPLE_EXPOSE_DESKTOP,
    HID_INPUT,              HID_IO_VARIABLE,
#endif // ENABLE_EXTRA_APPLE_KEYS
#endif // ^ ENABLE_APPLE_FN_KEY

    // LEDs
    HID_REPORT_COUNT,       LED_COUNT,
    HID_REPORT_SIZE,        1,
    HID_USAGE_PAGE,         HID_USAGE_PAGE_LEDS,
    HID_USAGE_MINIMUM,      1,
    HID_USAGE_MAXIMUM,      LED_COUNT,
    HID_OUTPUT,             HID_IO_VARIABLE,

    // Pad LEDs to 8 bits
#if LED_COUNT < 8
    HID_REPORT_COUNT,       1,
    HID_REPORT_SIZE,        8 - LED_COUNT,
    HID_OUTPUT,             HID_IO_CONSTANT,
#endif

    // Keys
    HID_REPORT_COUNT,       USB_MAX_KEY_ROLLOVER,
    HID_REPORT_SIZE,        8,
    HID_LOGICAL_MINIMUM,    0x00,
    HID_LOGICAL_MAXIMUM,    0xFF,
    HID_USAGE_PAGE,         HID_USAGE_PAGE_KEYCODES,
    HID_USAGE_MINIMUM,      0x00,
    HID_USAGE_MAXIMUM,      0xFF,
    HID_INPUT,              HID_IO_ARRAY,

    HID_END_COLLECTION
};

#if ENABLE_GENERIC_HID_ENDPOINT
static const uint8_t PROGMEM generic_hid_descriptor[] = {
    HID_USAGE_PAGE_WORD,    WORD_BYTES(GENERIC_HID_USAGE_PAGE),
    HID_USAGE_WORD,         WORD_BYTES(GENERIC_HID_USAGE),
    HID_COLLECTION,         HID_COLLECTION_APPLICATION,

#if GENERIC_HID_REPORT_SIZE != 0
    HID_USAGE,              GENERIC_HID_INPUT_USAGE,
    HID_REPORT_SIZE,        8,
    HID_LOGICAL_MINIMUM,    0x00,
    HID_LOGICAL_MAXIMUM,    0xFF,
    HID_REPORT_COUNT,       GENERIC_HID_REPORT_SIZE,
    HID_INPUT,              HID_IO_VARIABLE,
#endif

    HID_USAGE,              GENERIC_HID_OUTPUT_USAGE,
    HID_REPORT_SIZE,        8,
    HID_LOGICAL_MINIMUM,    0x00,
    HID_LOGICAL_MAXIMUM,    0xFF,
    HID_REPORT_COUNT,       GENERIC_HID_FEATURE_SIZE,
    HID_FEATURE,            HID_IO_VARIABLE,

    HID_END_COLLECTION
};
#endif

#define FUNCTIONAL_CONFIGURATION_COUNT      INTERFACES_COUNT

#define CONFIGURATION_SIZE_I_F_E(if_count, func_count, ep_count)     (DESCRIPTOR_SIZE_CONFIGURATION + (DESCRIPTOR_SIZE_INTERFACE * (if_count)) + (DESCRIPTOR_SIZE_FUNCTIONAL * (func_count)) + (DESCRIPTOR_SIZE_ENDPOINT * (ep_count)))

#define CONFIGURATION_SIZE  CONFIGURATION_SIZE_I_F_E(INTERFACES_COUNT, FUNCTIONAL_CONFIGURATION_COUNT, ENDPOINT_COUNT)

/// The offset of the HID configuration at `index`, assuming all prior indices
/// are also HID interfaces with one endpoint and configuration each.
#define HID_CONFIGURATION_OFFSET(index)     CONFIGURATION_SIZE_I_F_E(((index) + 1), (index), (index))

#define KEYBOARD_HID_CONFIGURATION_OFFSET   HID_CONFIGURATION_OFFSET(KEYBOARD_INTERFACE_INDEX)
#define GENERIC_HID_CONFIGURATION_OFFSET    HID_CONFIGURATION_OFFSET(GENERIC_INTERFACE_INDEX)
#define DFU_CONFIGURATION_OFFSET            CONFIGURATION_SIZE_I_F_E(INTERFACES_COUNT, FUNCTIONAL_CONFIGURATION_COUNT - 1, ENDPOINT_COUNT)

#if IS_SUSPEND_SUPPORTED
#define CONFIGURATION_ATTRIBUTES (CONFIGURATION_ATTRIBUTES_RESERVED | CONFIGURATION_ATTRIBUTES_REMOTE_WAKE_UP_FLAG)
#else
#define CONFIGURATION_ATTRIBUTES (CONFIGURATION_ATTRIBUTES_RESERVED)
#endif

#ifndef COUNTRY_CODE
#if defined(ANSI_LAYOUT) && ANSI_LAYOUT
#define COUNTRY_CODE    COUNTRY_CODE_US
#elif defined(ISO_LAYOUT) && ISO_LAYOUT
#define COUNTRY_CODE    COUNTRY_CODE_ISO
#else
#define COUNTRY_CODE    COUNTRY_CODE_NONE
#endif
#endif

/// The configuration descriptor. Note that this is a collection of multiple
/// descriptors, so the length of the entire thing is not the first byte of
/// this descriptor.
static const uint8_t PROGMEM configuration_descriptor[] = {
    // Configuration
    DESCRIPTOR_SIZE_CONFIGURATION,          // bLength
    DESCRIPTOR_TYPE_CONFIGURATION,          // bType
    WORD_BYTES(CONFIGURATION_SIZE),         // wTotalLength
    INTERFACES_COUNT,                       // bNumInterfaces
    KEYBOARD_CONFIGURATION,                 // bConfigurationValue
    INTERFACE_NO_DESCRIPTOR,                // iConfiguration
    CONFIGURATION_ATTRIBUTES,               // bmAttributes
    DIV_ROUND_BYTE(2, MAX_POWER_CONSUMPTION_MA), // bMaxPower (2mA units)

#if ENABLE_KEYBOARD_ENDPOINT
    // Interface
    DESCRIPTOR_SIZE_INTERFACE,              // bLength
    DESCRIPTOR_TYPE_INTERFACE,              // bDescriptorType
    KEYBOARD_INTERFACE_INDEX,               // bInterfaceNumber
    0,                                      // bAlternateSetting
    1,                                      // bNumEndpoints
    INTERFACE_CLASS_HID,                    // bInterfaceClass
    INTERFACE_SUBCLASS_BOOT,                // bInterfaceSubClass
    INTERFACE_PROTOCOL_BOOT_KEYBOARD,       // bInterfaceProtocol
    INTERFACE_NO_DESCRIPTOR,                // iInterface

    // HID
    DESCRIPTOR_SIZE_HID,                    // bLength
    DESCRIPTOR_TYPE_HID,                    // bDescriptorType
    WORD_BYTES(0x0111),                     // bcdHID 1.11
    COUNTRY_CODE,                           // bCountryCode
    1,                                      // bNumDescriptors
    HID_DESCRIPTOR_TYPE_REPORT,             // bDescriptorType
    WORD_BYTES(sizeof kbd_boot_hid_descriptor), // wDescriptorLength
#endif

#if (ENABLE_KEYBOARD_ENDPOINT || ENABLE_DFU_INTERFACE)
    // Endpoint
    DESCRIPTOR_SIZE_ENDPOINT,               // bLength
    DESCRIPTOR_TYPE_ENDPOINT,               // bDescriptorType
    KEYBOARD_ENDPOINT_ADDRESS,              // bEndpointAddress
    ENDPOINT_ATTRIBUTES_INTERRUPT,          // bmAttributes
    WORD_BYTES(KEYBOARD_ENDPOINT_SIZE),     // wMaxPacketSize
    KEYBOARD_POLL_INTERVAL_MS,              // bInterval
#endif

#if ENABLE_GENERIC_HID_ENDPOINT
    // Interface
    DESCRIPTOR_SIZE_INTERFACE,              // bLength
    DESCRIPTOR_TYPE_INTERFACE,              // bDescriptorType
    GENERIC_INTERFACE_INDEX,                // bInterfaceNumber
    0,                                      // bAlternateSetting
    1 + ENABLE_GENERIC_HID_OUTPUT,          // bNumEndpoints
    INTERFACE_CLASS_HID,                    // bInterfaceClass
    INTERFACE_NO_SPECIFIC_SUBCLASS,         // bInterfaceSubClass
    INTERFACE_NO_SPECIFIC_PROTOCOL,         // bInterfaceProtocol
    INTERFACE_NO_DESCRIPTOR,                // iInterface

    // HID
    DESCRIPTOR_SIZE_HID,                    // bLength
    DESCRIPTOR_TYPE_HID,                    // bDescriptorType
    WORD_BYTES(0x0111),                     // bcdHID 1.11
    COUNTRY_CODE_NONE,                      // bCountryCode
    1,                                      // bNumDescriptors
    HID_DESCRIPTOR_TYPE_REPORT,             // bDescriptorType
    WORD_BYTES(sizeof generic_hid_descriptor), // wDescriptorLength

    // Endpoint
    DESCRIPTOR_SIZE_ENDPOINT,               // bLength
    DESCRIPTOR_TYPE_ENDPOINT,               // bDescriptorType
    GENERIC_ENDPOINT_ADDRESS_IN,            // bEndpointAddress
    ENDPOINT_ATTRIBUTES_INTERRUPT,          // bmAttributes
    WORD_BYTES(GENERIC_ENDPOINT_SIZE),      // iwMaxPacketSize
    GENERIC_HID_POLL_INTERVAL_MS,           // bInterval

#if ENABLE_GENERIC_HID_OUTPUT
    DESCRIPTOR_SIZE_ENDPOINT,               // bLength
    DESCRIPTOR_TYPE_ENDPOINT,               // bDescriptorType
    GENERIC_ENDPOINT_ADDRESS_OUT,           // bEndpointAddress
    ENDPOINT_ATTRIBUTES_INTERRUPT,          // bmAttributes
    WORD_BYTES(GENERIC_ENDPOINT_SIZE),      // iwMaxPacketSize
    GENERIC_HID_POLL_INTERVAL_MS,           // bInterval
#endif
#endif

#if ENABLE_DFU_INTERFACE
    // Interface
    DESCRIPTOR_SIZE_INTERFACE,              // bLength
    DESCRIPTOR_TYPE_INTERFACE,              // bDescriptorType
    DFU_INTERFACE_INDEX,                    // bInterfaceNumber
    0,                                      // bAlternateSetting
    0,                                      // bNumEndpoints
    INTERFACE_CLASS_APPLICATION_SPECIFIC,   // bInterfaceClass
    INTERFACE_SUBCLASS_APPLICATION_DFU,     // bInterfaceSubClass
    INTERFACE_PROTOCOL_DFU_RUNTIME,         // bInterfaceProtocol
    INTERFACE_NO_DESCRIPTOR,                // iInterface

    // Functional
    DESCRIPTOR_SIZE_FUNCTIONAL,             // bLength
    DESCRIPTOR_TYPE_FUNCTIONAL,             // bDescriptorType
    (DFU_ATTRIBUTE_CAN_UPLOAD | DFU_ATTRIBUTE_CAN_DOWNLOAD), // bmAttributes
    WORD_BYTES(255),                        // wDetachTimeOut (ms)
    WORD_BYTES(0x0C00),                     // wTransferSize (as per Atmel DFU)
    WORD_BYTES(0x0101)                      // bcdDFUVersion 1.1
#endif
};

#define SUPPORTED_LANGUAGES_SIZE (2 + (SUPPORTED_LANGUAGE_COUNT * 2))

/// The list of supported language ids.
/// To support more languages, one would need to add the language ids to this
/// list, and then add corresponding strings to the descriptor list such that
/// the `wIndex` is the language id.
static const uint16_t PROGMEM supported_languages[] = {
    BYTES_WORD(2 + (SUPPORTED_LANGUAGE_COUNT * 2), DESCRIPTOR_TYPE_STRING),
    LANGUAGE_ID
};


#if USB_STRINGS_STORED_AS_ASCII
// Note that string descriptors (other than `supported_languages` are stored as
// ASCII strings, and the UTF-16 strings are formed programmatically from them.
// This may cause trouble if you want non-ASCII characters in the strings, but
// it is probably better to keep these USB device names ASCII.

static const char PROGMEM manufacturer_string[] = MANUFACTURER_STRING;
static const char PROGMEM product_string[] = PRODUCT_STRING;
static const char PROGMEM serial_string[] = SERIAL_NUMBER_STRING;

/// Helper to list a descriptor from an ASCII string.
#define DESC_STR(num, lang, str, ascii) {BYTES_WORD((num), DESCRIPTOR_TYPE_STRING), (lang), (str), sizeof(ascii) * 2}
#else
#include <stddef.h>

#define STR_TO_UNICODE(ascii) L ## ascii

#ifndef PACKED
#define PACKED __attribute__ ((packed))
#endif

#if WCHAR_MAX == 0x7FFF
typedef int16_t unicode_char_t;
#else
typedef uint16_t unicode_char_t;
#endif

// Flexible array member initialisation is a GCC extension
#pragma GCC diagnostic ignored "-Wpedantic"

struct usb_string_descriptor {
    uint8_t size;
    uint8_t type;
    unicode_char_t unicode_string[];
} PACKED;

#define USB_STRING_DESCRIPTOR(ascii) { .size = sizeof(ascii) * 2, .type = DESCRIPTOR_TYPE_STRING, .unicode_string = STR_TO_UNICODE(ascii) }

static const struct usb_string_descriptor PROGMEM manufacturer_string = USB_STRING_DESCRIPTOR(MANUFACTURER_STRING);
static const struct usb_string_descriptor PROGMEM product_string = USB_STRING_DESCRIPTOR(PRODUCT_STRING);
static const struct usb_string_descriptor PROGMEM serial_string = USB_STRING_DESCRIPTOR(SERIAL_NUMBER_STRING);

#define DESC_STR(num, lang, desc, ascii) {BYTES_WORD((num), DESCRIPTOR_TYPE_STRING), (lang), (const char *) &(desc), sizeof(ascii) * 2 }
#endif

/// Helper to list a descriptor that is the entire contents of the array
/// or struct provided.
#define DESC_FULL(type, num, index, data) {BYTES_WORD((num), (type)), (index), (const char *) (data), sizeof((data))}

/// Helper to list a descriptor from a pointer to partial contents of an
/// array or struct. The size must be provided explicitly.
#define DESC_PART(type, num, index, data, size) {BYTES_WORD((num), (type)), (index), (const char *) (data), (size)}

/// The list of descriptors. This list is available via USB request.
const struct usb_descriptor PROGMEM descriptor_list[] = {
    DESC_FULL(
        DESCRIPTOR_TYPE_DEVICE, 0,
        0,
        device_descriptor
    ),
    DESC_FULL(
        DESCRIPTOR_TYPE_CONFIGURATION, 0,
        0,
        configuration_descriptor
    ),
    DESC_FULL(
        HID_DESCRIPTOR_TYPE_REPORT, 0,
        KEYBOARD_INTERFACE_INDEX,
        kbd_boot_hid_descriptor
    ),
    DESC_PART(
        DESCRIPTOR_TYPE_HID, 0,
        KEYBOARD_INTERFACE_INDEX,
        configuration_descriptor + KEYBOARD_HID_CONFIGURATION_OFFSET,
        DESCRIPTOR_SIZE_HID
    ),
#if ENABLE_GENERIC_HID_ENDPOINT
    DESC_FULL(
        HID_DESCRIPTOR_TYPE_REPORT, 0,
        GENERIC_INTERFACE_INDEX,
        generic_hid_descriptor
    ),
    DESC_PART(
        DESCRIPTOR_TYPE_HID, 0,
        GENERIC_INTERFACE_INDEX,
        configuration_descriptor + GENERIC_HID_CONFIGURATION_OFFSET,
        DESCRIPTOR_SIZE_HID
    ),
#endif
#if ENABLE_DFU_INTERFACE
    DESC_PART(
        DESCRIPTOR_TYPE_FUNCTIONAL, 0, // TBH not sure should it be 0 or index
        DFU_INTERFACE_INDEX,
        configuration_descriptor + DFU_CONFIGURATION_OFFSET,
        DESCRIPTOR_SIZE_FUNCTIONAL
    ),
#endif
    DESC_FULL(DESCRIPTOR_TYPE_STRING, 0, 0, supported_languages),
    DESC_STR(STRING_INDEX_MANUFACTURER,     LANGUAGE_ID, manufacturer_string, MANUFACTURER_STRING),
    DESC_STR(STRING_INDEX_PRODUCT,          LANGUAGE_ID, product_string, PRODUCT_STRING),
    DESC_STR(STRING_INDEX_SERIAL_NUMBER,    LANGUAGE_ID, serial_string, SERIAL_NUMBER_STRING),
#if HARDWARE_SUPPORTS_HIGH_SPEED
    DESC_FULL(
        DESCRIPTOR_TYPE_DEVICE_QUALIFIER, 0,
        0,
        device_qualifier_descriptor
    )
#endif
};

void
usb_descriptors_init (void) {
    _Static_assert(DESCRIPTOR_SIZE_DEVICE == sizeof(device_descriptor), "Invalid device_descriptor (size mismatch)");
    _Static_assert(CONFIGURATION_SIZE == sizeof(configuration_descriptor), "CONFIGURATION_SIZE calculation needs updating");
    _Static_assert(SUPPORTED_LANGUAGES_SIZE == sizeof(supported_languages), "Invalid supported_languages");
#ifdef FIXED_NUM_CONFIGURATIONS
    _Static_assert(FIXED_NUM_CONFIGURATIONS == CONFIGURATIONS_COUNT, "Wrong FIXED_NUM_CONFIGURATIONS");
#endif
}

#define DESCRIPTOR_COUNT ((int_fast8_t) (sizeof(descriptor_list)/sizeof(*descriptor_list)))

uint8_t
usb_descriptor_length_and_data (const uint16_t value, const uint16_t index, const char *address[static 1]) {
    for (int_fast8_t i = 0; i < DESCRIPTOR_COUNT; ++i) {
        if (pgm_read_word(&(descriptor_list[i].value)) != value) {
            continue;
        }
        if (pgm_read_word(&(descriptor_list[i].index)) != index) {
            continue;
        }
        *address = pgm_read_ptr(&(descriptor_list[i].data));
        return pgm_read_byte(&(descriptor_list[i].length));
    }

    return 0;
}

#if (USB_MAX_KEY_ROLLOVER + APPLE_BYTES_IN_REPORT) < USB_BOOT_PROTOCOL_ROLLOVER
#error "USB_MAX_KEY_ROLLOVER must be at least 6 (or 5 with ENABLE_APPLE_FN_KEY)"
#endif
#if MAX_KEY_ROLLOVER < USB_MAX_KEY_ROLLOVER
#error "MAX_KEY_ROLLOVER must be at least equal to USB_MAX_KEY_ROLLOVER"
#endif
#if ENABLE_APPLE_FN_KEY && USB_VENDOR_ID != USB_VENDOR_ID_APPLE
#error "USB_VENDOR_ID must be USB_VENDOR_ID_APPLE for ENABLE_APPLE_FN_KEY"
#endif
#if APPLE_FN_IS_MODIFIER && RESERVE_BOOT_PROTOCOL_RESERVED_BYTE
#error "APPLE_FN_IS_MODIFIER is not compatible with RESERVE_BOOT_PROTOCOL_RESERVED_BYTE"
#endif
