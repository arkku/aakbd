KEYBOARD_NAME = "keyboards/xwhatsit/brand_new_model_f/f77/wcass/wcass"
PRODUCT ?= "F77 Keyboard"

include modelf77/modelf-common.mk

DEVICE_FLAGS += -DMODELF=77
