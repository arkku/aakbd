# Build flags and dependencies for the ARM QMK platform port.

DEVICE_FLAGS += -ffunction-sections -fdata-sections -fshort-wchar

$(BUILDDIR)/qmk_port.o: qmk_port.c qmk_port.h $(COMMON_HEADERS)
$(BUILDDIR)/platform.o: $(COMMON_HEADERS)
$(BUILDDIR)/timer.o: $(COMMON_HEADERS)
$(BUILDDIR)/suspend.o: suspend.h action.h keyboard.h $(COMMON_HEADERS)
$(BUILDDIR)/dfu.o: bootloader.h $(COMMON_HEADERS)
$(BUILDDIR)/spi.o: spi.h $(COMMON_HEADERS)
