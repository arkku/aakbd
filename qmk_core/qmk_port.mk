QMK_DIR = qmk_core
QMK_PLATFORMS_DIR = $(QMK_DIR)/platforms
PLATFORM_DIR = $(QMK_PLATFORMS_DIR)/$(QMK_PLATFORM)

KEYBOARD_NAME ?= $(DEVICE)

DEVICE_FLAGS += -DKEYBOARD_NAME=$(KEYBOARD_NAME) -Dasm=__asm -DNO_PRINT -DNO_DEBUG -I$(QMK_DIR) -I$(QMK_PLATFORMS_DIR) -I$(PLATFORM_DIR) -Wno-old-style-declaration -Wno-pedantic -include config.h

include $(PLATFORM_DIR)/$(QMK_PLATFORM).mk

vpath %.c $(QMK_DIR) $(QMK_DIR)/debounce $(QMK_PLATFORMS_DIR)
vpath %.h $(QMK_DIR) $(QMK_PLATFORMS_DIR)

COMMON_HEADERS += config.h config_common.h pin_defs.h _pin_defs.h quantum.h platform_deps.h wait.h _wait.h matrix.h timer.h _timer.h gpio.h bitwise.h print.h util.h $(QMK_PLATFORM)/gpio.h eeconfig.h
COMMON_HEADERS += $(wildcard local.mk) $(wildcard $(DEVICE)/local.mk)

$(BUILDDIR)/qmk_main.o: keys.h led.h main.h usbkbd.h keyboard.h keymap.h qmk_port.h progmem.h suspend.h $(COMMON_HEADERS)

$(BUILDDIR)/keyboard.o: keyboard.h led.h $(COMMON_HEADERS)
$(BUILDDIR)/led.o: led.h debug.h host.h $(COMMON_HEADERS)
$(BUILDDIR)/matrix.o: matrix.h debounce.h debug.h action_layer.h $(COMMON_HEADERS)
$(BUILDDIR)/matrix_common.o: $(COMMON_HEADERS)
$(BUILDDIR)/i2c_master.o: i2c_master.h $(COMMON_HEADERS)
$(BUILDDIR)/bitwise.o: bitwise.h util.h
$(BUILDDIR)/suspend_core.o: suspend.h matrix.h

$(BUILDDIR)/sym_eager_pk.o: debounce.h $(COMMON_HEADERS)
$(BUILDDIR)/sym_eager_pr.o: debounce.h $(COMMON_HEADERS)
$(BUILDDIR)/sym_defer_pk.o: debounce.h $(COMMON_HEADERS)
$(BUILDDIR)/sym_defer_g.o: debounce.h $(COMMON_HEADERS)
$(BUILDDIR)/asym_eager_defer_pk.o: debounce.h $(COMMON_HEADERS)
$(BUILDDIR)/none.o: debounce.h $(COMMON_HEADERS)
