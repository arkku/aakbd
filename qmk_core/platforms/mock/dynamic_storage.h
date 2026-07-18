/* dynamic_storage.h: Mock for host testing — EEPROM is backed by RAM.
 *
 * The eeprom array has sentinel bytes at both ends to detect overflow.
 * Implementations are in the test file (test_keys.c).
 */

#ifndef AAKBD_DYNAMIC_STORAGE_H
#define AAKBD_DYNAMIC_STORAGE_H

#include <stddef.h>
#include <stdint.h>

#define EEPROM_MAX 1024
#define EEPROM_SENTINEL 0xA5

// Eeprom RAM with sentinels for overflow detection
extern uint8_t eeprom_sentinel_head;
extern uint8_t eeprom_ram[EEPROM_MAX];
extern uint8_t eeprom_sentinel_tail;

uint8_t eeprom_read_byte(const void *addr);
void eeprom_update_byte(void *addr, uint8_t val);
uint16_t eeprom_read_word(const void *addr);
void eeprom_update_word(void *addr, uint16_t val);
uint32_t eeprom_read_dword(const uint32_t *addr);
void eeprom_update_dword(uint32_t *addr, uint32_t val);
void eeprom_read_block(void *dst, const void *src, size_t n);
void eeprom_write_block(const void *buf, void *addr, size_t len);

#endif
