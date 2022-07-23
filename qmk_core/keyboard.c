#include "keyboard.h"
#include "matrix.h"
#include "timer.h"
#include "led.h"

__attribute__((weak)) void keyboard_pre_init_user(void) {}

/** \brief keyboard_pre_init_kb
 *
 * FIXME: needs doc
 */
__attribute__((weak)) void keyboard_pre_init_kb(void) { keyboard_pre_init_user(); }

/** \brief keyboard_post_init_user
 *
 * FIXME: needs doc
 */

__attribute__((weak)) void keyboard_post_init_user() {}

/** \brief keyboard_post_init_kb
 *
 * FIXME: needs doc
 */

__attribute__((weak)) void keyboard_post_init_kb(void) { keyboard_post_init_user(); }

void quantum_init(void) {
    led_init_ports();
#ifdef BACKLIGHT_ENABLE
    backlight_init_ports();
#endif
#ifdef LED_MATRIX_ENABLE
    led_matrix_init();
#endif
#ifdef RGB_MATRIX_ENABLE
    rgb_matrix_init();
#endif
#ifdef HAPTIC_ENABLE
    haptic_init();
#endif
}

void keyboard_setup(void) {
    matrix_setup();
    keyboard_pre_init_kb();
}

void keyboard_init(void) {
    timer_init();
    matrix_init();
    quantum_init();
    keyboard_post_init_kb(); /* Always keep this last */
}
