#ifndef KK_USBKBD_DESCRIPTORS_H
#define KK_USBKBD_DESCRIPTORS_H

#include <stdint.h>

#include "usb.h"
#include "usbkbd_config.h"
#include "progmem.h"

// MARK: - Configuration

#if APPLE_FN_IS_MODIFIER
#define APPLE_BYTES_IN_REPORT ENABLE_EXTRA_APPLE_KEYS
#else
#define APPLE_BYTES_IN_REPORT ENABLE_APPLE_FN_KEY
#endif

#ifndef RESERVE_BOOT_PROTOCOL_RESERVED_BYTE
#if USE_MULTIPLE_REPORTS
#define RESERVE_BOOT_PROTOCOL_RESERVED_BYTE 0
#elif APPLE_BYTES_IN_REPORT
#if USB_MAX_KEY_ROLLOVER >= USB_BOOT_PROTOCOL_ROLLOVER
#define RESERVE_BOOT_PROTOCOL_RESERVED_BYTE 0
#endif
#elif USB_MAX_KEY_ROLLOVER > USB_BOOT_PROTOCOL_ROLLOVER
#define RESERVE_BOOT_PROTOCOL_RESERVED_BYTE 0
#else
// If we don't exceed the boot protocol report size, the boot protocol
// reserved byte is kept as zero. Otherwise we can claim it to use for the
// last key, which should be fine since it will be zero unless we are at
// maximum rollover.
#define RESERVE_BOOT_PROTOCOL_RESERVED_BYTE 1
#endif
#endif

#if RESERVE_BOOT_PROTOCOL_RESERVED_BYTE || APPLE_BYTES_IN_REPORT
#define KEY_IN_RESERVED_BYTE 0
#else
#define KEY_IN_RESERVED_BYTE 1
#endif

#define KEYBOARD_REPORT_SIZE        (1 + RESERVE_BOOT_PROTOCOL_RESERVED_BYTE + USB_MAX_KEY_ROLLOVER + APPLE_BYTES_IN_REPORT)

#if KEYBOARD_REPORT_SIZE <= 8
#define KEYBOARD_ENDPOINT_SIZE      8
#elif KEYBOARD_REPORT_SIZE <= 16
#define KEYBOARD_ENDPOINT_SIZE      16
#elif KEYBOARD_REPORT_SIZE <= 32
#define KEYBOARD_ENDPOINT_SIZE      32
#else
#define KEYBOARD_ENDPOINT_SIZE      64
#endif

#define KEYBOARD_INTERFACE_INDEX    0
#define KEYBOARD_CONFIGURATION      1
#define KEYBOARD_ENDPOINT_NUM       1
#define KEYBOARD_ENDPOINT_ADDRESS   (ENDPOINT_DIR_IN | KEYBOARD_ENDPOINT_NUM)

#ifdef FIXED_CONTROL_ENDPOINT_SIZE
#define ENDPOINT_0_SIZE             FIXED_CONTROL_ENDPOINT_SIZE
#else
#define ENDPOINT_0_SIZE             64
#endif

#define CONFIGURATIONS_COUNT        1

#if ENABLE_GENERIC_HID_ENDPOINT
#define GENERIC_HID_ENDPOINT_COUNT  (1 + ENABLE_GENERIC_HID_OUTPUT)
#else
#define GENERIC_HID_ENDPOINT_COUNT  0
#endif

#define ENDPOINT_COUNT              ((ENABLE_KEYBOARD_ENDPOINT | ENABLE_DFU_INTERFACE) + GENERIC_HID_ENDPOINT_COUNT)
#define INTERFACES_COUNT            (ENABLE_KEYBOARD_ENDPOINT + ENABLE_GENERIC_HID_ENDPOINT + ENABLE_DFU_INTERFACE)
#define SUPPORTED_LANGUAGE_COUNT    1

#if ENABLE_DFU_INTERFACE
#if !ENABLE_KEYBOARD_ENDPOINT
#error "ENABLE_DFU_INTERFACE requires ENABLE_KEYBOARD_ENDPOINT"
#endif
#define DFU_INTERFACE_INDEX         (INTERFACES_COUNT - 1)
#endif // ^ ENABLE_DFU_INTERFACE

#if ENABLE_GENERIC_HID_ENDPOINT
#define GENERIC_ENDPOINT_ADDRESS_IN     (ENDPOINT_DIR_IN | GENERIC_HID_ENDPOINT_IN_NUM)
#define GENERIC_INTERFACE_INDEX         (INTERFACES_COUNT - (ENABLE_KEYBOARD_ENDPOINT + ENABLE_DFU_INTERFACE))

#if ENABLE_GENERIC_HID_OUTPUT
#define GENERIC_ENDPOINT_ADDRESS_OUT    (ENDPOINT_DIR_OUT | GENERIC_HID_ENDPOINT_OUT_NUM)
#endif

#if GENERIC_HID_REPORT_SIZE > GENERIC_HID_FEATURE_SIZE
#define GENERIC_HID_MAX_SIZE GENERIC_HID_REPORT_SIZE
#else
#define GENERIC_HID_MAX_SIZE GENERIC_HID_FEATURE_SIZE
#endif

#if GENERIC_HID_MAX_SIZE > 32
#define GENERIC_ENDPOINT_SIZE       64
#elif GENERIC_HID_MAX_SIZE > 16
#define GENERIC_ENDPOINT_SIZE       32
#elif GENERIC_HID_MAX_SIZE > 8
#define GENERIC_ENDPOINT_SIZE       16
#else
#define GENERIC_ENDPOINT_SIZE       8
#endif
#endif // ^ ENABLE_GENERIC_HID_ENDPOINT

#define STRING_INDEX_LANGUAGE       0
#define STRING_INDEX_MANUFACTURER   1
#define STRING_INDEX_PRODUCT        2
#define STRING_INDEX_SERIAL_NUMBER  3

#ifndef USB_STRINGS_STORED_AS_ASCII
/// If USB_STRINGS_STORED_AS_ASCII is 1, then descriptor memory is saved by
/// storing USB strings (e.g., manufacturer and product names) as ASCII,
/// which is converted on the fly to the expected UTF-16 (doubling the size).
///
/// However, if storage space is available, it is simpler to store as UTF-16
/// directly. Also, not all USB implementations/platforms may support the
/// on-the-fly conversion.
#define USB_STRINGS_STORED_AS_ASCII 0
#endif

// MARK: - Descriptors

struct usb_descriptor {
    /// The value matching `wValue` in the USB request for this descriptor.
    /// The MSB is the type of descriptor and the LSB the index among
    /// descriptors of that type (but also see `index`).
    uint16_t    value;

    /// The index matching `wIndex` in the USB request for this descriptor.
    /// The meaning of this depends on the type of descriptor, e.g., for
    /// strings it is the language id, for interfaces it is the interface
    /// number, etc.
    uint16_t    index;

    /// A pointer to the `PROGMEM` stored data for the descriptor. Normally it
    /// is sent as is, but if `USB_STRINGS_STORED_AS_ASCII` is non-zero, and
    /// the type is string, and this is not the list of supported languages,
    /// then the data is expected to be a plain ASCII string that will be
    /// converted to UTF-16 when sending.
    const char *data;

    /// The length of the descriptor. Note that for ASCII strings (see `data`),
    /// this is the converted length (conveniently `2 * sizeof(str)` since the
    /// NUL terminator covers the header).
    uint8_t     length;
};

extern const struct usb_descriptor PROGMEM descriptor_list[];

/// Does any runtime setup needed for the USB descriptors.
void usb_descriptors_init(void);

/// Given the `wValue` and `wIndex` of the USB descriptor request, returns
/// the length of the matching descriptor, and sets the `address` pointer
/// (which must be non-NULL) to the address of the data for that descriptor.
/// If no matching descriptor is found, returns 0.
uint8_t usb_descriptor_length_and_data(const uint16_t value, const uint16_t index, const char *address[static 1]);

#endif
