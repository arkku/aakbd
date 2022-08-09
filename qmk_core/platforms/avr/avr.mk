DEVICE_FLAGS += -funsigned-char
DEVICE_FLAGS += -funsigned-bitfields
DEVICE_FLAGS += -ffunction-sections
DEVICE_FLAGS += -fdata-sections
DEVICE_FLAGS += -fpack-struct
DEVICE_FLAGS += -fshort-enums
DEVICE_FLAGS += -mcall-prologues

vpath %.c $(PLATFORM_DIR) $(PLATFORM_DIR)/bootloaders
vpath %.h $(PLATFORM_DIR)

$(BUILDDIR)/avr.o: platform_deps.h qmk_port.h
$(BUILDDIR)/platform.o: platform_deps.h
$(BUILDDIR)/timer.o: timer.h timer_avr.h
$(BUILDDIR)/suspend.o: suspend.h timer.h action.h keyboard.h
$(BUILDDIR)/eeconfig.o: eeconfig.h $(COMMON_HEADERS)
$(BUILDDIR)/$(BOOTLOADER_TYPE).o: bootloader.h platform_deps.h $(COMMON_HEADERS)
