#include <stdint.h>
#include <stdbool.h>
#include "eeconfig.h"
#include "dynamic_storage.h"

#ifdef EEPROM_DRIVER
#include "eeprom_driver.h"
#endif
#ifdef HAPTIC_ENABLE
#include "haptic.h"
#endif

uint32_t eeconfig_read_kb(void) {
    return eeprom_read_dword(EECONFIG_KEYBOARD);
}

void eeconfig_update_kb(uint32_t val) {
    eeprom_update_dword(EECONFIG_KEYBOARD, val);
}

uint32_t eeconfig_read_user(void) {
    return eeprom_read_dword(EECONFIG_USER);
}

void eeconfig_update_user(uint32_t val) {
    eeprom_update_dword(EECONFIG_USER, val);
}

#if defined(HAPTIC_ENABLE) && defined(EECONFIG_HAPTIC)
void eeconfig_read_haptic(haptic_config_t *haptic_config) {
    haptic_config->raw = eeprom_read_dword(EECONFIG_HAPTIC);
}

void eeconfig_update_haptic(const haptic_config_t *haptic_config) {
    eeprom_update_dword(EECONFIG_HAPTIC, haptic_config->raw);
}
#endif

__attribute__((weak)) void eeconfig_init_user(void) {
    eeconfig_update_user(0);
}

__attribute__((weak)) void eeconfig_init_kb(void) {
    eeconfig_update_kb(0);
    eeconfig_init_user();
}

void eeconfig_init_quantum(void) {
#if defined(EEPROM_DRIVER)
    eeprom_driver_erase();
#else
    for (intptr_t i = 0; i < EECONFIG_SIZE; ++i) {
        eeprom_update_byte((uint8_t *) i, 0);
    }
#endif

    eeprom_update_word(EECONFIG_MAGIC_NUMBER_PTR, EECONFIG_MAGIC_NUMBER);

#if defined(HAPTIC_ENABLE)
    haptic_reset();
#endif

    eeconfig_init_kb();
}

void eeconfig_init(void) {
    eeconfig_init_quantum();
}

void eeconfig_enable(void) {
    eeprom_update_word(EECONFIG_MAGIC_NUMBER_PTR, EECONFIG_MAGIC_NUMBER);
}

void eeconfig_disable(void) {
#if defined(EEPROM_DRIVER)
    eeprom_driver_erase();
#endif
    eeprom_update_word(EECONFIG_MAGIC_NUMBER_PTR, EECONFIG_MAGIC_NUMBER_OFF);
}

bool eeconfig_is_enabled(void) {
    return (eeprom_read_word(EECONFIG_MAGIC_NUMBER_PTR) == EECONFIG_MAGIC_NUMBER);
}

bool eeconfig_is_disabled(void) {
    return (eeprom_read_word(EECONFIG_MAGIC_NUMBER_PTR) == EECONFIG_MAGIC_NUMBER_OFF);
}

layer_state_t eeconfig_read_default_layer(void) {
    return (layer_state_t)eeprom_read_byte(EECONFIG_DEFAULT_LAYER);
}

void eeconfig_update_default_layer(layer_state_t state) {
    eeprom_update_byte(EECONFIG_DEFAULT_LAYER, (uint8_t)state);
}
