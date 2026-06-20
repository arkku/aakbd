/**
 * atomic_util.h: ATOMIC_BLOCK for ARM (disable/enable IRQ).
 */

#pragma once

#include "platform_deps.h"
#define ATOMIC_BLOCK_FORCEON \
    for (uint8_t _atomic_guard = ({ __disable_irq(); 0; }); \
         _atomic_guard == 0; \
         _atomic_guard = 1, __enable_irq())
