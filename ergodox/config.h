/*
Copyright 2012 Jun Wako <wakojun@gmail.com>
Copyright 2013 Oleg Kostyuk <cub.uanic@gmail.com>
Copyright 2015 ZSA Technology Labs Inc (@zsa)
Copyright 2020 Christopher Courtney <drashna@live.com> (@drashna)

Ported from QMK to AAKBD by Kimmo Kulovesi, 2021. Any issues are
almost certainly caused by that and not by the original authors.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#ifndef DEBOUNCE
/// Number of milliseconds to debounce switches. Cherry MX switches should be
/// ok with 5, but some other switches may (according to internet forums)
/// require 15 or more.
#define DEBOUNCE                8
#endif

#define MATRIX_ROWS             14
#define MATRIX_ROWS_PER_SIDE    (MATRIX_ROWS / 2)
#define MATRIX_COLS             6

#ifndef LED_BRIGHTNESS_LO
#define LED_BRIGHTNESS_LO       15
#endif
#ifndef LED_BRIGHTNESS_HI
#define LED_BRIGHTNESS_HI       255
#endif
#define LED_BRIGHTNESS_DEFAULT (LED_BRIGHTNESS_HI)
