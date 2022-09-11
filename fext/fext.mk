KEYBOARD_NAME = "keyboards/xwhatsit/ibm/fext/universal/universal"
MANUFACTURER ?= "IBM Corp."
PRODUCT ?= "Model FEXT Keyboard"
DEBOUNCE ?= 5
MCU ?= atmega32u4
BOOTLOADER_TYPE ?= caterina
MODELF_CONTROLLER ?= universal
VENDOR_ID ?= 0x0481
PRODUCT_ID ?= 0x0002

vpath modelf.c modelf77

include modelf77/modelf-common.mk

DEVICE_FLAGS += -DMODELF=101
