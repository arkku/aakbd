QMK_DIR = qmk_core
TMK_DIR = tmk_core/common

KEYBOARD_NAME ?= $(DEVICE)

DEVICE_FLAGS += -DKEYBOARD_NAME=$(KEYBOARD_NAME) -Dasm=__asm -DNO_PRINT -DNO_DEBUG -I$(QMK_DIR) -I$(TMK_DIR) -I$(TMK_DIR)/avr -Wno-old-style-declaration -Wno-pedantic
DEVICE_FLAGS += -funsigned-char
DEVICE_FLAGS += -funsigned-bitfields
DEVICE_FLAGS += -ffunction-sections
DEVICE_FLAGS += -fdata-sections
DEVICE_FLAGS += -fpack-struct
DEVICE_FLAGS += -fshort-enums

vpath %.c $(TMK_DIR) $(TMK_DIR)/avr $(QMK_DIR) $(QMK_DIR)/debounce
vpath %.h $(TMK_DIR) $(TMK_DIR)/avr $(QMK_DIR)

COMMON_HEADERS += config.h config_common.h pin_defs.h quantum.h platform_deps.h wait.h matrix.h timer.h gpio.h bitwise.h print.h _wait.h _timer.h util.h avr/gpio.h
COMMON_HEADERS += $(wildcard local.mk) $(wildcard $(DEVICE)/local.mk)

$(BUILDDIR)/qmk_main.o: avrtimer.h keys.h led.h main.h usbkbd.h keymap.h $(COMMON_HEADERS)
$(BUILDDIR)/matrix.o: matrix.h debounce.h debug.h action_layer.h $(COMMON_HEADERS)
$(BUILDDIR)/i2c_master.o: i2c_master.h $(COMMON_HEADERS)
$(BUILDDIR)/matrix_common.o: $(COMMON_HEADERS)
$(BUILDDIR)/timer.o: timer.h timer_avr.h _timer.h
$(BUILDDIR)/bitwise.o: bitwise.h util.h
$(BUILDDIR)/suspend.o: suspend.h $(COMMON_HEADERS)
$(BUILDDIR)/bootloader.o: bootloader.h
$(BUILDDIR)/sym_eager_pk.o: debounce.h $(COMMON_HEADERS)
$(BUILDDIR)/sym_eager_pr.o: debounce.h $(COMMON_HEADERS)
$(BUILDDIR)/sym_defer_pk.o: debounce.h $(COMMON_HEADERS)
$(BUILDDIR)/sym_defer_g.o: debounce.h $(COMMON_HEADERS)
$(BUILDDIR)/asym_eager_defer_pk.o: debounce.h $(COMMON_HEADERS)
$(BUILDDIR)/none.o: debounce.h $(COMMON_HEADERS)
