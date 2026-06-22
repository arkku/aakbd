/**
 * host_fingerprint.h: Try to fingerprint/identify the USB host OS based on the
 * wLength observed in requests.
 *
 * Implement this function in, e.g., `macros.c` to react to fingerprints:
 *
 *     void host_os_fingerprint_updated(uint8_t fingerprint);
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
#ifndef HOST_FINGERPRINT_H
#define HOST_FINGERPRINT_H

#include "usbkbd_config.h"

#if ENABLE_HOST_FINGERPRINT
#include <stdint.h>
#include <stdbool.h>

#define HOST_FINGERPRINT_SEEN_0XFF      (1 << 0)
#define HOST_FINGERPRINT_SEEN_NOT_0XFF  (1 << 1)
#define HOST_FINGERPRINT_OS_BIT_MASK    (HOST_FINGERPRINT_SEEN_0XFF | HOST_FINGERPRINT_SEEN_NOT_0XFF)

#define HOST_FINGERPRINT_1ST_2          (1 << 2)
#define HOST_FINGERPRINT_3RD_2          (1 << 3)
#define HOST_FINGERPRINT_4TH_NOT_2      (1 << 4)
#define HOST_FINGERPRINT_ALTERNATING_2_MASK (HOST_FINGERPRINT_1ST_2 | HOST_FINGERPRINT_3RD_2 | HOST_FINGERPRINT_4TH_NOT_2)

enum host_os_fingerprint {
    HOST_OS_UNKNOWN = 0,
    HOST_OS_LINUX = (HOST_FINGERPRINT_SEEN_0XFF),
    HOST_OS_MACOS = (HOST_FINGERPRINT_SEEN_NOT_0XFF),
    HOST_OS_WINDOWS = (HOST_FINGERPRINT_SEEN_0XFF | HOST_FINGERPRINT_SEEN_NOT_0XFF)
};

#ifndef HOST_FINGERPRINT_RING_SIZE
/// The ring buffer size for `host_fingerprint_get_wlength_at(index)`.
#define HOST_FINGERPRINT_RING_SIZE 0
#endif

/// Reset the fingerprint (e.g., after USB reset).
void host_fingerprint_reset(void);

/// This must be called from the USB implementation to report the wLength in
/// each descriptor query.
void host_fingerprint_observe(uint16_t wlength);

/// The raw host OS fingerprint bit flags. Normally you probably want to
/// use `host_fingerprint_os_guess()` to get a guess of the OS.
uint8_t host_fingerprint_bits(void);

/// Current guess of the host OS.
enum host_os_fingerprint host_fingerprint_os_guess(void);

/// The number of observations for the fingerprint. (This wraps around after
/// 255 but not to zero.
uint8_t host_fingerprint_count(void);

/// The number of wLength observations in the fingerprint ring buffer. This
/// is the maximum index for `host_fingerprint_get_wlength_at(index)`.
uint8_t host_fingerprint_wlength_count(void);

/// Get the wLength observation at `index`. These raw observations can be used
/// to detect patterns between hosts beyond the simple fingerprint analysis
/// for `host_fingerprint_os_guess()`. You might (or might not) be able to
/// tell your own specific computers apart from the raw observations. Use the
/// `host_fingerprint_wlength_count()` function to get the maximum index.
uint16_t host_fingerprint_get_wlength_at(uint8_t index);

/// Call this to check whether a notification is pending and subscribed to,
/// if yes, call `host_os_fingerprint_updated(fingerprint)`. This must be
/// called from somewhere suitable for the handler, e.g., not from inside
/// an interrupt handler (where the observation might come from).
///
/// - Note: This just enables the notifications, it is also possible to just
/// manually poll `host_fingerprint_os_guess()` where appropriate.
void host_fingerprint_notify_if_needed(void);

/// Call this to stop notifications about fingerprint updates until reset.
/// For example, if the fingerprint is acted on, or the user has already
/// started typing and we don't want to make unexpected changes to
/// the configuration. The fingerprint will continue to be updated and can be
/// queried manually; this only stops the notifications.
void host_fingerprint_stop_notifications(void);

/// Called when new observations are added to the fingerprint. Note that the
/// actual guess of the host OS might not change as a result, the intent is
/// that `host_fingerprint_get_wlength_at` can be used to detect patterns
/// beyond the (fairly simple) built-in fingerprinting.
///
/// The fingerprint passed here as the argument is the raw fingerprint. Call
/// `host_fingerprint_os_guess()` to get the guess.
///
/// - Note: Call `host_fingerprint_stop_notifications()` to unsubscribe until
/// the next reset occurs.
void host_os_fingerprint_updated(uint8_t fingerprint);

#endif /* ENABLE_HOST_FINGERPRINT */
#endif /* HOST_FINGERPRINT_H */
