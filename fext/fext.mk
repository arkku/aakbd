MCU ?= atmega32u4
BOOTLOADER_TYPE ?= caterina
VENDOR_ID ?= 0x0481
PRODUCT_ID ?= 0x0002
KEYBOARD_NAME = "keyboards/xwhatsit/ibm/fext/universal/universal"
DEBOUNCE ?= 5
XWHATSIT_CONTROLLER ?= universal

MANUFACTURER ?= "IBM Corp."
PRODUCT ?= "Model FEXT Keyboard"

include xwhatsit_core/xwhatsit_port.mk

DEVICE_FLAGS += -DMODELF=101
