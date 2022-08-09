QMK_DIR = qmk_core
QMK_PLATFORMS_DIR = $(QMK_DIR)/platforms
PLATFORM_DIR = $(QMK_PLATFORMS_DIR)/$(QMK_PLATFORM)
DEBOUNCE_TYPE ?= sym_defer_g

KEYBOARD_NAME ?= $(DEVICE)

DEVICE_FLAGS += -DKEYBOARD_NAME=$(KEYBOARD_NAME) -Dasm=__asm -DNO_PRINT -DNO_DEBUG -I$(QMK_DIR) -I$(QMK_PLATFORMS_DIR) -I$(PLATFORM_DIR) -Wno-old-style-declaration -Wno-pedantic -include config.h

include $(PLATFORM_DIR)/$(QMK_PLATFORM).mk

vpath %.c $(QMK_DIR) $(QMK_DIR)/debounce $(QMK_PLATFORMS_DIR) $(QMK_DIR)/drivers/haptic
vpath %.h $(QMK_DIR) $(QMK_PLATFORMS_DIR) $(QMK_DIR)/drivers/haptic

COMMON_HEADERS += config.h config_common.h pin_defs.h _pin_defs.h quantum.h platform_deps.h wait.h _wait.h matrix.h timer.h _timer.h gpio.h bitwise.h print.h util.h $(QMK_PLATFORM)/gpio.h eeconfig.h
COMMON_HEADERS += $(wildcard local.mk) $(wildcard $(DEVICE)/local.mk)

ifneq (,$(CUSTOM_KEYMAP))
KEYMAP_FILE = keymap_$(CUSTOM_KEYMAP)
else
ifneq (,$(wildcard $(DEVICE)/keymap_custom.c))
KEYMAP_FILE = keymap_custom
else
KEYMAP_FILE = keymap
endif
endif

QMK_CORE_OBJS = keyboard.o led.o $(QMK_PLATFORM).o qmk_main.o $(KEYMAP_FILE).o matrix_common.o matrix.o timer.o bitwise.o suspend.o suspend_core.o $(BOOTLOADER_TYPE).o $(DEBOUNCE_TYPE).o platform.o usb_device_state.o

$(BUILDDIR)/qmk_main.o: keys.h led.h aakbd.h usb_hardware.h usbkbd.h usbkbd_config.h keyboard.h keymap.h qmk_port.h progmem.h suspend.h timer.h usb_device_state.h $(COMMON_HEADERS)

$(BUILDDIR)/$(KEYMAP_FILE).o: keymap.h
$(BUILDDIR)/keyboard.o: keyboard.h led.h $(COMMON_HEADERS)
$(BUILDDIR)/led.o: keys.h led.h debug.h host.h $(COMMON_HEADERS)
$(BUILDDIR)/matrix.o: matrix.h debounce.h debug.h action_layer.h $(COMMON_HEADERS)
$(BUILDDIR)/matrix_common.o: $(COMMON_HEADERS)
$(BUILDDIR)/i2c_master.o: i2c_master.h $(COMMON_HEADERS)
$(BUILDDIR)/bitwise.o: bitwise.h util.h
$(BUILDDIR)/suspend_core.o: suspend.h matrix.h
$(BUILDDIR)/usb_device_state.o: usb_device_state.h haptic.h
$(BUILDDIR)/haptic.o: haptic.h solenoid.h usb_device_state.h debug.h $(COMMON_HEADERS)
$(BUILDDIR)/solenoid.o: solenoid.h haptic.h $(COMMON_HEADERS)

$(BUILDDIR)/sym_eager_pk.o: debounce.h $(COMMON_HEADERS)
$(BUILDDIR)/sym_eager_pr.o: debounce.h $(COMMON_HEADERS)
$(BUILDDIR)/sym_defer_pk.o: debounce.h $(COMMON_HEADERS)
$(BUILDDIR)/sym_defer_g.o: debounce.h $(COMMON_HEADERS)
$(BUILDDIR)/asym_eager_defer_pk.o: debounce.h $(COMMON_HEADERS)
$(BUILDDIR)/none.o: debounce.h $(COMMON_HEADERS)
