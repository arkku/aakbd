// via.h: Via/Vial communications protocol.

#pragma once

#include <stdint.h>

#define VIA_PROTOCOL_VERSION 0x0009

enum via_command_id {
    id_get_protocol_version = 0x01,
    id_get_keyboard_value = 0x02,
    id_set_keyboard_value = 0x03,
    id_dynamic_keymap_get_keycode = 0x04,
    id_dynamic_keymap_set_keycode = 0x05,
    id_dynamic_keymap_reset = 0x06,
    id_lighting_set_value = 0x07,
    id_lighting_get_value = 0x08,
    id_lighting_save = 0x09,
    id_eeprom_reset = 0x0A,
    id_bootloader_jump = 0x0B,
    id_dynamic_keymap_macro_get_count = 0x0C,
    id_dynamic_keymap_macro_get_buffer_size = 0x0D,
    id_dynamic_keymap_macro_get_buffer = 0x0E,
    id_dynamic_keymap_macro_set_buffer = 0x0F,
    id_dynamic_keymap_macro_reset = 0x10,
    id_dynamic_keymap_get_layer_count = 0x11,
    id_dynamic_keymap_get_buffer = 0x12,
    id_dynamic_keymap_set_buffer = 0x13,
    id_vial_prefix = 0xFE,
    id_unhandled = 0xFF,
};

enum via_keyboard_value_id {
    id_uptime = 0x01,
    id_layout_options = 0x02,
    id_switch_matrix_state = 0x03,
};
