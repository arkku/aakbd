/**
 * eeconfig.c: EEPROM config stubs for ARM (currently not supported).
 */

#include "eeconfig.h"

__attribute__((weak)) void eeconfig_init(void) { }
__attribute__((weak)) void eeconfig_enable(void) { }
__attribute__((weak)) void eeconfig_disable(void) { }
__attribute__((weak)) bool eeconfig_is_enabled(void) { return false; }
__attribute__((weak)) bool eeconfig_is_disabled(void) { return true; }
__attribute__((weak)) uint32_t eeconfig_read_kb(void) { return 0; }
__attribute__((weak)) void eeconfig_update_kb(uint32_t val) { (void)val; }
__attribute__((weak)) uint32_t eeconfig_read_user(void) { return 0; }
__attribute__((weak)) void eeconfig_update_user(uint32_t val) { (void)val; }
