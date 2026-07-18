/* Copyright 2025 QMK
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * AAKBD: minimal version — all config values come from -D flags. */
#pragma once

#ifndef BACKING_STORE_WRITE_SIZE
#error "BACKING_STORE_WRITE_SIZE must be defined"
#endif

#ifndef WEAR_LEVELING_BACKING_SIZE
#error "WEAR_LEVELING_BACKING_SIZE must be defined"
#endif

#ifndef WEAR_LEVELING_LOGICAL_SIZE
#error "WEAR_LEVELING_LOGICAL_SIZE must be defined"
#endif
