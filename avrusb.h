/**
 * avrusb.h: Helper macros for using AVR USB.
 *
 * The idea here is that the code is relatively easy to port to
 * another platform by substituting this header and a few other
 * things.
 *
 * Copyright (c) 2021 Kimmo Kulovesi, https://arkku.dev/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

#ifndef KK_AVRUSB_H
#define KK_AVRUSB_H

#include <avr/io.h>

#if (defined(__AVR_AT90USB162__) || defined(__AVR_AT90USB82__)  || defined(__AVR_ATmega32U2__) || defined(__AVR_ATmega16U2__) || defined(__AVR_ATmega8U2__))
#define USB_SERIES_2_AVR
#elif (defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__))
#define USB_SERIES_4_AVR
#endif

#ifdef EPRST6
#define USB_MAX_ENDPOINT            6
#elif defined(EPRST5)
#define USB_MAX_ENDPOINT            5
#elif defined(EPRST4)
#define USB_MAX_ENDPOINT            4
#elif defined(EPRST3)
#define USB_MAX_ENDPOINT            3
#else
#error "No USB endpoints on this device?"
#endif

#define EP_SIZE_8                   (0)
#define EP_SIZE_16                  ((1 << EPSIZE0))
#define EP_SIZE_32                  ((1 << EPSIZE1))
#define EP_SIZE_64                  ((1 << EPSIZE0) | (1 << EPSIZE1))

#define EP_ALLOC                    (1 << ALLOC)
#define EP_SINGLE_BUFFER            (EP_ALLOC | (0 << EPBK0))
#define EP_DOUBLE_BUFFER            (EP_ALLOC | (1 << EPBK0))

#define EP_SIZE_FLAGS(s)            ((s) == 64 ? EP_SIZE_64 : ((s) == 32 ? EP_SIZE_32 : ((s) == 16 ? EP_SIZE_16 : EP_SIZE_8)))
#define IS_ENDPOINT_SIZE_VALID(s)   ((s) == 64 || (s) == 32 || (s) == 16 || (s) == 8)

#define EP_TYPE_CONTROL             (0)
#define EP_TYPE_BULK                (1 << EPTYPE1)
#define EP_TYPE_INTERRUPT           ((1 << EPTYPE0) | (1 << EPTYPE1))
#define EP_TYPE_ISOCHRONOUS         (1 << EPTYPE0)
#define EP_TYPE_IN_FLAG             (1 << EPDIR)

#define EP_TYPE_BULK_IN             (EP_TYPE_BULK | EP_TYPE_IN_FLAG)
#define EP_TYPE_BULK_OUT            (EP_TYPE_BULK)
#define EP_TYPE_INTERRUPT_IN        (EP_TYPE_INTERRUPT | EP_TYPE_IN_FLAG)
#define EP_TYPE_INTERRUPT_OUT       (EP_TYPE_INTERRUPT)
#define EP_TYPE_ISOCHRONOUS_IN      (EP_TYPE_ISOCHRONOUS | EP_TYPE_IN_FLAG)
#define EP_TYPE_ISOCHRONOUS_OUT     (EP_TYPE_ISOCHRONOUS)

#define INT_END_OF_RESET_FLAG       (1 << EORSTI)
#define INT_START_OF_FRAME_FLAG     (1 << SOFI)
#define INT_WAKE_UP_FLAG            (1 << WAKEUPI)
#define INT_SUSPEND_FLAG            (1 << SUSPI)

#define is_usb_rw_allowed           (UEINTX & (1 << RWAL))
#define is_usb_tx_in_ready          (UEINTX & (1 << TXINI))
#define is_usb_rx_out_ready         (UEINTX & (1 << RXOUTI))
#define is_usb_in_or_out_ready      (UEINTX & ((1 << TXINI) | (1 << RXOUTI)))
#define is_usb_rx_int_setup         (UEINTX & (1 << RXSTPI))
#define is_usb_stalled              (UEINTX & (1 << STALLEDI))
#define is_usb_stall_requested      (UECONX & (1 << STALLRQ))
#define is_usb_remote_wakeup_set    (UDCON & (1 << RMWKUP))

#define usb_wait_tx_in()            do { } while (!is_usb_tx_in_ready)
#define usb_wait_rx_out()           do { } while (!is_usb_rx_out_ready)
#define usb_wait_in_or_out()        do { } while (!is_usb_in_or_out_ready)

#if defined(UEBCX)
#define usb_fifo_byte_count         UEBCX
#elif defined(UEBCHX)
#define usb_fifo_byte_count         ((((uint16_t) UEBCHX) << 8) | UEBCLX)
#else
#define usb_fifo_byte_count         UEBCLX
#endif
#define usb_interrupt_flags_reg     UDINT
#define usb_interrupt_enable_reg    UDIEN
#define usb_frame_count             UDFNUML

#ifdef UHWCON
#define usb_hardware_init()         (UHWCON = (1 << UVREGE))
#else
#define usb_hardware_init()         (REGCR &= ~(1 << REGDIS))
#endif
#define usb_freeze()                (USBCON = (1 << USBE) | (1 << FRZCLK))

#ifdef OTGPADE
#define usb_start_clock()           (USBCON = (USBCON & ~(1 << FRZCLK)) | (1 << OTGPADE))
#define usb_disable()               (USBCON &= ~((1 << USBE) | (1 << OTGPADE)))
#else
#define usb_start_clock()           (USBCON &= ~((1 << FRZCLK)))
#define usb_disable()               (USBCON &= ~((1 << USBE)))
#endif

#ifdef LSM
#define usb_attach()                (UDCON &= ~((1 << RSTCPU) | (1 << LSM) | (1 << RMWKUP) | (1 << DETACH)))
#else
#define usb_attach()                (UDCON &= ~((1 << RSTCPU) | (1 << RMWKUP) | (1 << DETACH)))
#endif
#define usb_detach()                (UDCON |= (1 << DETACH))

#define usb_set_endpoint(num)       (UENUM = (num))
#define usb_enable_endpoint()       (UECONX = (1 << EPEN))
#define usb_disable_endpoint()      (UECONX = 0)
#define usb_set_endpoint_type(t)    (UECFG0X = (t))
#define usb_set_endpoint_flags_and_size(f, size)   (UECFG1X = (f) | EP_SIZE_FLAGS((size)))

#define usb_deallocate_endpoint()   (UECFG1X &= ~EP_ALLOC)

#define usb_endpoint_flags_config       UECFG1X
#define usb_endpoint_type_config        UECFG0X
#define usb_endpoint_interrupts_config  UEIENX

#define usb_clear_interrupts(x)         (usb_interrupt_flags_reg &= ~(x))
#ifdef USBINT
#define usb_clear_all_interrupts(x)     do { usb_interrupt_flags_reg = 0; USBINT = 0; } while (0)
#else
#define usb_clear_all_interrupts(x)     (usb_interrupt_flags_reg = 0)
#endif
#define usb_enable_interrupts(x)        (usb_interrupt_enable_reg |= (x))
#define usb_disable_interrupts(x)       (usb_interrupt_enable_reg &= ~(x))
#define usb_set_enabled_interrupts(x)   (usb_interrupt_enable_reg = (x))

#define usb_enable_endpoint_interrupts()    (UEIENX = (1 << RXSTPE))

#define usb_tx(byte)                (UEDATX = (byte))
#define usb_rx()                    (UEDATX)
#define usb_ack_rx_out()            (UEINTX = (uint8_t) ~(((1 << RXOUTI) | (1 << FIFOCON))))
#define usb_flush_tx_in()           (UEINTX = (uint8_t) ~(((1 << TXINI) | (1 << FIFOCON))))

#define pll_enable()                do { \
    PLLCSR = PLL_DIV_FLAG; \
    (PLLCSR = (1 << PLLE) | PLL_DIV_FLAG); \
} while (0)

#define pll_disable()               (PLLCSR &= ~(1 << PLLE))
#define is_pll_locked               (PLLCSR & (1 << PLOCK))

#define usb_stall()                 (UECONX = (1 << STALLRQ) | (1 << EPEN))
#define usb_clear_stall()           (UECONX = (1 << STALLRQC) | (1 << RSTDT) | (1 << EPEN))
#define usb_reset_data_toggle()     (UECONX |= (1 << RSTDT))

#define usb_set_remote_wakeup()     (UDCON |= (1 << RMWKUP))
#define usb_clear_remote_wakeup()   (UDCON &= ~(1 << RMWKUP))

#define usb_set_address(addr)       (UDADDR = (addr) | (1 << ADDEN))
#define usb_get_address()           (UDADDR & ~(1 << ADDEN))

#define usb_clear_setup()           (UEINTX &= ~(1 << RXSTPI))
#define usb_clear_setup_int()       (UEINTX = ~((1 << RXSTPI) | (1 << RXOUTI) | (1 << TXINI)))
#define usb_release_rx()            (UEINTX = (1 << NAKINI) | (1 << RWAL) | (1 << RXSTPI) | (1 << STALLEDI) | (1 << TXINI)) // 0x6B
#define usb_release_tx()            (UEINTX = (1 << NAKOUTI) | (1 << RWAL) | (1 << RXSTPI) | (1 << STALLEDI)) // 0x3A

#define usb_flush_endpoint(num)     do {    \
    usb_set_endpoint((ep));                 \
    if (usb_fifo_byte_count) {              \
        usb_release_tx();                   \
    }                                       \
} while (0)

#define usb_reset_endpoint(num) do { \
    UERST = (1 << (num));            \
    UERST = 0;                       \
} while (0)

#define usb_reset_endpoints_1to(num) do {                                   \
    UERST = (num == 7) ? 0x7E : (((1 << ((num) + 1)) - 1U) & ~1U);          \
    UERST = 0;                                                              \
} while (0)

#define usb_setup_endpoint(number, type, size, flags) do {                  \
    usb_set_endpoint(number);                                               \
    usb_enable_endpoint();                                                  \
    usb_set_endpoint_type((type));                                          \
    usb_set_endpoint_flags_and_size(flags, size);                           \
} while (0)

#if EORSTE != EORSTI || SOFE != SOFI || WAKEUPE != WAKEUPI || SUSPE != SUSPI
#error "Interrupt enable and state flags differ."
#endif
#if EPRST0 != 0 || EPRST1 != 1 || EPRST2 != 2 || EPRST3 != 3
#error "Weird endpoint reset flags."
#endif

#if F_CPU == 16000000UL
#ifdef PINDIV
#define PLL_DIV_FLAG            (1 << PINDIV)
#elif defined(__AVR_ATmega32U2__)
#define PLL_DIV_FLAG            (1 << PLLP0)
#endif
#elif F_CPU == 8000000UL
#define PLL_DIV_FLAG            (0)
#warning "16 MHz F_CPU highly recommended!"
#else
#error "Unsupported F_CPU"
#endif

#endif
