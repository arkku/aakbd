#include "eeconfig.h"
#include <avr/eeprom.h>

#if defined(EEPROM_DRIVER)
#include "eeprom_driver.h"
#endif

#if defined(HAPTIC_ENABLE)
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

#ifdef EECONFIG_HAPTIC
uint32_t eeconfig_read_haptic(void) {
    return eeprom_read_dword(EECONFIG_HAPTIC);
}

void eeconfig_update_haptic(uint32_t val) {
    eeprom_update_dword(EECONFIG_HAPTIC, val);
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
#endif
    eeprom_update_word(EECONFIG_MAGIC, EECONFIG_MAGIC_NUMBER);
    eeprom_update_byte(EECONFIG_BACKLIGHT, 0);
    eeprom_update_dword(EECONFIG_RGBLIGHT, 0);
    eeprom_update_dword(EECONFIG_RGB_MATRIX, 0);
    eeprom_update_word(EECONFIG_RGB_MATRIX_EXTENDED, 0);

#if defined(HAPTIC_ENABLE)
    haptic_reset();
#else
    // this is used in case haptic is disabled, but we still want sane defaults
    // in the haptic configuration eeprom. All zero will trigger a haptic_reset
    // when a haptic-enabled firmware is loaded onto the keyboard.
    eeprom_update_dword(EECONFIG_HAPTIC, 0);
#endif
    eeconfig_init_kb();
}

void eeconfig_init(void) {
    eeconfig_init_quantum();
}

void eeconfig_enable(void) {
    eeprom_update_word(EECONFIG_MAGIC, EECONFIG_MAGIC_NUMBER);
}

void eeconfig_disable(void) {
#if defined(EEPROM_DRIVER)
    eeprom_driver_erase();
#endif
    eeprom_update_word(EECONFIG_MAGIC, EECONFIG_MAGIC_NUMBER_OFF);
}

bool eeconfig_is_enabled(void) {
    return (eeprom_read_word(EECONFIG_MAGIC) == EECONFIG_MAGIC_NUMBER);
}

bool eeconfig_is_disabled(void) {
    return (eeprom_read_word(EECONFIG_MAGIC) == EECONFIG_MAGIC_NUMBER_OFF);
}
