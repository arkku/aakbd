XWHATSIT_DIR = xwhatsit_core
KEYBOARD_NAME = xwhatsit

vpath %.c $(XWHATSIT_DIR)
vpath %.h $(XWHATSIT_DIR)

DEVICE_FLAGS += -I$(XWHATSIT_DIR) -DXWHATSIT=1

COMMON_HEADERS += post_config.h xwhatsit_port.h

$(BUILDDIR)/matrix.o: matrix_manipulate.h $(COMMON_HEADERS)
$(BUILDDIR)/util_comm.o: util_comm.h matrix_manipulate.h $(COMMON_HEADERS)
