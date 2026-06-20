#include "qmk_port.h"
#include "platform_deps.h"
#include <avr/power.h>
#include <avr/wdt.h>

void protocol_pre_init(void) {
    cli();
}

void protocol_post_init(void) {
    sei();
}

void protocol_setup(void) {
    /* Disable watchdog if enabled by bootloader/fuses */
    MCUSR &= ~_BV(WDRF);
    wdt_disable();

// For boards running at 3.3V and crystal at 16 MHz
#if (F_CPU == 8000000 && F_USB == 16000000)
    /* Divide clock by 2 */
    clock_prescale_set(clock_div_2);
#else /* Disable clock division */
    clock_prescale_set(clock_div_1);
#endif
}
