VENDOR_ID ?= 0x3297U
PRODUCT_ID ?= 0x4974U
MANUFACTURER ?= "ZSA Technology Labs"
PRODUCT ?= "ErgoDox EZ"
DEBOUNCE ?= 8
DEBOUNCE_TYPE = sym_eager_pr
KEYBOARD_NAME = ergodox_ez
MCU ?= atmega32u4

QMK_PLATFORM = avr
BOOTLOADER_TYPE = halfkay
include qmk_core/qmk_port.mk

DEVICE_OBJS = keyboard.o qmk_main.o ergodox_ez.o matrix.o keymap.o i2c_master.o matrix_common.o timer.o bitwise.o led.o suspend.o suspend_core.o eeconfig.o platform.o $(QMK_PLATFORM).o $(BOOTLOADER_TYPE).o $(DEBOUNCE_TYPE).o
DEVICE_FLAGS += -DBOOTLOADER_HALFKAY -DBOOTLOADER_SIZE=512 -DENABLE_I2C=1
DEVICE_FLAGS += -DGENERIC_HID_REPORT_SIZE=22 -DGENERIC_HID_FEATURE_SIZE=2

CONFIG_FLAGS ?= \
	-DUSB_VENDOR_ID=$(VENDOR_ID) \
	-DUSB_PRODUCT_ID=$(PRODUCT_ID) \
	-DMANUFACTURER_STRING='$(MANUFACTURER)' \
	-DPRODUCT_STRING='$(PRODUCT)'
	-DDEBOUNCE=$(DEBOUNCE)

$(BUILDDIR)/ergodox.o: led.h ergodox_ez.h usbkbd.h usbkbd_config.h
$(BUILDDIR)/ergodox_ez.o: ergodox_ez.h i2c_master.h $(COMMON_HEADERS)
$(BUILDDIR)/keymap.o: device_keymap.h keymap.h ergodox_ez.h config.h usb_keys.h
$(BUILDDIR)/keys.o: device_keymap.h keymap.h ergodox_ez.h
