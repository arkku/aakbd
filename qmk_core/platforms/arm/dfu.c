/**
 * dfu.c: Bootloader jump for ARM (NVIC_SystemReset + magic).
 *
 * The magic address is checked in the startup code (Reset_Handler in
 * arch/arm/stm32f3/startup_stm32f303xc.c) which then performs a direct
 * jump to the ROM DFU bootloader (system memory). Only safe in the
 * reset context — DO NOT jump directly from application code!
 */

#include "bootloader.h"
#include "platform_deps.h"
#include "bootloader_magic.h"

#ifndef SYSTEM_MEMORY_BASE
#error "SYSTEM_MEMORY_BASE must be defined in bootloader_magic.h"
#endif

void bootloader_jump(void) {
    __disable_irq();

    *BOOTLOADER_MAGIC_ADDRESS = BOOTLOADER_MAGIC;

    // Disable SysTick before reset
    SysTick->CTRL = -1;

    NVIC_SystemReset();

    for (;;) {}
}

void mcu_reset(void) {
    NVIC_SystemReset();
}
