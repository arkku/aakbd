/**
 * layers.h: Layer definitions for advanced key mapping.
 *
 * Copyright (c) 2021 Kimmo Kulovesi, https://arkku.dev/
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

#ifndef KK_LAYERS_H
#define KK_LAYERS_H

#include "progmem.h"
#include "keycodes.h"

// MARK: - Internals

#define LAYER_SIZE(num)     ((LAYER_COUNT >= (num)) ? (sizeof PASTE(layer, num)) / sizeof(keycode_t) : 0)

#define LAYER_ARRAY(num)    PASTE(layer, num)

#define DEFINE_LAYER(num)   static const keycode_t PASTE(layer, num)[] PROGMEM =

#define DEFINE_EMPTY_LAYER(num) DEFINE_LAYER(num) { 0 }

#define ALL_KEYS_NOT_DEFINED_BELOW [1 ... 0xFE]

#define DISABLE_ALL_KEYS_NOT_DEFINED_BELOW ALL_KEYS_NOT_DEFINED_BELOW = NONE

#ifdef __clang__
// There will be some warnings if layers.c is compiled as stand-alone, since it
// is meant to be included from keys.c
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

#endif
