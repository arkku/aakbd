#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void keyboard_setup(void);
void keyboard_init(void);

void keyboard_pre_init_kb(void);
void keyboard_pre_init_user(void);
void keyboard_post_init_kb(void);
void keyboard_post_init_user(void);

#ifdef __cplusplus
}
#endif
