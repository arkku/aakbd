# gmmkpro1.mk: GMMK Pro device build overrides.

# Configuration defaults (override in local.mk or on command line)
ISO_LAYOUT ?= 1
MAX_KEY_ROLLOVER ?= 6
POLL_INTERVAL ?= 1
POWER_CONSUMPTION ?= 500
DEBOUNCE_TYPE ?= asym_eager_defer_pk
SIMULATED_TYPING ?= 1
ENABLE_HOST_FINGERPRINT ?= 1

VENDOR_ID ?= 0x320F
PRODUCT_ID ?= 0x5044
MANUFACTURER ?= "Glorious"
PRODUCT ?= $(if $(filter 1,$(ISO_LAYOUT)),"GMMK Pro ISO","GMMK Pro ANSI")
BOOTLOADER_TYPE = dfu

CONFIG_FLAGS ?= \
	-DUSB_VENDOR_ID=$(VENDOR_ID) \
	-DUSB_PRODUCT_ID=$(PRODUCT_ID) \
	-DMANUFACTURER_STRING='$(MANUFACTURER)' \
	-DPRODUCT_STRING='$(PRODUCT)' \
	-DDEVICE_VERSION=0x0001U \
	-DMAX_POWER_CONSUMPTION_MA=$(POWER_CONSUMPTION)

DEVICE_FLAGS += -DAW20216S_ENABLE -DENCODER_ENABLE -DRGB_MATRIX_ENABLE -DISO_LAYOUT=$(ISO_LAYOUT) -DBOOTMAGIC_ENABLE -DBOOTMAGIC_ROW=1 -DBOOTMAGIC_COLUMN=3 -DENABLE_BOOTLOADER_SHORTCUT=0

MCU_FAMILY = stm32f3

DEVICE_OBJS = matrix_gpio.o $(QMK_CORE_OBJS) aw20216s.o led_map.o encoder.o spi_master.o rgb_matrix.o gmmkpro1.o

# Select Vial layout JSON based on ISO/ANSI variant (must be before qmk_port.mk)
VIAL_JSON_FILE = $(DEVICE)/vial_$(if $(filter 1,$(ISO_LAYOUT)),iso,ansi).json

include arch/arm/stm32-common.mk
include qmk_core/qmk_port.mk

$(BUILDDIR)/keymap.o: gmmkpro1.h usb_keys.h $(COMMON_HEADERS)
$(BUILDDIR)/led_map.o: led_map.h aw20216s.h rgb_matrix.h usb_keys.h $(COMMON_HEADERS)
$(BUILDDIR)/gmmkpro1.o: generic_hid.h usbkbd.h usb_hardware.h keys.h rgb_matrix.h aw20216s.h led_map.h gpio.h wait.h $(COMMON_HEADERS)
