# arm-common.mk: Shared build rules for ARM MCU keyboards.
#
# Include this from your MCU-family .mk (e.g., stm32-common.mk)
# after setting MCU-family-specific variables.

ARCH = arm
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

ARM_MCU_FLAGS = -mcpu=$(ARM_MCU) -mthumb
HARD_FLOAT ?= 0
ifeq (1,$(HARD_FLOAT))
ARM_MCU_FLAGS += -mfloat-abi=hard
else
ARM_MCU_FLAGS += -mfloat-abi=soft
endif

TINYUSB_DIR = lib/tinyusb/src

CC_FLAGS += -I$(TINYUSB_DIR) -Iarch/arm
TINYUSB_OBJS = tinyusb.o tusb.o hid_device.o dfu_rt_device.o \
               $(MCU_DCD_OBJ) fsdev_common.o tusb_fifo.o

vpath %.c arch/arm
vpath %.h arch/arm
vpath %.c $(TINYUSB_DIR)
vpath %.c $(TINYUSB_DIR)/device
vpath %.c $(TINYUSB_DIR)/class/hid
vpath %.c $(TINYUSB_DIR)/class/dfu
vpath %.c $(TINYUSB_DIR)/common
vpath %.c $(TINYUSB_DIR)/$(MCU_TUSB_PORT)

# usbd.c from tinyusb needs to be patched to support host fingerprint
ifeq (0,$(ENABLE_HOST_FINGERPRINT))
	TINYUSB_OBJS += usbd.o
else
	TINYUSB_OBJS += usbd_patched.o
endif

# No .hex output for ARM by default
HEX ?=

# ARM hex conversion from the ELF (produced by the device link rule)
TARGET_ELF ?= $(BUILDDIR)/$(DEVICE).elf
ifneq (,$(HEX))
all: $(HEX)
$(HEX): $(TARGET_ELF)
	$(OBJCOPY) -O ihex $< $@
endif

PLATFORM_OBJS = syscalls.o $(MCU_OBJS) $(TINYUSB_OBJS)

DFU_TARGET = dfuarm
DFU_VID ?= 0483
DFU_PID ?= DF11
DFU_ARGS ?= -d $(DFU_VID):$(DFU_PID) -a 0 -s 0x08000000:leave
DFU_SUFFIX ?= dfu-suffix
DFU_SUFFIX_ARGS = -v $(DFU_VID) -p $(DFU_PID)

dfuarm: $(BIN)
	$(SUDO) dfu-util -e && sleep 1 || true
	$(SUDO) dfu-util -w $(DFU_ARGS) -D $<

upload: dfuarm

.PHONY: dfuarm upload

.ccls: Makefile local.mk $(DEVICE)/local.mk
	@echo $(CC) >$@
	@echo $(CFLAGS) | awk '{ gsub(/["][^"]*["]/, "\"!SKIP!\""); for (i=1; i<=NF; i++) { if (!($$i ~ /!SKIP!/)) print $$i } }' >>$@
	#@[ -d /usr/arm-none-eabi/include ] && echo -I/usr/arm-none-eabi/include >>$@ || true
	@echo | $(CC) $(CFLAGS) -E -Wp,-v - 2>&1 | awk '/#include .* search starts here:/ { output=1; next } !output { next } /^End/ || /^#/ { output=0 } output && $$1 ~ /^\// { sub(/^[ ]*/, ""); print "-I" $$0 }' >>$@
	@cat $@

.clangd: Makefile local.mk $(DEVICE)/local.mk
	@printf 'CompileFlags:\n  Compiler: %s\n  Add:\n' "$(CC)" >$@
	@echo '    - -xc' >>$@
	@echo $(CFLAGS) | awk '{ gsub(/["][^"]*["]/, "\"!SKIP!\""); for (i=1; i<=NF; i++) { if (!($$i ~ /!SKIP!/)) printf "    - %s\n", $$i } }' >>$@
	#@[ -d /usr/arm-none-eabi/include ] && printf '    - -I/usr/arm-none-eabi/include\n' >>$@ || true
	@echo | $(CC) $(CFLAGS) -E -Wp,-v - 2>&1 | awk '/#include .* search starts here:/ { output=1; next } !output { next } /^End/ || /^#/ { output=0 } output && $$1 ~ /^\// { sub(/^[ ]*/, ""); printf "    - -I%s\n", $$0 }' >>$@
	@cat $@

$(TINYUSB_DIR)/tusb.h:
	git submodule update --init

# ARM link rule (prerequisites added by root rule after device .mk is included)
TARGET_ELF ?= $(BUILDDIR)/$(DEVICE).elf

$(BIN): | $(BUILDDIR)
	$(CC) $(LDFLAGS) -Wl,-Map=$(BUILDDIR)/$(DEVICE).map -o $(TARGET_ELF) $^ $(LDLIBS)
	$(SIZE) $(TARGET_ELF)
	$(OBJCOPY) -O binary $(TARGET_ELF) $@
	@[ -n "$(DFU_SUFFIX)" ] && which "$(DFU_SUFFIX)" >/dev/null 2>&1 && \
		$(DFU_SUFFIX) $(DFU_SUFFIX_ARGS) -a $@ || true
	@chmod a-x $@

USBD_PATCHED = $(BUILDDIR)/usbd_patched.c
$(USBD_PATCHED): $(TINYUSB_DIR)/device/usbd.c | $(BUILDDIR)
	@echo Patching $< to $@
	@sed -e '0,/^#include/s/^#include/#include "host_fingerprint.h"\n#include/' \
	    -e '/uint8_t const\* desc_str = (uint8_t const\*) tud_descriptor_string_cb(desc_index, p_request->wIndex);/i\#if ENABLE_HOST_FINGERPRINT\n    host_fingerprint_observe(p_request->wLength);\n#endif' $< > $@
	@grep -q '^#if ENABLE_HOST_FINGERPRINT' $@ || { echo "Error: patch not applied" >&2; exit 1; }

$(BUILDDIR)/tinyusb.o: tinyusb.c tinyusb.h usb_hardware.h usbkbd.h usbkbd_config.h usbkbd_descriptors.h aakbd.h generic_hid.h $(COMMON_HEADERS) $(TINYUSB_DIR)/tusb.h
$(BUILDDIR)/usbd_patched.o: $(USBD_PATCHED) host_fingerprint.h $(TINYUSB_DIR)/tusb.h $(COMMON_HEADERS)
$(BUILDDIR)/usbd.o: usbd.c $(TINYUSB_DIR)/tusb.h
$(BUILDDIR)/tusb.o $(BUILDDIR)/usbd.o $(BUILDDIR)/hid_device.o $(BUILDDIR)/dfu_rt_device.o $(BUILDDIR)/fsdev_common.o $(BUILDDIR)/tusb_fifo.o $(BUILDDIR)/dcd_stm32_fsdev.o: $(TINYUSB_DIR)/tusb.h
