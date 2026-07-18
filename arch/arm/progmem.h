/* progmem.h: ARM helpers for program space stored values.
 *
 * Copyright (c) 2026 Kimmo Kulovesi, https://arkku.dev/
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

#ifndef AAKBD_PROGMEM_H
#define AAKBD_PROGMEM_H

#include <string.h>

#define PROGMEM
#define PSTR(x)                       x
#define PGM_P                         const char *
#define memcpy_P(dest, src, n)        memcpy(dest, src, n)
#define pgm_read_byte(address_short)  *((const uint8_t *) (address_short))
#define pgm_read_word(address_short)  *((const uint16_t *) (address_short))
#define pgm_read_dword(address_short) *((const uint32_t *) (address_short))
#define pgm_read_ptr(address_short)   *((const void **) (address_short))
#define strcmp_P(s1, s2)              strcmp(s1, s2)
#define strcpy_P(dest, src)           strcpy(dest, src)
#define strlen_P(src)                 strlen(src)
#define fprintf_P(...)                fprintf(__VA_ARGS__)

#endif
