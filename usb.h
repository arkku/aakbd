/**
 * usb.h: Helper definitions for implementing USB devices.
 *
 * Copied from various USB specifications and random websites. These seem to
 * work for my use cases, but no guarantees that either the naming or the
 * values are accurate.
 *
 * Composed by Kimmo Kulovesi, https://arkku.dev/
 * Provided with absolutely no warranty, use at your own risk only.
 * Use and distribute freely, mark modified copies as such.
 */

#ifndef KK_USB_H
#define KK_USB_H

#define LANGUAGE_ID_EN_US                   0x0409U

#define COUNTRY_CODE_NONE                   0
#define COUNTRY_CODE_ISO                    13
#define COUNTRY_CODE_FINNISH                7
#define COUNTRY_CODE_US                     33

#define DEVICE_NO_SPECIFIC_CLASS            0x00
#define DEVICE_CLASS_CDC                    0x02

#define DEVICE_NO_SPECIFIC_SUBCLASS         0x00

#define DEVICE_NO_SPECIFIC_PROTOCOL         0x00

#define DESCRIPTOR_TYPE_DEVICE              0x01
#define DESCRIPTOR_TYPE_CONFIGURATION       0x02
#define DESCRIPTOR_TYPE_STRING              0x03
#define DESCRIPTOR_TYPE_INTERFACE           0x04
#define DESCRIPTOR_TYPE_ENDPOINT            0x05
#define DESCRIPTOR_TYPE_DEVICE_QUALIFIER    0x06
#define DESCRIPTOR_TYPE_POWER               0x08
#define DESCRIPTOR_TYPE_HID                 0x21
#define DESCRIPTOR_TYPE_FUNCTIONAL          0x21
#define DESCRIPTOR_TYPE_CDC_INTERFACE       0x24
#define DESCRIPTOR_TYPE_CDC_ENDPOINT        0x25

#define DESCRIPTOR_SIZE_DEVICE              18
#define DESCRIPTOR_SIZE_DEVICE_QUALIFIER    10
#define DESCRIPTOR_SIZE_CONFIGURATION       9
#define DESCRIPTOR_SIZE_INTERFACE           9
#define DESCRIPTOR_SIZE_HID                 9
#define DESCRIPTOR_SIZE_FUNCTIONAL          9
#define DESCRIPTOR_SIZE_ENDPOINT            7

#define INTERFACE_NO_DESCRIPTOR             0x00

#define HID_USAGE_PAGE                      0x05
#define HID_USAGE_PAGE_WORD                 0x06
#define HID_USAGE_PAGE_GENERIC_DESKTOP      0x01
#define HID_USAGE_PAGE_KEYCODES             0x07
#define HID_USAGE_PAGE_LEDS                 0x08
#define HID_USAGE_PAGE_CONSUMER_DEVICES     0x0C
#define HID_USAGE_PAGE_VENDOR_RESERVED      0xFF

#define HID_USAGE                           0x09
#define HID_USAGE_WORD                      0x0A
#define HID_USAGE_KEYBOARD                  0x06
#define HID_USAGE_CONSUMER_CONTROL          0x01

#define HID_USAGE_APPLE_VENDOR_BR_UP_KEY    0x20 // 0x04 in top cover
#define HID_USAGE_APPLE_VENDOR_BR_DOWN_KEY  0x21 // 0x05 in top cover
#define HID_USAGE_APPLE_VENDOR_POWER_KEY    0x04
#define HID_USAGE_APPLE_VENDOR_GLOBE_KEY    0x01

#define HID_USAGE_PAGE_VENDOR_APPLE_TOP_COVER   0xFF
#define HID_USAGE_APPLE_FN_KEY                  0x03
#define HID_USAGE_PAGE_VENDOR_APPLE_KEYBOARD    0xFF01U
#define HID_USAGE_APPLE_BRIGHTNESS_UP           0x20 // 0x04 in 0x00FF page
#define HID_USAGE_APPLE_BRIGHTNESS_DOWN         0x21 // 0x05 in 0x00FF page
#define HID_USAGE_APPLE_SPOTLIGHT               0x01
#define HID_USAGE_APPLE_DASHBOARD               0x02
#define HID_USAGE_APPLE_LAUNCHPAD               0x04
#define HID_USAGE_APPLE_EXPOSE                  0x10
#define HID_USAGE_APPLE_EXPOSE_DESKTOP          0x11

#define HID_USAGE_MINIMUM                   0x19
#define HID_USAGE_MINIMUM_WORD              0x1A
#define HID_USAGE_MAXIMUM                   0x29
#define HID_USAGE_MAXIMUM_WORD              0x2A

#define HID_LOGICAL_MINIMUM                 0x15
#define HID_LOGICAL_MINIMUM_WORD            0x16
#define HID_LOGICAL_MAXIMUM                 0x25
#define HID_LOGICAL_MAXIMUM_WORD            0x26

#define HID_COLLECTION                      0xA1
#define HID_COLLECTION_APPLICATION          0x01

#define HID_REPORT_SIZE                     0x75
#define HID_REPORT_COUNT                    0x95
#define HID_REPORT_ID                       0x85

#define HID_INPUT                           0x81
#define HID_OUTPUT                          0x91
#define HID_FEATURE                         0xB1
#define HID_IO_ARRAY                        0x00
#define HID_IO_CONSTANT                     0x01
#define HID_IO_VARIABLE                     0x02

#define HID_IO_ABSOLUTE                     0x00
#define HID_IO_RELATIVE                     0x04
#define HID_IO_RELATIVE_VARIABLE            (HID_IO_VARIABLE | HID_IO_RELATIVE)

#define HID_END_COLLECTION                  0xC0

#define CONFIGURATION_ATTRIBUTES_RESERVED               (1 << 7)
#define CONFIGURATION_ATTRIBUTES_SELF_POWERED_FLAG      (1 << 6)
#define CONFIGURATION_ATTRIBUTES_REMOTE_WAKE_UP_FLAG    (1 << 5)

#define INTERFACE_CLASS_CDC                     0x02
#define INTERFACE_CLASS_HID                     0x03
#define INTERFACE_CLASS_APPLICATION_SPECIFIC    0xFE

#define INTERFACE_NO_SPECIFIC_SUBCLASS      0x00
#define INTERFACE_SUBCLASS_BOOT             0x01
#define INTERFACE_SUBCLASS_ACM              0x02
#define INTERFACE_SUBCLASS_APPLICATION_DFU  0x01

#define INTERFACE_NO_SPECIFIC_PROTOCOL      0x00
#define INTERFACE_PROTOCOL_BOOT_KEYBOARD    0x01
#define INTERFACE_PROTOCOL_BOOT_MOUSE       0x02
#define INTERFACE_PROTOCOL_AT_COMMAND       0x01
#define INTERFACE_PROTOCOL_DFU_RUNTIME      0x01
#define INTERFACE_PROTOCOL_VENDOR_SPECIFC   0xFF

#define HID_PROTOCOL_BOOT                   0
#define HID_PROTOCOL_REPORT                 1

#define HID_DESCRIPTOR_TYPE_REPORT          0x22

#define ENDPOINT_DIR_IN                     0x80
#define ENDPOINT_DIR_OUT                    0x00
#define ENDPOINT_ATTRIBUTES_INTERRUPT       0x03

#define STRING_DESCRIPTOR_HEADER_SIZE       2

#define USB_REQUEST_DIRECTION_TO_DEVICE     0x00
#define USB_REQUEST_DIRECTION_TO_HOST       0x80
#define USB_REQUEST_DIRECTION_MASK          0x80

#define USB_REQUEST_TYPE_STANDARD           0x00
#define USB_REQUEST_TYPE_CLASS              0x20
#define USB_REQUEST_TYPE_VENDOR             0x40
#define USB_REQUEST_TYPE_MASK               0x60

#define USB_REQUEST_RECIPIENT_DEVICE        0x00
#define USB_REQUEST_RECIPIENT_INTERFACE     0x01
#define USB_REQUEST_RECIPIENT_ENDPOINT      0x02
#define USB_REQUEST_RECIPIENT_OTHER         0x03
#define USB_REQUEST_RECIPIENT_MASK          0x03

#define USB_REQUEST_DEVICE_TO_HOST_CLASS_INTERFACE          (USB_REQUEST_DIRECTION_TO_HOST | USB_REQUEST_TYPE_CLASS | USB_REQUEST_RECIPIENT_INTERFACE)
#define USB_REQUEST_HOST_TO_DEVICE_CLASS_INTERFACE          (USB_REQUEST_DIRECTION_TO_DEVICE | USB_REQUEST_TYPE_CLASS | USB_REQUEST_RECIPIENT_INTERFACE)
#define USB_REQUEST_DEVICE_TO_HOST_STANDARD_INTERFACE       (USB_REQUEST_DIRECTION_TO_HOST | USB_REQUEST_TYPE_STANDARD | USB_REQUEST_RECIPIENT_INTERFACE)
#define USB_REQUEST_DEVICE_TO_HOST_STANDARD_DEVICE          (USB_REQUEST_DIRECTION_TO_HOST | USB_REQUEST_TYPE_STANDARD | USB_REQUEST_RECIPIENT_DEVICE)
#define USB_REQUEST_HOST_TO_DEVICE_STANDARD_DEVICE          (USB_REQUEST_DIRECTION_TO_DEVICE | USB_REQUEST_TYPE_STANDARD | USB_REQUEST_RECIPIENT_DEVICE)
#define USB_REQUEST_HOST_TO_DEVICE_STANDARD_ENDPOINT        (USB_REQUEST_DIRECTION_TO_DEVICE | USB_REQUEST_TYPE_STANDARD | USB_REQUEST_RECIPIENT_ENDPOINT)
#define USB_REQUEST_DEVICE_TO_HOST_STANDARD_ENDPOINT        (USB_REQUEST_DIRECTION_TO_HOST | USB_REQUEST_TYPE_STANDARD | USB_REQUEST_RECIPIENT_ENDPOINT)
#define USB_REQUEST_DEVICE_TO_HOST_CLASS_INTERFACE          (USB_REQUEST_DIRECTION_TO_HOST | USB_REQUEST_TYPE_CLASS | USB_REQUEST_RECIPIENT_INTERFACE)
#define USB_REQUEST_HOST_TO_DEVICE_CLASS_INTERFACE          (USB_REQUEST_DIRECTION_TO_DEVICE | USB_REQUEST_TYPE_CLASS | USB_REQUEST_RECIPIENT_INTERFACE)

#define USB_REQUEST_GET_STATUS              0
#define USB_REQUEST_CLEAR_FEATURE           1
#define USB_REQUEST_SET_FEATURE             3
#define USB_REQUEST_SET_ADDRESS             5
#define USB_REQUEST_GET_DESCRIPTOR          6
#define USB_REQUEST_SET_DESCRIPTOR          7
#define USB_REQUEST_GET_CONFIGURATION       8
#define USB_REQUEST_SET_CONFIGURATION       9
#define USB_REQUEST_GET_INTERFACE           10
#define USB_REQUEST_SET_INTERFACE           11
#define USB_REQUEST_SYNCH_FRAME             12

#define USB_FEATURE_DEVICE_REMOTE_WAKEUP    1
#define USB_FEATURE_HALT_ENDPOINT           2
#define USB_FEATURE_TEST_MODE               3

#define USB_STATUS_SELF_POWERED_ENABLED     (1 << 0)
#define USB_STATUS_REMOTE_WAKEUP_ENABLED    (1 << 1)

#define HID_REQUEST_FLAG_SET                8
#define HID_REQUEST_GET_REPORT              1
#define HID_REQUEST_GET_IDLE                2
#define HID_REQUEST_GET_PROTOCOL            3
#define HID_REQUEST_SET_REPORT              (HID_REQUEST_GET_REPORT | HID_REQUEST_FLAG_SET)
#define HID_REQUEST_SET_IDLE                (HID_REQUEST_GET_REPORT | HID_REQUEST_FLAG_SET)
#define HID_REQUEST_SET_PROTOCOL            (HID_REQUEST_GET_PROTOCOL | HID_REQUEST_FLAG_SET)

#define DFU_REQUEST_DETACH                  0
#define DFU_REQUEST_DOWNLOAD                1
#define DFU_REQUEST_UPLOAD                  2
#define DFU_REQUEST_GET_STATUS              3
#define DFU_REQUEST_CLEAR_STATUS            4
#define DFU_REQUEST_GET_STATE               5
#define DFU_REQUEST_ABORT                   6

#define DFU_APP_STATE_IDLE                  0
#define DFU_APP_STATE_DETACH                1
#define DFU_STATUS_OK                       0

#define DFU_ATTRIBUTE_CAN_DOWNLOAD              (1 << 0)
#define DFU_ATTRIBUTE_CAN_UPLOAD                (1 << 1)
#define DFU_ATTRIBUTE_MANIFESTATION_TOLERANT    (1 << 2)
#define DFU_ATTRIBUTE_WILL_DETACH               (1 << 3)

#define CDC_INTERFACE_HEADER                0
#define CDC_INTERFACE_CALL_MANAGEMENT       1
#define CDC_INTERFACE_ACM                   2
#define CDC_INTERFACE_DIRECT_LINE           3
#define CDC_INTERFACE_TELEPHONE_RINGER      4
#define CDC_INTERFACE_TELEPHONE_CALL        5
#define CDC_INTERFACE_UNION                 6
#define CDC_INTERFACE_COUNTRY_SELECTION     7
#define CDC_INTERFACE_TELEPHONE_OP_MODES    8
#define CDC_INTERFACE_USB_TERMINAL          9
#define CDC_INTERFACE_NETWORK_CHANNEL       10
#define CDC_INTERFACE_PROTOCOL_UNIT         11
#define CDC_INTERFACE_EXTENSION_UNIT        12
#define CDC_INTERFACE_MULTI_CHANNEL         13
#define CDC_INTERFACE_CAPI                  14
#define CDC_INTERFACE_ETHERNET              15
#define CDC_INTERFACE_ATM                   16

#define CDC_LINE_ENCODING_1_STOP_BIT        0
#define CDC_LINE_ENCODING_1_HALF_STOP_BITS  1
#define CDC_LINE_ENCODING_2_STOP_BITS       2

#define CDC_PARITY_NONE                     0
#define CDC_PARITY_ODD                      1
#define CDC_PARITY_EVEN                     2
#define CDC_PARITY_MARK                     3
#define CDC_PARITY_SPACE                    4

#define CDC_NOTIFICATION_SERIAL_STATE       0x20

#define CDC_REQUEST_SEND_ENCAPSULATED_CMD   0x00
#define CDC_REQUEST_GET_ENCAPSULATED_REPLY  0x01
#define CDC_REQUEST_SET_LINE_CODING         0x20
#define CDC_REQUEST_GET_LINE_CODING         0x21
#define CDC_REQUEST_SET_CONTROL_LINE_STATE  0x22
#define CDC_REQUEST_SEND_BREAK              0x23

#define CDC_CONTROL_LINE_ERROR_FRAME        (1 << 4)
#define CDC_CONTROL_LINE_ERROR_PARITY       (1 << 5)
#define CDC_CONTROL_LINE_ERROR_OVERFLOW     (1 << 6)

/// The number of simultaneous keys supported by the boot protocol keyboard.
#define USB_BOOT_PROTOCOL_ROLLOVER          6

#define USB_VENDOR_ID_APPLE                 0x05ACU

#endif
