/**
 * timer.c: ARM SysTick-based 1ms timer.
 */

#include <stdint.h>
#include "platform_deps.h"
#include "timer.h"

volatile uint32_t timer_count;
static uint32_t saved_ms;

void timer_init(void) {
    timer_count = 0;
    SysTick_Config(SystemCoreClock / 1000);

    // Enable DWT cycle counter for cycle-accurate delays
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void timer_clear(void) {
    timer_count = 0;
}

void timer_save(void) {
    saved_ms = timer_count;
}

void timer_restore(void) {
    timer_count = saved_ms;
}

uint16_t timer_read(void) {
    return (uint16_t)(timer_count & 0xFFFF);
}

uint32_t timer_read32(void) {
    return timer_count;
}

uint16_t timer_elapsed(uint16_t last) {
    return TIMER_DIFF_16(timer_read(), last);
}

uint32_t timer_elapsed32(uint32_t last) {
    return TIMER_DIFF_32(timer_read32(), last);
}

void SysTick_Handler(void) {
    timer_count++;
}
