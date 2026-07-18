# Makefile
# Copyright (c) 2021-2026 Kimmo Kulovesi, https://arkku.dev/
###############################################################################
# Put your local settings in "local.mk", it is ignored by Git.
# Set NO_LOCAL_MK=1 on command line for release builds to skip local.mk.
ifndef NO_LOCAL_MK
-include local.mk
endif

OPTIMIZATION ?= s
CFLAGS=-O$(OPTIMIZATION) -Wall -std=gnu11 -Wextra -Wno-unused-parameter $(CUSTOM_FLAGS) $(CC_FLAGS) $(DEVICE_FLAGS) $(CONFIG_FLAGS) $(EXTRA_FLAGS)
LDFLAGS=-O$(OPTIMIZATION) $(LD_FLAGS)
AR ?= avr-ar
ARFLAGS=rcs

CC_FLAGS=-I$(DEVICE) -I. -Iarch/$(ARCH)
LD_FLAGS=

# Default goal must be the first target in the Makefile
all:

ifneq (,$(TARGET))
DEVICE = $(TARGET)
else
DEVICE ?= ps2usb
TARGET = $(DEVICE)
endif

BIN ?= $(DEVICE:=.bin)
OBJ = $(DEVICE:=.o)

OBJS = $(OBJ) usbkbd_descriptors.o usbkbd.o keys.o host_fingerprint.o $(DEVICE_OBJS) $(PLATFORM_OBJS)

BUILDDIR ?= $(DEVICE)/build

vpath %.c . $(DEVICE) arch/$(ARCH) ps2
vpath %.h . $(DEVICE) arch/$(ARCH) ps2

ifneq (,$(CUSTOM_DIR))
CUSTOM_FLAGS += -I$(CUSTOM_DIR)
MACROS_C = $(CUSTOM_DIR)/macros.c
LAYERS_C = $(CUSTOM_DIR)/layers.c
endif

DEVICE_FLAGS += $(CUSTOM_FLAGS)

ifeq ($(MODEL),vial)
VIAL_ENABLE = 1
endif

ifndef NO_LOCAL_MK
-include $(DEVICE)/local.mk
endif
include $(DEVICE)/$(DEVICE).mk

# Detect MODEL change and force recompilation
MODEL_FILE = $(BUILDDIR)/model
MODEL_OLD := $(wildcard $(MODEL_FILE))
ifneq ($(MODEL_OLD),)
MODEL_SAVED := $(file < $(MODEL_FILE))
ifneq ($(MODEL_SAVED),$(MODEL))
$(shell rm -f $(BUILDDIR)/*.o $(MODEL_FILE))
endif
endif

# Select macros and layers file (CUSTOM_DIR or device .mk may have set these)
ifndef MACROS_C
ifneq (,$(MODEL))
ifneq (,$(wildcard $(DEVICE)/macros_$(MODEL).c))
MACROS_C = $(DEVICE)/macros_$(MODEL).c
else
MACROS_C = $(DEVICE)/macros.c
endif
else
MACROS_C = $(DEVICE)/macros.c
endif
ifndef MACROS_C
MACROS_C = macros.c
endif
endif
ifndef LAYERS_C
ifneq (,$(wildcard $(DEVICE)/layers_$(MODEL).c))
LAYERS_C = $(DEVICE)/layers_$(MODEL).c
else
LAYERS_C = $(DEVICE)/layers.c
endif
ifndef LAYERS_C
LAYERS_C = layers.c
endif
endif

DEVICE_FLAGS += -DLAYERS_INCLUDE='<$(notdir $(LAYERS_C))>'
DEVICE_FLAGS += -DMACROS_INCLUDE='<$(notdir $(MACROS_C))>'

# Allow enabling one-shot keycodes via ENABLE_ONE_SHOT_KEYCODES=1 on command line
ifdef ENABLE_ONE_SHOT_KEYCODES
CONFIG_FLAGS += -DENABLE_ONE_SHOT_KEYCODES=$(ENABLE_ONE_SHOT_KEYCODES)
endif

ifndef ARCH
$(error ARCH not set. Make sure arch/$(ARCH)/$(ARCH)-common.mk gets included.)
endif

ifeq (0,$(ENABLE_HOST_FINGERPRINT))
	DEVICE_FLAGS += -DENABLE_HOST_FINGERPRINT=0
else
ifeq (1,$(ENABLE_HOST_FINGERPRINT))
	DEVICE_FLAGS += -DENABLE_HOST_FINGERPRINT=1
endif
endif

ifeq (0,$(ENABLE_PS2_DEVICE))
	DEVICE_FLAGS += -DENABLE_PS2_DEVICE=0
else
ifeq (1,$(ENABLE_PS2_DEVICE))
	DEVICE_FLAGS += -DENABLE_PS2_DEVICE=1 -Ips2
	DEVICE_OBJS += ps2_output.o usb2ps2_keys.o kk_ps2_device.o
endif
endif

all: $(BIN)
	@mkdir -p $(BUILDDIR) && echo '$(MODEL)' > $(MODEL_FILE)

# Object prerequisites for the link rule (added after all variables are resolved)
$(BIN): $(addprefix $(BUILDDIR)/, $(OBJS))

OBJECT_FILES = $(OBJS:%.o=$(BUILDDIR)/%.o)

$(BUILDDIR)/avrusb.o: avrusb.h usb_hardware.h usbkbd.h usbkbd_config.h usb.h usbkbd_descriptors.h generic_hid.h progmem.h aakbd.h local.mk
$(BUILDDIR)/usbkbd.o: usbkbd.h usb_hardware.h usbkbd_config.h usb.h usbkbd_descriptors.h usb_keys.h generic_hid.h aakbd.h progmem.h local.mk
$(BUILDDIR)/usbkbd_descriptors.o: usbkbd_descriptors.h usbkbd_config.h usb.h usb_keys.h generic_hid.h progmem.h aakbd.h local.mk
$(BUILDDIR)/keys.o: keys.h keycodes.h usbkbd.h usbkbd_config.h aakbd.h usb_keys.h layers.h macros.h progmem.h $(MACROS_C) $(LAYERS_C)
ifeq (1,$(VIAL_ENABLE))
$(BUILDDIR)/keys.o: vial/vial.h vial/dynamic_keymap.h
ifeq (1,$(ENABLE_PS2_DEVICE))
$(BUILDDIR)/keys.o: ps2/ps2_output.h
endif
endif
$(BUILDDIR)/host_fingerprint.o: host_fingerprint.h usbkbd_config.h
$(BUILDDIR)/ps2_output.o: ps2_output.c ps2_output.h usb2ps2_keys.h kk_ps2_device.h kk_ps2_avr.h $(COMMON_HEADERS)
$(BUILDDIR)/usb2ps2_keys.o: usb2ps2_keys.c usb2ps2_keys.h progmem.h usb_keys.h kk_ps2.h ps2_keys.h $(COMMON_HEADERS)
$(BUILDDIR)/kk_ps2_device.o: kk_ps2_device.c kk_ps2_device.h kk_ps2.h kk_ps2_avr.h usbkbd_config.h $(COMMON_HEADERS)
$(OBJECT_FILES): Makefile $(DEVICE)/$(DEVICE).mk $(wildcard local.mk) $(wildcard $(DEVICE)/local.mk)

$(BUILDDIR)/%.o: %.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Suppress some warnings here to allow more convenient layer init
$(BUILDDIR)/keys.o: keys.c
	$(CC) $(CFLAGS) -Wno-pedantic -Wno-override-init -Wno-type-limits -c $< -o $@
ifeq (1,$(VIAL_ENABLE))
$(BUILDDIR)/keys.o: $(DEVICE)/layers_vial.c $(DEVICE)/macros_vial.c
endif

local.mk:
	touch $@

$(DEVICE)/local.mk:
	touch $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(LAYERS_C): $(wildcard $(DEVICE)/template_layers_$(MODEL).c) $(wildcard $(DEVICE)/template_layers.c) template_layers.c
	@templates="$<"; [ -n "$$templates" ] && [ ! -r $@ ] && cp -v -f $$templates $@ || echo NOTICE: $< is newer than $@ - silence with: touch $@

$(MACROS_C): $(wildcard $(DEVICE)/template_macros_$(MODEL).c) $(wildcard $(DEVICE)/template_macros.c) template_macros.c
	@[ ! -r $@ ] && cp -v -f $< $@ || echo NOTICE: $< is newer than $@ - silence with: touch $@

# Auto-create MODEL-specific layers/macros from templates if missing.
# Tries $(DEVICE)/template_layers_$(MODEL).c first, then root template_layers.c.
$(DEVICE)/layers_%.c: $(DEVICE)/template_layers_%.c
	@[ ! -r $@ ] && cp -v -f $< $@ || true

$(DEVICE)/macros_%.c: $(DEVICE)/template_macros_%.c
	@[ ! -r $@ ] && cp -v -f $< $@ || true

layers_%.c: template_layers_%.c
	@[ ! -r $@ ] && cp -v -f $< $@ || true

macros_%.c: template_macros_%.c
	@[ ! -r $@ ] && cp -v -f $< $@ || true

reset:
	$(SUDO) dfu-util -e

dfu: $(DFU_TARGET)

clean:
	rm -f *.o *.gcda *.gcno
	@[ -d ./$(BUILDDIR) ] && rm -f ./$(BUILDDIR)/*.o ./$(BUILDDIR)/*.elf ./$(BUILDDIR)/*.map ./$(BUILDDIR)/*.c ./$(BUILDDIR)/*.h ./$(BUILDDIR)/*.i $(BUILDDIR)/model || true
	@[ -e $(BUILDDIR) ] && rmdir $(BUILDDIR) || true
	@[ -d ./release/build ] && rm -rf ./release/build || true

release:
	@./build_release

upload_release:
	@./upload_release $(TAG)

distclean: | clean
	@make -C vial clean
	@make -C ps2 clean
	rm -f *.hex *.bin
	rm -rf release vial/*.bin ps2/*.bin
	find . -depth -name 'build' -type d -exec rm -rf '{}' ';'
	find . -depth -name '__pycache__' -type d -exec rm -rf '{}' ';'
	@[ -e $(MACROS_C) -o -e $(LAYERS_C) ] && echo NOT deleting $(MACROS_C) and $(LAYERS_C) files! || true

backup:
	cp $(MACROS_C) $(MACROS_C)~
	cp $(LAYERS_C) $(LAYERS_C)~

.PHONY: all clean distclean release upload_release backup dfu reset
