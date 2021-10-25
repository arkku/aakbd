/**
 * avrtimer.h
 */

#ifndef KK_AVRTIMER_H
#define KK_AVRTIMER_H

#define TIMER_PASTE_(a, b)      a##b
#define TIMER_PASTE(a, b)       TIMER_PASTE_(a, b)

#ifndef TIMER_NUM
#define TIMER_NUM               0
#endif

#define TIMER_COUNTER           TIMER_PASTE(TCNT, TIMER_NUM)
#define TIMER_OCRA              TIMER_PASTE(TIMER_PASTE(OCR, TIMER_NUM), A)
#define TIMER_OCRB              TIMER_PASTE(TIMER_PASTE(OCR, TIMER_NUM), B)

#define TIMER_OVF_VECTOR        TIMER_PASTE(TIMER, TIMER_PASTE(TIMER_NUM, _OVF_vect))
#define TIMER_COMPA_VECTOR      TIMER_PASTE(TIMER, TIMER_PASTE(TIMER_NUM, _COMPA_vect))

#define TIMER_CCRA_REG          TIMER_PASTE(TIMER_PASTE(TCCR, TIMER_NUM), A)
#define TIMER_CCRB_REG          TIMER_PASTE(TIMER_PASTE(TCCR, TIMER_NUM), B)
#define TIMER_MASK_REG          TIMER_PASTE(TIMSK, TIMER_NUM)
#define TIMER_FLAG_REG          TIMER_PASTE(TIFR, TIMER_NUM)

#define TIMER_TOIE              TIMER_PASTE(TOIE, TIMER_NUM)
#define TIMER_TOIE_BIT          ((uint8_t) (1U << TIMER_TOIE))
#define TIMER_OCIEA             TIMER_PASTE(TIMER_PASTE(OCIE, TIMER_NUM), A)
#define TIMER_OCIEA_BIT         ((uint8_t) (1U << TIMER_OCIEA))
#define TIMER_OCIEB             TIMER_PASTE(TIMER_PASTE(OCIE, TIMER_NUM), B)
#define TIMER_OCIEB_BIT         ((uint8_t) (1U << TIMER_OCIEB))

#define TIMER_CS0               TIMER_PASTE(TIMER_PASTE(CS, TIMER_NUM), 0)
#define TIMER_CS0_BIT           ((uint8_t) (1U << TIMER_CS0))
#define TIMER_CS1               TIMER_PASTE(TIMER_PASTE(CS, TIMER_NUM), 1)
#define TIMER_CS1_BIT           ((uint8_t) (1U << TIMER_CS1))
#define TIMER_CS2               TIMER_PASTE(TIMER_PASTE(CS, TIMER_NUM), 2)
#define TIMER_CS2_BIT           ((uint8_t) (1U << TIMER_CS2))

#define TIMER_WGM0               TIMER_PASTE(TIMER_PASTE(WGM, TIMER_NUM), 0)
#define TIMER_WGM0_BIT           ((uint8_t) (1U << TIMER_WGM0))
#define TIMER_WGM1               TIMER_PASTE(TIMER_PASTE(WGM, TIMER_NUM), 1)
#define TIMER_WGM1_BIT           ((uint8_t) (1U << TIMER_WGM1))
#define TIMER_WGM2               TIMER_PASTE(TIMER_PASTE(WGM, TIMER_NUM), 2)
#define TIMER_WGM2_BIT           ((uint8_t) (1U << TIMER_WGM2))

#define TIMER_OCFA              TIMER_PASTE(TIMER_PASTE(OCF, TIMER_NUM), A)
#define TIMER_OCFA_BIT          ((uint8_t) (1U << TIMER_OCFA))
#define TIMER_OCFB              TIMER_PASTE(TIMER_PASTE(OCF, TIMER_NUM), B)
#define TIMER_OCFB_BIT          ((uint8_t) (1U << TIMER_OCFB))
#define TIMER_OVF_FLAG          TIMER_PASTE(TOV, TIMER_NUM)
#define TIMER_OVF_FLAG_BIT      ((uint8_t) (1U << TIMER_OVF_FLAG))

#define timer_enable_ovf()          do { TIMER_MASK_REG |= TIMER_TOIE_BIT; } while (0)
#define timer_enable_compa()        do { TIMER_MASK_REG |= TIMER_OCIEA_BIT; } while (0)
#define timer_disable()             do { TIMER_MASK_REG = 0U; } while (0)

#define timer_clear_flags()         do { TIMER_FLAG_REG |= TIMER_OVF_FLAG_BIT | TIMER_OCFA_BIT | TIMER_OCFB_BIT; } while (0)
#define timer_clear_ovf()           do { TIMER_FLAG_REG |= TIMER_OVF_FLAG_BIT; } while (0)
#define timer_reset_counter()       do { timer_clear_flags(); TIMER_COUNTER = 0U; } while (0)

#define timer_set_prescaler_1()     do { TIMER_CCRB_REG &= ~(TIMER_CS1_BIT | TIMER_CS2_BIT); TIMER_CCRB_REG |= TIMER_CS0_BIT; } while (0)
#define timer_set_prescaler_8()     do { TIMER_CCRB_REG &= ~(TIMER_CS0_BIT | TIMER_CS2_BIT); TIMER_CCRB_REG |= TIMER_CS1_BIT; } while (0)
#define timer_set_prescaler_16()    do { TIMER_CCRB_REG &= ~TIMER_CS2_BIT; TIMER_CCRB_REG |= TIMER_CS0_BIT | TIMER_CS1_BIT; } while (0)
#define timer_set_prescaler_256()   do { TIMER_CCRB_REG &= ~(TIMER_CS0_BIT | TIMER_CS1_BIT); TIMER_CCRB_REG |= TIMER_CS2_BIT; } while (0)
#define timer_set_prescaler_1024()  do { TIMER_CCRB_REG &= ~TIMER_CS1_BIT; TIMER_CCRB_REG |= TIMER_CS0_BIT | TIMER_CS2_BIT; } while (0)

#define timer_set_normal_mode()     do { TIMER_CCRB_REG &= ~TIMER_WGM2_BIT; TIMER_CCRA_REG &= ~(TIMER_WGM1_BIT | TIMER_WGM0_BIT); } while (0)
#define timer_set_ctc_mode()        do { TIMER_CCRB_REG &= ~TIMER_WGM2_BIT; TIMER_CCRA_REG &= ~TIMER_WGM0_BIT; TIMER_CCRA_REG |= TIMER_WGM1_BIT; } while (0)

#endif
