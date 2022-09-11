QMK_PLATFORM = avr

include qmk_core/qmk_port.mk

XWHATSIT_DIR = xwhatsit_core
KEYBOARD_NAME ?= xwhatsit
DEBOUNCE_TYPE ?= sym_defer_g
DEVICE_VERSION ?= 0x0001

vpath %.c $(XWHATSIT_DIR)
vpath %.h $(XWHATSIT_DIR)

DEVICE_FLAGS += -I$(XWHATSIT_DIR) -DXWHATSIT=1
DEVICE_FLAGS += -DGENERIC_HID_REPORT_SIZE=32 -DGENERIC_HID_FEATURE_SIZE=16 -DGENERIC_HID_USAGE_PAGE=0xFF60U -DGENERIC_HID_USAGE=0x61 -DGENERIC_HID_INPUT_USAGE=0x62 -DGENERIC_HID_OUTPUT_USAGE=0x63 -DENABLE_GENERIC_HID_OUTPUT=1 -DGENERIC_HID_POLL_INTERVAL_MS=2

COMMON_HEADERS += post_config.h xwhatsit_port.h

DEVICE_OBJS ?= xwhatsit.o avrusb.o $(QMK_CORE_OBJS) util_comm.o eeconfig.o

ifeq (1,$(ERASE_CALIBRATION))
DEVICE_FLAGS = -DERASE_CALIBRATION_ON_START=1 -DCAPSENSE_CAL_VERSION=0
endif

$(BUILDDIR)/matrix.o: qmk_port.h progmem.h matrix_manipulate.h eeconfig.h $(COMMON_HEADERS)
$(BUILDDIR)/util_comm.o: util_comm.h matrix_manipulate.h $(COMMON_HEADERS)
$(BUILDDIR)/xwhatsit.o: led.h generic_hid.h
$(BUILDDIR)/keymap.o: keymap.h config.h usb_keys.h $(XWHATSIT_CONTROLLER).h
