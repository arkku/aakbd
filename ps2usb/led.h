/**
 * led.h
 */

#ifndef KK_LED_H
#define KK_LED_H

#define LED_PASTE_(a, b)        a##b
#define LED_PASTE(a, b)         LED_PASTE_(a, b)

#ifndef LED_PORT
#define LED_PORT                B
#define LED_PIN                 0
#endif

#ifndef ERROR_LED_PORT
#define ERROR_LED_PORT          D
#define ERROR_LED_PIN           5
#endif

#ifndef LEDS_INVERTED
#define LEDS_INVERTED           1
#endif

#define LED_DDR                 LED_PASTE(DDR, LED_PORT)
#define LED_PORT_REG            LED_PASTE(PORT, LED_PORT)
#define LED_BIT                 ((uint8_t) (1U << (LED_PIN)))

#define ERROR_LED_DDR           LED_PASTE(DDR, ERROR_LED_PORT)
#define ERROR_LED_PORT_REG      LED_PASTE(PORT, ERROR_LED_PORT)
#define ERROR_LED_BIT           ((uint8_t) (1U << (ERROR_LED_PIN)))

#define led_set_output()        do { \
        LED_DDR |= LED_BIT; \
        ERROR_LED_DDR |= ERROR_LED_BIT; \
    } while (0)

#define led_toggle()            do { LED_PORT_REG ^= LED_BIT; } while (0)

#if LEDS_INVERTED == 1
#define led_set(state)          do { \
        if (!(state)) { \
            LED_PORT_REG |= LED_BIT; \
        } else { \
            LED_PORT_REG &= ~LED_BIT; \
        } \
    } while (0)

#define error_led_set(state)    do { \
        if (!(state)) { \
            ERROR_LED_PORT_REG |= ERROR_LED_BIT; \
        } else { \
            ERROR_LED_PORT_REG &= ~ERROR_LED_BIT; \
        } \
    } while (0)
#else
#define led_set(state)          do { \
        if ((state)) { \
            LED_PORT_REG |= LED_BIT; \
        } else { \
            LED_PORT_REG &= ~LED_BIT; \
        } \
    } while (0)

#define error_led_set(state)    do { \
        if ((state)) { \
            ERROR_LED_PORT_REG |= ERROR_LED_BIT; \
        } else { \
            ERROR_LED_PORT_REG &= ~ERROR_LED_BIT; \
        } \
    } while (0)
#endif

#endif
