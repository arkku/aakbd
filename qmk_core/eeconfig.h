#ifndef KK_QMK_EECONFIG_H
#define KK_QMK_EECONFIG_H

#include <stdint.h>
#include <stdbool.h>

#ifndef EECONFIG_MAGIC_NUMBER
#define EECONFIG_MAGIC_NUMBER ((uint16_t) 0xFEE8) // When changing, decrement this value to avoid future re-init issues
#endif
#define EECONFIG_MAGIC_NUMBER_OFF (uint16_t)0xFFFF

#define EECONFIG_MAGIC      ((uint16_t *) 0)
#define EECONFIG_BACKLIGHT  ((uint8_t *)  6)
#define EECONFIG_RGBLIGHT   ((uint32_t *) 8)
#define EECONFIG_KEYBOARD   ((uint32_t *) 15)
#define EECONFIG_USER       ((uint32_t *) 19)
#define EECONFIG_HAPTIC     ((uint32_t *) 24)
#define EECONFIG_LED_MATRIX ((uint32_t *) 28)
#define EECONFIG_RGB_MATRIX ((uint32_t *) 28)
#define EECONFIG_LED_MATRIX_EXTENDED ((uint16_t *) 32)
#define EECONFIG_RGB_MATRIX_EXTENDED ((uint16_t *) 32)
#define EECONFIG_KEYMAP_UPPER_BYTE ((uint8_t *) 34)

uint32_t eeconfig_read_kb(void);
void eeconfig_update_kb(uint32_t val);

uint32_t eeconfig_read_user(void);
void eeconfig_update_user(uint32_t val);

bool eeconfig_is_enabled(void);
bool eeconfig_is_disabled(void);

void eeconfig_init(void);
void eeconfig_init_quantum(void);
void eeconfig_init_user(void);
void eeconfig_init_kb(void);

#ifdef HAPTIC_ENABLE
uint32_t eeconfig_read_haptic(void);
void eeconfig_update_haptic(uint32_t val);
#endif

#endif
