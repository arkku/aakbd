#include "keyboard.h"
#include "matrix.h"
#include "timer.h"

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

void keyboard_setup(void) {
    matrix_setup();
    keyboard_pre_init_kb();
}

void keyboard_init(void) {
    timer_init();
    matrix_init();
    keyboard_post_init_kb(); /* Always keep this last */
}
