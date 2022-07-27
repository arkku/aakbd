# Makefile
# Copyright (c) 2021 Kimmo Kulovesi, https://arkku.dev/
###############################################################################
# Put your local settings in "local.mk", it is ignored by Git.
-include local.mk

CC=avr-gcc
CFLAGS=-Wall -std=c11 -pedantic -Wextra -Wno-unused-parameter -Os -I$(DEVICE) -I. $(AVR_FLAGS) $(DEVICE_FLAGS) $(CONFIG_FLAGS)
LDFLAGS=-Os $(AVR_FLAGS)
AR=avr-ar
ARFLAGS=rcs
OBJCOPY=avr-objcopy
AVRDUDE=avrdude

AVR_FLAGS=-mmcu=$(MCU) -DF_CPU=$(F_CPU)

DEVICE ?= ps2usb

HEX = $(DEVICE:=.hex)
BIN = $(HEX:.hex=.bin)
OBJ = $(BIN:.bin=.o)

OBJS = $(OBJ) usbkbd_descriptors.o usbkbd.o keys.o $(DEVICE_OBJS)

vpath %.c . $(DEVICE)
vpath %.h . $(DEVICE)

BUILDDIR ?= $(DEVICE)/build

all: $(HEX)

-include $(DEVICE)/local.mk
include $(DEVICE)/$(DEVICE).mk

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

### AVR MCU ###################################################################

MCU ?= atmega32u4
F_CPU ?= 16000000UL

# LFUSE configures the clock speed; it is probably correct out of the box if
# you use a ready-made board. If not, `make fuses LFUSE=CE` for a 16 MHz
# crystal oscillator.
#LFUSE ?= CE
HFUSE ?= D0
EFUSE ?= FB
#EFUSE ?= F4 # HWBEN

#### BURNER ###################################################################
# Specify the burner on the command-line if you wish, e.g.,
#	make burn BURNER=avrisp2 PORT=/dev/ttyUSB0 BPS=115200

# Burner device
BURNER ?= dragon_isp
# Burner port
#PORT ?= /dev/ttyUSB0
# Burner speed
#BPS ?= 115200

# Protocol for the bootloader, e.g., make upload PORT=/dev/cu.usbmodem1401
UPLOAD_PROTOCOL ?= avr109
###############################################################################

OBJECT_FILES = $(OBJS:%.o=$(BUILDDIR)/%.o)

$(BUILDDIR)/usbkbd.o: usbkbd.h usbkbd_config.h usb.h usbkbd_descriptors.h usb_keys.h avrusb.h generic_hid.h main.h progmem.h local.mk
$(BUILDDIR)/usbkbd_descriptors.o: usbkbd_descriptors.h usbkbd_config.h usb.h usb_keys.h generic_hid.h progmem.h local.mk
$(BUILDDIR)/keys.o: keys.h keycodes.h usbkbd.h usbkbd_config.h main.h usb_keys.h layers.h macros.h progmem.h $(MACROS_C) $(LAYERS_C)
$(OBJECT_FILES): Makefile $(DEVICE)/$(DEVICE).mk $(wildcard local.mk) $(wildcard $(DEVICE)/local.mk)

$(HEX): $(BIN)

$(BUILDDIR)/%.o: %.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Suppress some warnings here to allow more convenient layer init
$(BUILDDIR)/keys.o: keys.c
	$(CC) $(CFLAGS) -Wno-pedantic -Wno-override-init -Wno-type-limits -c $< -o $@

$(BIN): $(OBJECT_FILES)
	$(CC) $(LDFLAGS) -s -o $@ $+
	@chmod a-x $@
	@avr-size $@

$(HEX): $(BIN)
	@echo $(OBJECT_FILES)
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

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

burn: $(HEX)
	$(AVRDUDE) -c $(BURNER) $(if $(PORT),-P $(PORT) ,)$(if $(BPS),-b $(BPS) ,)-p $(MCU) -U flash:w:$< -v

upload: $(HEX)
	stty -f $(if $(PORT),$(PORT),/dev/ttyUSB0) 1200
	$(AVRDUDE) -c $(UPLOAD_PROTOCOL) $(if $(PORT),-P $(PORT),-P /dev/ttyUSB0) $(if $(BPS),-b $(BPS),) -p $(MCU) -U flash:w:$< -v

fuses:
	$(AVRDUDE) -c $(BURNER) $(if $(PORT),-P $(PORT) ,)$(if $(BPS),-b (BPS) ,)-p $(MCU) -U efuse:w:0x$(EFUSE):m -U hfuse:w:0x$(HFUSE):m $(if $(LFUSE),-U lfuse:w:0x$(LFUSE):m,)

unlock:
	$(AVRDUDE) -c $(BURNER) $(if $(PORT),-P $(PORT) ,)$(if $(BPS),-b $(BPS) ,)-p $(MCU) -U lock:w:0x3F:m -v

lock:
	$(AVRDUDE) -c $(BURNER) $(if $(PORT),-P $(PORT) ,)$(if $(BPS),-b $(BPS) ,)-p $(MCU) -U lock:w:0x0F:m -v

.ccls: Makefile local.mk $(DEVICE)/local.mk
	@echo $(CC) >$@
	@echo --target=avr >>$@
	@echo $(CFLAGS) | awk '{ gsub(/["][^"]*["]/, "\"!SKIP!\""); for (i=1; i<=NF; i++) { if (!($$i ~ /!SKIP!/)) print $$i } }' >>$@
	@echo -nostdinc >>$@
	@[ -d /usr/lib/avr/include ] && echo -I/usr/lib/avr/include >>$@ || true
	@[ -d /usr/local/avr/include ] && echo -I/usr/local/avr/include >>$@ || true
	@echo | $(CC) $(CFLAGS) -E -Wp,-v - 2>&1 | awk '/#include .* search starts here:/ { output=1; next } !output { next } /^End/ || /^#/ { output=0 } output && $$1 ~ /^\// { sub(/^[ ]*/, ""); print "-I" $$0 }' >>$@
	@echo -Wno-attributes >>$@
	@echo -Wno-gnu-zero-variadic-macro-arguments >>$@
	@cat $@

clean:
	rm -f *.o
	@[ -d ./$(BUILDDIR) ] && rm -f ./$(BUILDDIR)/*.o || true
	@[ -e $(BUILDDIR) ] && rmdir $(BUILDDIR) || true

distclean: | clean
	rm -f *.hex *.bin .ccls
	find . -name '*.o' -type f -delete
	find . -name 'build' -type d -d -exec rmdir '{}' ';'
	@[ -e $(MACROS_C) -o -e $(LAYERS_C) ] && echo NOT deleting $(MACROS_C) and $(LAYERS_C) files! || true

backup:
	cp $(MACROS_C) $(MACROS_C)~
	cp $(LAYERS_C) $(LAYERS_C)~

.PHONY: all clean distclean burn fuses upload lock unlock bootloader backup
