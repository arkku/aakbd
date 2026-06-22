QMK_DIR = qmk_core
QMK_PLATFORM ?= $(ARCH)
QMK_PLATFORMS_DIR = $(QMK_DIR)/platforms
PLATFORM_DIR = $(QMK_PLATFORMS_DIR)/$(QMK_PLATFORM)
KEYBOARD_NAME ?= $(DEVICE)

# Default debounce type (see QMK docs) and time (ms)
DEBOUNCE ?= 5

ifeq ($(DEBOUNCE),0)
DEBOUNCE_TYPE ?= none
else
DEBOUNCE_TYPE ?= sym_defer_g
endif

DEVICE_FLAGS += -DKEYBOARD_NAME=$(KEYBOARD_NAME) -Dasm=__asm -DNO_PRINT -DNO_DEBUG -I$(QMK_DIR) -I$(QMK_DIR)/drivers -I$(QMK_PLATFORMS_DIR) -I$(PLATFORM_DIR) -Wno-old-style-declaration -Wno-pedantic -include config.h

include $(PLATFORM_DIR)/qmk_$(QMK_PLATFORM).mk

vpath %.c $(QMK_DIR) $(QMK_DIR)/debounce $(PLATFORM_DIR) $(QMK_PLATFORMS_DIR) $(QMK_DIR)/drivers/haptic $(QMK_DIR)/drivers
vpath %.h $(QMK_DIR) $(PLATFORM_DIR) $(QMK_PLATFORMS_DIR) $(QMK_DIR)/drivers/haptic $(QMK_DIR)/drivers

COMMON_HEADERS += config.h pin_defs.h _pin_defs.h quantum.h platform_deps.h wait.h _wait.h matrix.h timer.h _timer.h gpio.h bitwise.h print.h util.h $(QMK_PLATFORM)/gpio.h eeconfig.h
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

QMK_CORE_OBJS = keyboard.o led.o qmk_port.o qmk_main.o $(KEYMAP_FILE).o matrix_common.o timer.o bitwise.o suspend.o suspend_core.o $(BOOTLOADER_TYPE).o $(DEBOUNCE_TYPE).o platform.o bootmagic.o eeconfig.o

ifneq ($(DEBOUNCE_TYPE),none)
DEVICE_FLAGS += -DDEBOUNCE=$(DEBOUNCE)

ifeq ($(DEBOUNCE_DEBUG),1)
DEVICE_FLAGS += -DDEBOUNCE_DEBUG=1
QMK_CORE_OBJS += debounce_debug.o
endif
endif

$(BUILDDIR)/qmk_main.o: keys.h led.h aakbd.h usb_hardware.h usbkbd.h usbkbd_config.h keyboard.h keymap.h qmk_port.h progmem.h suspend.h timer.h haptic.h bootloader.h $(COMMON_HEADERS)

$(BUILDDIR)/$(KEYMAP_FILE).o: keymap.h
$(BUILDDIR)/keyboard.o: keyboard.h led.h $(COMMON_HEADERS)
$(BUILDDIR)/led.o: keys.h led.h debug.h host.h $(COMMON_HEADERS)
$(BUILDDIR)/matrix_gpio.o: matrix.h debounce.h debug.h $(COMMON_HEADERS)
$(BUILDDIR)/matrix_common.o: debounce.h debug.h $(COMMON_HEADERS)
ifeq ($(QMK_PLATFORM),avr)
$(BUILDDIR)/i2c_master.o: i2c_master.h $(COMMON_HEADERS)
endif
$(BUILDDIR)/bitwise.o: bitwise.h util.h
$(BUILDDIR)/suspend_core.o: suspend.h matrix.h qmk_port.h
$(BUILDDIR)/haptic.o: haptic.h solenoid.h usb_device_state.h usb_hardware.h debug.h keyboard.h $(COMMON_HEADERS)
$(BUILDDIR)/solenoid.o: solenoid.h haptic.h usb_device_state.h usb_hardware.h $(COMMON_HEADERS)

$(BUILDDIR)/sym_eager_pk.o: debounce.h $(COMMON_HEADERS)
$(BUILDDIR)/sym_eager_pr.o: debounce.h $(COMMON_HEADERS)
$(BUILDDIR)/sym_defer_pk.o: debounce.h $(COMMON_HEADERS)
$(BUILDDIR)/sym_defer_g.o: debounce.h $(COMMON_HEADERS)
$(BUILDDIR)/asym_eager_defer_pk.o: debounce.h $(COMMON_HEADERS)
$(BUILDDIR)/asym_eager_defer_g.o: debounce.h $(COMMON_HEADERS)
$(BUILDDIR)/sym_eager_first_press_g.o: debounce.h $(COMMON_HEADERS)
$(BUILDDIR)/sym_eager_lone_press_g.o: sym_eager_first_press_g.c debounce.h $(COMMON_HEADERS)
$(BUILDDIR)/none.o: debounce.h $(COMMON_HEADERS)

$(BUILDDIR)/debounce_debug.o: matrix.h debounce.h usbkbd.h $(COMMON_HEADERS)

$(BUILDDIR)/encoder.o: encoder.h $(COMMON_HEADERS)
$(BUILDDIR)/rgb_matrix.o: rgb_matrix.h $(COMMON_HEADERS)
$(BUILDDIR)/aw20216s.o: aw20216s.h spi_master.h $(COMMON_HEADERS)
$(BUILDDIR)/bootmagic.o: bootmagic.h matrix.h keyboard.h wait.h eeconfig.h bootloader.h $(COMMON_HEADERS)
$(BUILDDIR)/spi_master.o: spi_master.h $(COMMON_HEADERS)
