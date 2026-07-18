# Vial support Make fragment — included from qmk_core/qmk_port.mk when VIAL_ENABLE=1
#
# This adds Vial protocol handler, dynamic keymap, keycode translation,
# MAGIC keycodes, and the keyboard definition to the build.

VIAL_INSECURE ?= 1

DEVICE_FLAGS += -DVIAL_ENABLE=1
DEVICE_FLAGS += -DVIAL_INSECURE=$(VIAL_INSECURE)

CC_FLAGS += -Ivial
vpath %.c vial
vpath %.h vial
GENERIC_HID_FEATURE_SIZE = 32

# Setting MODEL changes from layers.c to layers_$(MODEL).c, and similarly for
# macros.c - with vial we use this mechanism to switch the layers_vial.c
MODEL ?= vial

QMK_CORE_OBJS += via_handler.o vial.o vial_keys.o dynamic_keymap.o qmk_translate.o vial_magic.o keyboard_definition.o
$(BUILDDIR)/via_handler.o: vial/via_handler.c generic_hid.h via.h vial.h dynamic_keymap.h $(COMMON_HEADERS)
$(BUILDDIR)/vial.o: vial/vial.c vial.h vial_keys.h dynamic_keymap.h qmk_translate.h qmk_keycodes.h vial_magic.h progmem.h keys.h usbkbd.h aakbd.h dynamic_storage.h vial_config.h $(COMMON_HEADERS)
$(BUILDDIR)/vial_keys.o: vial/vial_keys.c vial_keys.h vial.h dynamic_keymap.h qmk_translate.h qmk_keycodes.h vial_magic.h progmem.h keys.h usbkbd.h aakbd.h dynamic_storage.h vial_config.h $(COMMON_HEADERS)
$(BUILDDIR)/dynamic_keymap.o: vial/dynamic_keymap.c dynamic_keymap.h qmk_translate.h qmk_keycodes.h progmem.h $(COMMON_HEADERS)
$(BUILDDIR)/qmk_translate.o: vial/qmk_translate.c qmk_translate.h qmk_keycodes.h keycodes.h vial_magic.h progmem.h $(COMMON_HEADERS)
$(BUILDDIR)/vial_magic.o: vial/vial_magic.c vial_magic.h dynamic_keymap.h $(COMMON_HEADERS)
$(BUILDDIR)/keyboard_definition.o: $(BUILDDIR)/vial_keyboard_definition.c
	$(CC) $(CFLAGS) -c $< -o $@

# Generate the keyboard definition source from vial.json
VIAL_DEFINITION_SRC = $(BUILDDIR)/vial_keyboard_definition.c

# User keycodes for the keyboard definition are generated from keys.c
# (which includes the actual layers and macros via LAYERS_INCLUDE/MACROS_INCLUDE)
VIAL_DEFINITION_I = $(BUILDDIR)/macros.i

$(VIAL_DEFINITION_I): keys.c $(BUILDDIR)/keys.o
	$(CC) $(CFLAGS) -E -P $< | awk '/enum macro/, /}/' > $@

VIAL_JSON_FILE ?= $(DEVICE)/vial.json
$(VIAL_DEFINITION_SRC): $(VIAL_JSON_FILE) vial/generate_definition.py $(VIAL_DEFINITION_I)
	mkdir -p $(BUILDDIR)
	python3 vial/generate_definition.py $< $@ \
		--vendor-id=$(VENDOR_ID) --product-id=$(PRODUCT_ID) \
		--macros-cpp=$(VIAL_DEFINITION_I)
