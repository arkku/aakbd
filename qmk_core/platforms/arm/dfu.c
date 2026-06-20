/**
 * dfu.c: DFU bootloader jump via system memory.
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

// Called from startup code before main() if magic is present
void enter_bootloader(void) {
    // Clear the magic
    *BOOTLOADER_MAGIC_ADDRESS = 0;

    // Reconfigure the vector table to system memory
    SCB->VTOR = SYSTEM_MEMORY_BASE;

    // Set the MSP from the bootloader's vector table
    __set_MSP(*(volatile uint32_t *)SYSTEM_MEMORY_BASE);

    // Jump to the bootloader's reset handler
    typedef void (*boot_fn)(void);
    boot_fn reset = (boot_fn)*(volatile uint32_t *)(SYSTEM_MEMORY_BASE + 4);
    reset();
}
