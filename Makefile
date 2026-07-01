# Makefile
# Copyright (c) 2021-2026 Kimmo Kulovesi, https://arkku.dev/
###############################################################################
# Put your local settings in "local.mk", it is ignored by Git.
-include local.mk

OPTIMIZATION ?= s
CFLAGS=-O$(OPTIMIZATION) -Wall -std=gnu11 -Wextra -Wno-unused-parameter $(CUSTOM_FLAGS) $(CC_FLAGS) $(DEVICE_FLAGS) $(CONFIG_FLAGS)
LDFLAGS=-O$(OPTIMIZATION) $(LD_FLAGS)
AR ?= avr-ar
ARFLAGS=rcs

CC_FLAGS=-I$(DEVICE) -I. -Iarch/$(ARCH)
LD_FLAGS=

MODEL ?=

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

MODEL_FILE = $(BUILDDIR)/model
MODEL_OLD := $(wildcard $(MODEL_FILE))
ifneq ($(MODEL_OLD),)
MODEL_SAVED := $(file < $(MODEL_FILE))
ifneq ($(MODEL_SAVED),$(MODEL))
# Delete *.o on MODEL change, as there may be different config from that
$(shell rm -f $(BUILDDIR)/*.o $(MODEL_FILE))
endif
endif

vpath %.c . $(DEVICE) arch/$(ARCH) ps2
vpath %.h . $(DEVICE) arch/$(ARCH) ps2

ifneq (,$(CUSTOM_DIR))
CUSTOM_FLAGS += -I$(CUSTOM_DIR)
MACROS_C = $(CUSTOM_DIR)/macros.c
LAYERS_C = $(CUSTOM_DIR)/layers.c
else # ^ CUSTOM_DIR
ifneq (,$(MODEL))
ifneq (,$(wildcard $(DEVICE)/macros_$(MODEL).c))
MACROS_C = $(DEVICE)/macros_$(MODEL).c
DEVICE_FLAGS += -DMACROS_INCLUDE='<macros_$(MODEL).c>'
else
ifneq (,$(wildcard $(DEVICE)/macros.c))
MACROS_C = $(DEVICE)/macros.c
else
MACROS_C = macros.c
endif
endif
ifneq (,$(wildcard $(DEVICE)/layers_$(MODEL).c))
LAYERS_C = $(DEVICE)/layers_$(MODEL).c
DEVICE_FLAGS += -DLAYERS_INCLUDE='<layers_$(MODEL).c>'
else
ifneq (,$(wildcard $(DEVICE)/layers.c))
LAYERS_C = $(DEVICE)/layers.c
else
LAYERS_C = layers.c
endif
endif
else # ^ MODEL
ifneq (,$(wildcard $(DEVICE)/macros.c))
MACROS_C = $(DEVICE)/macros.c
else
MACROS_C = macros.c
endif
ifneq (,$(wildcard $(DEVICE)/layers.c))
LAYERS_C = $(DEVICE)/layers.c
else
LAYERS_C = layers.c
endif
endif
endif

-include $(DEVICE)/local.mk
include $(DEVICE)/$(DEVICE).mk

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
$(BUILDDIR)/host_fingerprint.o: host_fingerprint.h usbkbd_config.h
$(BUILDDIR)/ps2_output.o: ps2/ps2_output.c ps2/ps2_output.h ps2/usb2ps2_keys.h ps2/kk_ps2_device.h ps2/kk_ps2_avr.h $(COMMON_HEADERS)
$(BUILDDIR)/usb2ps2_keys.o: ps2/usb2ps2_keys.c ps2/usb2ps2_keys.h progmem.h usb_keys.h ps2/kk_ps2.h ps2/ps2_keys.h $(COMMON_HEADERS)
$(BUILDDIR)/kk_ps2_device.o: ps2/kk_ps2_device.c ps2/kk_ps2_device.h ps2/kk_ps2.h ps2/kk_ps2_avr.h usbkbd_config.h $(COMMON_HEADERS)
$(BUILDDIR)/kk_ps2_host.o: ps2/kk_ps2_host.c ps2/kk_ps2_host.h ps2/kk_ps2.h ps2/kk_ps2_avr.h usbkbd_config.h $(COMMON_HEADERS)
$(OBJECT_FILES): Makefile $(DEVICE)/$(DEVICE).mk $(wildcard local.mk) $(wildcard $(DEVICE)/local.mk)

$(BUILDDIR)/%.o: %.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Suppress some warnings here to allow more convenient layer init
$(BUILDDIR)/keys.o: keys.c
	$(CC) $(CFLAGS) -Wno-pedantic -Wno-override-init -Wno-type-limits -c $< -o $@

local.mk:
	touch $@

$(DEVICE)/local.mk:
	touch $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(LAYERS_C): $(wildcard $(DEVICE)/template_layers.c) template_layers.c
	@[ ! -r $@ ] && cp -v -f $< $@ || echo NOTICE: $< is newer than $@ - silence with: touch $@

$(MACROS_C): $(wildcard $(DEVICE)/template_macros.c) template_macros.c
	@[ ! -r $@ ] && cp -v -f $< $@ || echo NOTICE: $< is newer than $@ - silence with: touch $@

reset:
	$(SUDO) dfu-util -e

dfu: $(DFU_TARGET)

clean:
	rm -f *.o *.gcda *.gcno
	@[ -d ./$(BUILDDIR) ] && rm -f ./$(BUILDDIR)/*.o ./$(BUILDDIR)/*.elf ./$(BUILDDIR)/*.map ./$(BUILDDIR)/*.c ./$(BUILDDIR)/*.h || true
	@[ -e $(BUILDDIR) ] && rmdir $(BUILDDIR) || true
	@[ -d ./release/build ] && rm -rf ./release/build || true

release:
	@./build_release

upload_release:
	@./upload_release $(TAG)

distclean: | clean
	rm -f *.hex *.bin
	find . -depth -name 'build' -type d -exec rm -rf '{}' ';'
	@[ -e $(MACROS_C) -o -e $(LAYERS_C) ] && echo NOT deleting $(MACROS_C) and $(LAYERS_C) files! || true

backup:
	cp $(MACROS_C) $(MACROS_C)~
	cp $(LAYERS_C) $(LAYERS_C)~

.PHONY: all clean distclean release upload_release backup dfu reset
