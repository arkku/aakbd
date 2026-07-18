BOOTLOADER_TYPE ?= dfu
MCU ?= atmega32u2
VENDOR_ID ?= 0x1209
PRODUCT_ID ?= 0x4704
MANUFACTURER ?= "Model F Labs"
XWHATSIT_CONTROLLER ?= wcass

# The capacitive sensing can't use asymmetric debounce, or it will produce
# phantom presses on other keys. Currently sym_defer_g is also the QMK default,
# but let's set it here again in case the QMK default changes.
DEBOUNCE_TYPE ?= sym_defer_g

# Empirically tested on multiple "brand new Model F keyboards", a debounce of
# 5 ms (the QMK default) is almost always enough, but 6 is safer. These aren't
# really keyboards that you would normally game on so defaulting to the safer
# value here, but please feel free to try lower values.
DEBOUNCE ?= 6

ISO_LAYOUT ?= 1
SPLIT_BACKSPACE ?= 0
SPLIT_RIGHT_SHIFT ?= 1
SPLIT_LEFT_SHIFT ?= $(ISO_LAYOUT)
SHORT_SPACE ?= 0

ifeq (1,$(BA_ENTER))
ISO_ENTER = 1
SPLIT_ENTER = 0
else
ISO_ENTER ?= $(ISO_LAYOUT)
SPLIT_ENTER ?= $(ISO_LAYOUT)
endif

HAPTIC_ENABLE ?= 0

include xwhatsit_core/xwhatsit_port.mk

ifeq (dfu,$(BOOTLOADER_TYPE))
DEVICE_FLAGS += -DBOOTLOADER_DFU=1 -DBOOTLOADER_ATMEL_DFU=1 -DBOOTLOADER_SIZE=4096
else
DEVICE_FLAGS += -DBOOTLOADER_DFU=0 -DBOOTLOADER_ATMEL_DFU=0 -DBOOTLOADER_SIZE=4096
endif

DEVICE_FLAGS += -DISO_LAYOUT=$(ISO_LAYOUT) -DSPLIT_BACKSPACE=$(SPLIT_BACKSPACE) -DSPLIT_RIGHT_SHIFT=$(SPLIT_RIGHT_SHIFT) -DSPLIT_LEFT_SHIFT=$(SPLIT_LEFT_SHIFT) -DISO_ENTER=$(ISO_ENTER) -DSPLIT_ENTER=$(SPLIT_ENTER) -DSHORT_SPACE=$(SHORT_SPACE) -DNO_PRINT -DDEVICE_VERSION=$(DEVICE_VERSION)

ifeq (1,$(HAPTIC_ENABLE))
DEVICE_FLAGS += -DHAPTIC_ENABLE=$(HAPTIC_ENABLE) -I$(QMK_DIR)/drivers/haptic
DEVICE_OBJS += solenoid.o haptic.o
endif

CONFIG_FLAGS += \
	-DUSB_VENDOR_ID=$(VENDOR_ID) \
	-DUSB_PRODUCT_ID=$(PRODUCT_ID) \
	-DMANUFACTURER_STRING='$(MANUFACTURER)' \
	-DPRODUCT_STRING='$(PRODUCT)'
