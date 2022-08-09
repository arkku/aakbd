#ifndef KK_QMK_PORT_H
#define KK_QMK_PORT_H

#include "quantum.h"
#include <stdbool.h>
#include <progmem.h>

void platform_setup(void);
void protocol_setup(void);
void protocol_pre_init(void);
void protocol_post_init(void);

bool matrix_has_keys_pressed(void);

#define usb_keycode_for_matrix(row, column) pgm_read_byte(&keymaps[0][(row)][(column)])

#endif
