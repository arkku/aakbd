#include "eeconfig.h"
#include <avr/eeprom.h>

#define EECONFIG_KEYBOARD ((uint32_t *) 15)

uint32_t
eeconfig_read_kb(void) {
    return eeprom_read_dword(EECONFIG_KEYBOARD);
}

void
eeconfig_update_kb (uint32_t val) {
    eeprom_update_dword(EECONFIG_KEYBOARD, val);
}

void
eeconfig_init_user(void) {
}
