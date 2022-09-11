QMK_PLATFORM = avr
BOOTLOADER_TYPE ?= dfu
MCU ?= atmega32u2
VENDOR_ID ?= 0x1209
PRODUCT_ID ?= 0x4704
MANUFACTURER ?= "Model F Labs"
DEBOUNCE_TYPE = sym_defer_g
MODELF_CONTROLLER ?= wcass
DEVICE_VERSION ?= 0x0001

ISO_LAYOUT ?= 1
SPLIT_BACKSPACE ?= 0
SPLIT_RIGHT_SHIFT ?= 1
SPLIT_LEFT_SHIFT ?= $(ISO_LAYOUT)
ISO_ENTER ?= $(ISO_LAYOUT)
SPLIT_ENTER ?= 0
SHORT_SPACE ?= 0

HAPTIC_ENABLE ?= 0

include qmk_core/qmk_port.mk
include xwhatsit_core/xwhatsit_port.mk

ifeq (dfu,$(BOOTLOADER_TYPE))
DEVICE_FLAGS += -DBOOTLOADER_DFU=1 -DBOOTLOADER_ATMEL_DFU=1 -DBOOTLOADER_SIZE=4096
else
DEVICE_FLAGS += -DBOOTLOADER_DFU=0 -DBOOTLOADER_ATMEL_DFU=0 -DBOOTLOADER_SIZE=4096
endif

DEVICE_OBJS = modelf.o avrusb.o $(QMK_CORE_OBJS) util_comm.o eeconfig.o
DEVICE_FLAGS += -DGENERIC_HID_REPORT_SIZE=32 -DGENERIC_HID_FEATURE_SIZE=16 -DGENERIC_HID_USAGE_PAGE=0xFF60U -DGENERIC_HID_USAGE=0x61 -DGENERIC_HID_INPUT_USAGE=0x62 -DGENERIC_HID_OUTPUT_USAGE=0x63 -DENABLE_GENERIC_HID_OUTPUT=1 -DGENERIC_HID_POLL_INTERVAL_MS=2 -DISO_LAYOUT=$(ISO_LAYOUT) -DSPLIT_BACKSPACE=$(SPLIT_BACKSPACE) -DSPLIT_RIGHT_SHIFT=$(SPLIT_RIGHT_SHIFT) -DSPLIT_LEFT_SHIFT=$(SPLIT_LEFT_SHIFT) -DISO_ENTER=$(ISO_ENTER) -DSPLIT_ENTER=$(SPLIT_ENTER) -DSHORT_SPACE=$(SHORT_SPACE) -DNO_PRINT -DDEVICE_VERSION=$(DEVICE_VERSION)

ifeq (1,$(HAPTIC_ENABLE))
DEVICE_FLAGS += -DHAPTIC_ENABLE=$(HAPTIC_ENABLE) -I$(QMK_DIR)/drivers/haptic
DEVICE_OBJS += solenoid.o haptic.o
endif

CONFIG_FLAGS ?= \
	-DUSB_VENDOR_ID=$(VENDOR_ID) \
	-DUSB_PRODUCT_ID=$(PRODUCT_ID) \
	-DMANUFACTURER_STRING='$(MANUFACTURER)' \
	-DPRODUCT_STRING='$(PRODUCT)' \
	-DDEBOUNCE=$(DEBOUNCE)

$(BUILDDIR)/modelf.o: led.h generic_hid.h
$(BUILDDIR)/keymap.o: keymap.h config.h usb_keys.h $(MODELF_CONTROLLER).h
