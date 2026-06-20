/**
 * bootloader_magic.h: STM32F303 DFU bootloader magic constants.
 *
 * The magic address is at the top of SRAM (0x20007FF0 = end of 32KB minus
 * 16 bytes). Writing 0xDEADBEEF here before a soft reset causes the startup
 * code to jump to the ROM DFU bootloader instead of the application.
 */

#pragma once

#include <stdint.h>

#define BOOTLOADER_MAGIC_ADDRESS   ((volatile uint32_t *)0x20007FF0U)
#define BOOTLOADER_MAGIC           0xDEADBEEFU

#define SYSTEM_MEMORY_BASE         0x1FFFD800U
