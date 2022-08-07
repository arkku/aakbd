KEYBOARD_NAME = "keyboards/brand_new_model_f/f77/wcass/wcass"
PRODUCT ?= "F77 Keyboard"
DEBOUNCE ?= 5

include $(DEVICE)/modelf-common.mk

DEVICE_FLAGS += -DMODELF=77
