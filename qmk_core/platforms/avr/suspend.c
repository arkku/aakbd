/*
 * suspend.c: AVR suspend from QMK, adapted for AAKBD.
 *
 * Copyright QMK Community
 * Copyright 2021 Kimmo Kulovesi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include "suspend.h"
#include "action.h"
#include "timer.h"

extern volatile uint8_t usb_keyboard_leds;

#ifdef PROTOCOL_LUFA
#    include "lufa.h"
#endif
#ifdef PROTOCOL_VUSB
#    include "vusb.h"
#endif

#if !defined(NO_SUSPEND_POWER_DOWN) && defined(WDT_vect)

// clang-format off
#define wdt_intr_enable(value) \
__asm__ __volatile__ ( \
    "in __tmp_reg__,__SREG__" "\n\t" \
    "cli" "\n\t" \
    "wdr" "\n\t" \
    "sts %0,%1" "\n\t" \
    "out __SREG__,__tmp_reg__" "\n\t" \
    "sts %0,%2" "\n\t" \
    : /* no outputs */ \
    : "M" (_SFR_MEM_ADDR(_WD_CONTROL_REG)), \
    "r" (_BV(_WD_CHANGE_BIT) | _BV(WDE)), \
    "r" ((uint8_t) ((value & 0x08 ? _WD_PS3_MASK : 0x00) | _BV(WDIE) | (value & 0x07))) \
    : "r0" \
)
// clang-format on

/** \brief Power down MCU with watchdog timer
 *
 * wdto: watchdog timer timeout defined in <avr/wdt.h>
 *          WDTO_15MS
 *          WDTO_30MS
 *          WDTO_60MS
 *          WDTO_120MS
 *          WDTO_250MS
 *          WDTO_500MS
 *          WDTO_1S
 *          WDTO_2S
 *          WDTO_4S
 *          WDTO_8S
 */
static uint8_t wdt_timeout = 0;

static void power_down(uint8_t wdto) {
    wdt_timeout = wdto;

    wdt_intr_enable(wdto);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sei();
    sleep_cpu();
    sleep_disable();

    // Disable watchdog after sleep
    wdt_disable();
}

/* watchdog timeout */
ISR(WDT_vect) {
    // compensate timer for sleep
    switch (wdt_timeout) {
        case WDTO_15MS:
            timer_count += 15 + 2; // WDTO_15MS + 2(from observation)
            break;
        default:;
    }
}

#endif

void suspend_power_down(void) {
#ifdef PROTOCOL_LUFA
    if (USB_DeviceState == DEVICE_STATE_Configured) return;
#endif
#ifdef PROTOCOL_VUSB
    if (!vusb_suspended) return;
#endif

    suspend_power_down_quantum();

#ifndef NO_SUSPEND_POWER_DOWN
#    if defined(WDT_vect)
    power_down(WDTO_15MS);
#    endif
#endif
}

void suspend_wakeup_init(void) {
    uint8_t saved_leds = usb_keyboard_leds;

    // clear keyboard state
    keyboard_wake_up();

    usb_keyboard_leds = saved_leds;
    suspend_wakeup_init_quantum();
}
