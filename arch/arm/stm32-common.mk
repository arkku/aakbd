# stm32-common.mk: Shared build rules for STM32 MCU keyboards.
#
# Include this from your device .mk. Set MCU_FAMILY before including.
# See gmmkpro1.mk for an example. This file in turn includes
# arch/arm/arm-common.mk for generic ARM build infrastructure.

# Per-family settings (override in device .mk or local.mk)
ifeq ($(MCU_FAMILY),stm32f3)
ARM_MCU          ?= cortex-m4
DEVICE_ARCH_DIR  ?= arch/arm/stm32f3
MCU_LD_FILE      ?= $(DEVICE_ARCH_DIR)/linker_stm32f303xc.ld
MCU_DEFINE       ?= -DSTM32F303xC -DMCU_SERIES_STM32F3
MCU_TUSB_DEFINE  ?= -DCFG_TUSB_MCU=OPT_MCU_STM32F3
CMSIS_MCU_DIR    ?= lib/STM32F3xx/Include
MCU_OBJS         ?= startup_stm32f303xc.o system_stm32f3xx.o tinyusb_hardware.o
MCU_DCD_OBJ      ?= dcd_stm32_fsdev.o
MCU_TUSB_PORT    ?= portable/st/stm32_fsdev
MCU_FLASH_SIZE_KB := $(shell sed -n 's/.*FLASH.*LENGTH = \([0-9]*\)K.*/\1/p' $(MCU_LD_FILE))
else
$(error Unsupported MCU_FAMILY "$(MCU_FAMILY)".)
endif

ifeq (,$(MCU_FLASH_SIZE_KB))
$(error Could not determine flash size from linker script $(MCU_LD_FILE))
endif

# Platform flags
CC_FLAGS = -I$(DEVICE) -I. $(ARM_MCU_FLAGS) -Ilib/CMSIS/Core/Include -I$(CMSIS_MCU_DIR) -I$(DEVICE_ARCH_DIR)
LD_FLAGS = $(ARM_MCU_FLAGS) -Wl,--gc-sections -Wl,-e,Reset_Handler -T$(MCU_LD_FILE) -Wl,--no-warn-rwx-segments -Wl,--no-wchar-size-warning -nostartfiles --specs=nano.specs

# MCU-specific defines
DEVICE_FLAGS += $(MCU_DEFINE) $(MCU_TUSB_DEFINE)

# VPATH for MCU architecture files
vpath %.c $(DEVICE_ARCH_DIR)
vpath %.c $(CMSIS_MCU_DIR)/Source

include arch/arm/arm-common.mk

# Wear-leveling EEPROM objects for ARM (flash-backed, no hardware EEPROM)
DEVICE_FLAGS += -DEEPROM_DRIVER -DEEPROM_WEAR_LEVELING -DEEPROM_MAX=1023
DEVICE_FLAGS += -DWEAR_LEVELING_EMBEDDED_FLASH
DEVICE_FLAGS += -DWEAR_LEVELING_BACKING_SIZE=2048 -DWEAR_LEVELING_LOGICAL_SIZE=1024
DEVICE_FLAGS += -DBACKING_STORE_WRITE_SIZE=2
CC_FLAGS += -Iqmk_core/wear_leveling
MCU_OBJS += wear_leveling_stm32f3.o
DEVICE_OBJS += eeprom_driver.o eeprom_wear_leveling.o wear_leveling.o fnv64.o
vpath %.c qmk_core
vpath %.c qmk_core/wear_leveling

# Explicit dependency rules for MCU objects
$(BUILDDIR)/startup_stm32f303xc.o: $(DEVICE_ARCH_DIR)/startup_stm32f303xc.c $(DEVICE_ARCH_DIR)/bootloader_magic.h
$(BUILDDIR)/tinyusb_hardware.o: $(DEVICE_ARCH_DIR)/tinyusb_hardware.c $(TINYUSB_DIR)/tusb.h
