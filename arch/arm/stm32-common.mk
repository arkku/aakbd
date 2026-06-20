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
else
$(error Unsupported MCU_FAMILY "$(MCU_FAMILY)".)
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

# Explicit dependency rules for MCU objects
$(BUILDDIR)/startup_stm32f303xc.o: $(DEVICE_ARCH_DIR)/startup_stm32f303xc.c $(DEVICE_ARCH_DIR)/bootloader_magic.h
$(BUILDDIR)/tinyusb_hardware.o: $(DEVICE_ARCH_DIR)/tinyusb_hardware.c $(TINYUSB_DIR)/tusb.h
