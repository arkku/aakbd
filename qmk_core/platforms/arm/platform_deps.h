/**
 * platform_deps.h: ARM platform includes.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Include the CMSIS device header — the device .mk must provide -DMY_MCU_DEFINE
// and the include path to the CMSIS device headers.
#if defined(STM32F303xC) || defined(STM32F3)
#include "stm32f3xx.h"
#else
#error "Add your MCU's CMSIS device header here (e.g., #include \"stm32f4xx.h\")"
#endif
