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

# Default goal must be the first target in the Makefile
all:

ifneq (,$(TARGET))
DEVICE = $(TARGET)
else
DEVICE ?= ps2usb
TARGET = $(DEVICE)
endif

BIN ?= $(DEVICE:=.bin)
HEX ?= $(BIN:.bin=.hex)
OBJ = $(DEVICE:=.o)

OBJS = $(OBJ) usbkbd_descriptors.o usbkbd.o keys.o $(DEVICE_OBJS) $(PLATFORM_OBJS)

BUILDDIR ?= $(DEVICE)/build

vpath %.c . $(DEVICE) arch/$(ARCH)
vpath %.h . $(DEVICE) arch/$(ARCH)

ifneq (,$(CUSTOM_DIR))
CUSTOM_FLAGS += -I$(CUSTOM_DIR)
MACROS_C = $(CUSTOM_DIR)/macros.c
LAYERS_C = $(CUSTOM_DIR)/layers.c
else # ^ CUSTOM_DIR
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

-include $(DEVICE)/local.mk
include $(DEVICE)/$(DEVICE).mk

ifndef ARCH
$(error ARCH not set. Make sure arch/$(ARCH)/$(ARCH)-common.mk gets included.)
endif

ifneq (,$(HEX))
all: $(HEX)
else
all: $(BIN)
endif

OBJECT_FILES = $(OBJS:%.o=$(BUILDDIR)/%.o)

$(BUILDDIR)/avrusb.o: avrusb.h usb_hardware.h usbkbd.h usbkbd_config.h usb.h usbkbd_descriptors.h generic_hid.h progmem.h aakbd.h local.mk
$(BUILDDIR)/usbkbd.o: usbkbd.h usb_hardware.h usbkbd_config.h usb.h usbkbd_descriptors.h usb_keys.h generic_hid.h aakbd.h progmem.h local.mk
$(BUILDDIR)/usbkbd_descriptors.o: usbkbd_descriptors.h usbkbd_config.h usb.h usb_keys.h generic_hid.h progmem.h aakbd.h local.mk
$(BUILDDIR)/keys.o: keys.h keycodes.h usbkbd.h usbkbd_config.h aakbd.h usb_keys.h layers.h macros.h progmem.h $(MACROS_C) $(LAYERS_C)
$(OBJECT_FILES): Makefile $(DEVICE)/$(DEVICE).mk $(wildcard local.mk) $(wildcard $(DEVICE)/local.mk)

$(BUILDDIR)/%.o: %.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Suppress some warnings here to allow more convenient layer init
$(BUILDDIR)/keys.o: keys.c
	$(CC) $(CFLAGS) -Wno-pedantic -Wno-override-init -Wno-type-limits -c $< -o $@

# AVR: link to .bin
ifneq (,$(HEX))
$(BIN): $(OBJECT_FILES)
	@echo $(CC) $(LDFLAGS) -s -o $@ ...
	@$(CC) $(LDFLAGS) -s -o $@ $+
	@chmod a-x $@
	@$(SIZE) $@

$(HEX): $(BIN)
	$(OBJCOPY) -j .text -j .data -O ihex $< $@
endif

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
upload: dfu

clean:
	rm -f *.o
	@[ -d ./$(BUILDDIR) ] && rm -f ./$(BUILDDIR)/*.o ./$(BUILDDIR)/*.elf ./$(BUILDDIR)/*.map || true
	@[ -e $(BUILDDIR) ] && rmdir $(BUILDDIR) || true
	@[ -d ./release/build ] && rm -rf ./release/build || true

release:
	@./build_release

upload_release:
	@./upload_release $(TAG)

distclean: | clean
	rm -f *.hex *.bin
	find . -name '*.elf' -type f -delete
	find . -name '*.map' -type f -delete
	find . -name '*.o' -type f -delete
	find . -depth -name 'build' -type d -exec rmdir '{}' ';'
	@[ -e $(MACROS_C) -o -e $(LAYERS_C) ] && echo NOT deleting $(MACROS_C) and $(LAYERS_C) files! || true

backup:
	cp $(MACROS_C) $(MACROS_C)~
	cp $(LAYERS_C) $(LAYERS_C)~

.PHONY: all clean distclean release upload_release backup dfu reset
