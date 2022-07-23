#ifndef KK_QMK_EECONFIG_H
#define KK_QMK_EECONFIG_H

#include <stdint.h>

uint32_t eeconfig_read_kb(void);
void eeconfig_update_kb(uint32_t val);
void eeconfig_init_user(void);
void eeconfig_init_kb(void);

#endif
