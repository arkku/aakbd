KEYBOARD_NAME = "keyboards/xwhatsit/brand_new_model_f/f62/wcass/wcass"
PRODUCT ?= "F62 Keyboard"
DEBOUNCE ?= 5

vpath modelf.c modelf77

include modelf77/modelf-common.mk

DEVICE_FLAGS += -DMODELF=62
