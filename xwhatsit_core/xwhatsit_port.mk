include arch/avr/avr-common.mk
include qmk_core/qmk_port.mk

XWHATSIT_DIR = xwhatsit_core
KEYBOARD_NAME ?= xwhatsit
DEVICE_VERSION ?= 0x0001
ENABLE_GENERIC_HID_OUTPUT ?= 1

vpath %.c $(XWHATSIT_DIR)
vpath %.h $(XWHATSIT_DIR)

DEVICE_FLAGS += -I$(XWHATSIT_DIR) -DXWHATSIT=1
GENERIC_HID_FEATURE_SIZE ?= 16
ifeq (1,$(VIAL_ENABLE))
  # Vial: generic HID config is in vial/vial_config.h
else
  # xwhatsit config protocol uses separate usages for input and output
  DEVICE_FLAGS += -DGENERIC_HID_REPORT_SIZE=32 -DGENERIC_HID_FEATURE_SIZE=$(GENERIC_HID_FEATURE_SIZE) -DGENERIC_HID_USAGE_PAGE=0xFF60U -DGENERIC_HID_USAGE=0x61 -DGENERIC_HID_INPUT_USAGE=0x62 -DGENERIC_HID_OUTPUT_USAGE=0x63 -DENABLE_GENERIC_HID_OUTPUT=$(ENABLE_GENERIC_HID_OUTPUT) -DGENERIC_HID_POLL_INTERVAL_MS=2
endif

COMMON_HEADERS += post_config.h xwhatsit_port.h

DEVICE_OBJS ?= xwhatsit.o matrix.o $(QMK_CORE_OBJS) util_comm.o

ifeq (1,$(ERASE_CALIBRATION))
DEVICE_FLAGS += -DERASE_CALIBRATION_ON_START=1 -DCAPSENSE_CAL_VERSION=0
endif

$(BUILDDIR)/matrix.o: qmk_port.h progmem.h matrix_manipulate.h eeconfig.h $(COMMON_HEADERS)
$(BUILDDIR)/util_comm.o: util_comm.h matrix_manipulate.h $(COMMON_HEADERS)
$(BUILDDIR)/xwhatsit.o: led.h generic_hid.h
$(BUILDDIR)/keymap.o: keymap.h config.h usb_keys.h $(XWHATSIT_CONTROLLER).h
