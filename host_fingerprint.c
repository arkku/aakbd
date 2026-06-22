/**
 * host_fingerprint.c: Try to fingerprint/identify the USB host OS based on the
 * wLength observed in requests.
 *
 * Copyright 2026 Kimmo Kulovesi
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
#include "host_fingerprint.h"

#if ENABLE_HOST_FINGERPRINT

#ifndef HOST_FINGERPRINT_MIN_COUNT
/// Gather at least this many observations before reporting the fingerprint.
/// 5 seems to be a good value to distinguish all three major OS in practice.
#define HOST_FINGERPRINT_MIN_COUNT 5
#endif

#define HOST_FINGERPRINT_MAX_COUNT 255

__attribute__((weak)) void
host_os_fingerprint_updated (uint8_t fingerprint) { }

static uint8_t fingerprint = 0;
static uint8_t fingerprint_count = 0;
static uint8_t reported_count = 0;

#if HOST_FINGERPRINT_RING_SIZE > 0
static uint16_t fingerprint_wlen[HOST_FINGERPRINT_RING_SIZE] = { 0 };
#else
static uint8_t reported_fingerprint = 0;
#endif

void
host_fingerprint_observe (uint16_t wLength) {
#if HOST_FINGERPRINT_RING_SIZE > 0
    fingerprint_wlen[fingerprint_count % HOST_FINGERPRINT_RING_SIZE] = wLength;
#endif

    if (wLength == 0xFF) {
        fingerprint |= HOST_FINGERPRINT_SEEN_0XFF;
    } else if (wLength != 0) {
        fingerprint |= HOST_FINGERPRINT_SEEN_NOT_0XFF;

        if (wLength == 2) {
            if (fingerprint_count == 0) {
                fingerprint |= HOST_FINGERPRINT_1ST_2;
            } else if (fingerprint_count == 2) {
                fingerprint |= HOST_FINGERPRINT_3RD_2;
            }
        } else if (fingerprint_count == 3) {
            fingerprint |= HOST_FINGERPRINT_4TH_NOT_2;
        }
    }

    if (fingerprint_count < HOST_FINGERPRINT_MAX_COUNT) {
        ++fingerprint_count;
    } else {
#if HOST_FINGERPRINT_RING_SIZE > 0
        fingerprint_count = HOST_FINGERPRINT_RING_SIZE + (HOST_FINGERPRINT_MAX_COUNT % HOST_FINGERPRINT_RING_SIZE) + 1;
#else
        fingerprint_count = HOST_FINGERPRINT_MIN_COUNT;
#endif
    }
}

uint8_t
host_fingerprint_bits (void) {
    return fingerprint;
}

enum host_os_fingerprint
host_fingerprint_os_guess (void) {
    enum host_os_fingerprint os = (fingerprint & HOST_FINGERPRINT_OS_BIT_MASK);

    if (os == HOST_OS_WINDOWS &&
        (fingerprint & HOST_FINGERPRINT_ALTERNATING_2_MASK) == HOST_FINGERPRINT_ALTERNATING_2_MASK
    ) {
        // Corner case for macOS misdetected as Windows
        return HOST_OS_MACOS;
    }

    return os;
}

uint8_t
host_fingerprint_count (void) {
    return fingerprint_count;
}

uint16_t
host_fingerprint_get_wlength_at (uint8_t index) {
#if HOST_FINGERPRINT_RING_SIZE > 0
    if (fingerprint_count < HOST_FINGERPRINT_RING_SIZE) {
        return fingerprint_wlen[index];
    }
    return fingerprint_wlen[(fingerprint_count + index) % HOST_FINGERPRINT_RING_SIZE];
#else
    return 0;
#endif
}

uint8_t
host_fingerprint_wlength_count (void) {
#if HOST_FINGERPRINT_RING_SIZE > 0
    return fingerprint_count < HOST_FINGERPRINT_RING_SIZE ? fingerprint_count : HOST_FINGERPRINT_RING_SIZE;
#else
    return 0;
#endif
}

void
host_fingerprint_notify_if_needed (void) {
    if (reported_count < fingerprint_count &&
        fingerprint_count >= HOST_FINGERPRINT_MIN_COUNT
#if HOST_FINGERPRINT_RING_SIZE == 0
        && reported_fingerprint != fingerprint
#endif
    ) {
#if HOST_FINGERPRINT_RING_SIZE == 0
        reported_fingerprint = fingerprint;
#endif
        reported_count = fingerprint_count;
        host_os_fingerprint_updated(fingerprint);
    }
}

void
host_fingerprint_stop_notifications (void) {
    reported_count = HOST_FINGERPRINT_MAX_COUNT;
}

void
host_fingerprint_reset (void) {
    fingerprint = 0;
    fingerprint_count = 0;
    reported_count = 0;
}

#endif /* ENABLE_HOST_FINGERPRINT */
