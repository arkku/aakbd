#ifndef KK_USBKBD_DESCRIPTORS_H
#define KK_USBKBD_DESCRIPTORS_H

#include <stdint.h>

#include "usb.h"
#include "usbkbd.h"

#if defined(__AVR__)
#include <avr/pgmspace.h>
#elif !defined(PROGMEM)
#define PROGMEM
#endif

// MARK: - Configuration

#if USE_MULTIPLE_REPORTS
#define RESERVE_BOOT_PROTOCOL_RESERVED_BYTE 0
#elif !ENABLE_APPLE_FN_KEY
#if USB_MAX_KEY_ROLLOVER > USB_BOOT_PROTOCOL_ROLLOVER
#define RESERVE_BOOT_PROTOCOL_RESERVED_BYTE 0
#endif
#elif USB_MAX_KEY_ROLLOVER >= USB_BOOT_PROTOCOL_ROLLOVER
#define RESERVE_BOOT_PROTOCOL_RESERVED_BYTE 0
#endif
#ifndef RESERVE_BOOT_PROTOCOL_RESERVED_BYTE
// If we don't exceed the boot protocol report size, the boot protocol
// reserved byte is kept as zero. Otherwise we can claim it to use for the
// last key, which should be fine since it will be zero unless we are at
// maximum rollover.
#define RESERVE_BOOT_PROTOCOL_RESERVED_BYTE 1
#endif

#define KEYBOARD_REPORT_SIZE        (1 + RESERVE_BOOT_PROTOCOL_RESERVED_BYTE + USB_MAX_KEY_ROLLOVER + ENABLE_APPLE_FN_KEY)

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

#define ENDPOINT_0_SIZE             64

#define CONFIGURATIONS_COUNT        1
#define ENDPOINT_COUNT              ((ENABLE_KEYBOARD_ENDPOINT | ENABLE_DFU_INTERFACE) + ENABLE_GENERIC_HID_ENDPOINT)
#define INTERFACES_COUNT            (ENABLE_KEYBOARD_ENDPOINT + ENABLE_GENERIC_HID_ENDPOINT + ENABLE_DFU_INTERFACE)
#define SUPPORTED_LANGUAGE_COUNT    1

#if ENABLE_DFU_INTERFACE
#if !ENABLE_KEYBOARD_ENDPOINT
#error "ENABLE_DFU_INTERFACE requires ENABLE_KEYBOARD_ENDPOINT"
#endif
#define DFU_INTERFACE_INDEX         (INTERFACES_COUNT - 1)
#endif // ^ ENABLE_DFU_INTERFACE

#if ENABLE_GENERIC_HID_ENDPOINT
#define GENERIC_ENDPOINT_ADDRESS    (ENDPOINT_DIR_IN | GENERIC_HID_ENDPOINT_NUM)
#define GENERIC_INTERFACE_INDEX     (INTERFACES_COUNT - (ENABLE_KEYBOARD_ENDPOINT + ENABLE_DFU_INTERFACE))

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
    /// is sent as is, but if the type string and this is not the list of
    /// supported languages, then the data is expected to be a plain ASCII
    /// string that will be converted to UTF-16 when sending.
    const char *data;

    /// The length of the descriptor. Note that for ASCII strings (see `data`),
    /// this is the converted length (conveniently `2 * sizeof(str)` since the
    /// NUL terminator covers the header).
    uint8_t     length;
};

extern const uint8_t descriptor_count;
extern const struct usb_descriptor PROGMEM descriptor_list[];

void usb_descriptors_init(void);

// MARK: - Helper macros

#if defined(__GNUC__) || defined(__clang__)
#define INLINE inline __attribute__((always_inline))
#else
#define INLINE inline
#endif

/// The least significant byte of `word`.
#define LSB(word)                   ((word) & 0xFF)

/// The most signifant byte of `word`.
#define MSB(word)                   ((word) >> 8)

/// The bytes of `word` in little endian order (array construction helper).
#define WORD_BYTES(word)            LSB(word), MSB(word)

/// Form a 16-bit word from `lsb` and `msb`.
#define BYTES_WORD(lsb, msb)        (((msb) << 8) | (lsb))

/// Divide `value` by `n` and round up.
#define DIV_ROUND_BYTE(n, value)    (((value) / (n)) + ((value) & 1))

#endif
