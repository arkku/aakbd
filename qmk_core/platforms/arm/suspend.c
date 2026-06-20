/**
 * suspend.c: ARM suspend (WFI-based power down).
 */

#include <stdint.h>
#include <stdbool.h>
#include "platform_deps.h"
#include "suspend.h"
#include "action.h"

void suspend_power_down(void) {
    suspend_power_down_quantum();
    __WFI();
}

void suspend_wakeup_init(void) {
    keyboard_wake_up();
    suspend_wakeup_init_quantum();
}
