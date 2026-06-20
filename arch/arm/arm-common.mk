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
TINYUSB_OBJS = tinyusb.o tusb.o usbd.o hid_device.o dfu_rt_device.o \
               $(MCU_DCD_OBJ) fsdev_common.o tusb_fifo.o

vpath %.c arch/arm
vpath %.h arch/arm
vpath %.c $(TINYUSB_DIR)
vpath %.c $(TINYUSB_DIR)/device
vpath %.c $(TINYUSB_DIR)/class/hid
vpath %.c $(TINYUSB_DIR)/class/dfu
vpath %.c $(TINYUSB_DIR)/common
vpath %.c $(TINYUSB_DIR)/$(MCU_TUSB_PORT)

HEX =

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

.PHONY: dfuarm

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

$(BUILDDIR)/tinyusb.o: tinyusb.c tinyusb.h usb_hardware.h usbkbd.h usbkbd_config.h usbkbd_descriptors.h aakbd.h generic_hid.h $(COMMON_HEADERS) $(TINYUSB_DIR)/tusb.h

$(BUILDDIR)/tusb.o $(BUILDDIR)/usbd.o $(BUILDDIR)/hid_device.o \
$(BUILDDIR)/dfu_rt_device.o $(BUILDDIR)/fsdev_common.o $(BUILDDIR)/tusb_fifo.o \
$(BUILDDIR)/dcd_stm32_fsdev.o: $(TINYUSB_DIR)/tusb.h
