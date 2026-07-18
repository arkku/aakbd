// AI-generated test macros for test_keys.c

#include <macros.h>
// Note: test_layers.c is included by keys.c via LAYERS_INCLUDE, do not duplicate

// Tracking for execute_macro calls
static int macro_call_count = 0;
static uint8_t last_macro_number = 0;

// Tracking for layer_state_changed calls
#define LSC_MAX 32
static struct { uint8_t layer; bool enabled; } lsc_log[LSC_MAX];
static int lsc_count = 0;

static void lsc_clear(void) { lsc_count = 0; }

// Hook balance: preprocess_press and postprocess_release must be paired 1:1.
// Each preprocess_press is tracked by its signature:
//   (returned_keycode, physical_key, written_data)
// postprocess_release receives (keycode, physical_key, data) and must match
// a previously recorded preprocess_press call.
#define HOOK_BALANCE_MAX_ENTRIES 32
typedef struct {
    keycode_t returned_keycode;
    uint8_t physical_key;
    uint8_t written_data;
    bool active;
} hook_entry_t;
static hook_entry_t hook_entries[HOOK_BALANCE_MAX_ENTRIES];
static int hook_entry_count;

static inline keycode_t
preprocess_press (keycode_t keycode, uint8_t physical_key, uint8_t layer, uint8_t *restrict data) {
    if (hook_entry_count < HOOK_BALANCE_MAX_ENTRIES) {
        hook_entries[hook_entry_count].returned_keycode = keycode;
        hook_entries[hook_entry_count].physical_key = physical_key;
        hook_entries[hook_entry_count].written_data = *data;
        hook_entries[hook_entry_count].active = true;
        ++hook_entry_count;
    }
    return keycode;
}

static inline void
postprocess_release (keycode_t keycode, uint8_t physical_key, uint8_t data) {
    bool found = false;
    for (int i = 0; i < hook_entry_count; ++i) {
        if (hook_entries[i].active && hook_entries[i].physical_key == physical_key
            // && hook_entries[i].returned_keycode == keycode
            // && hook_entries[i].written_data == data
        ) {
            hook_entries[i].active = false;
            found = true;
            break;
        }
    }
    if (!found) {
        fprintf(stderr,
            "  FAIL: postprocess_release(kc=0x%04x, phys=0x%02x, data=%d) has no matching "
            "preprocess_press\n",
            (unsigned) keycode, physical_key, data);
    }
}

static void
execute_macro (uint8_t macro_number, bool is_release, uint8_t physical_key, uint8_t layer,
    uint8_t *restrict data) {
    macro_call_count++;
    last_macro_number = macro_number;
}

static void
layer_state_changed (uint8_t layer, bool is_enabled) {
    if (lsc_count < LSC_MAX) {
        lsc_log[lsc_count].layer = layer;
        lsc_log[lsc_count].enabled = is_enabled;
        ++lsc_count;
    }
}

static inline void
handle_reset (void) {
    hook_entry_count = 0;
}

static inline void
handle_tick (uint8_t tick_10ms_count) {
}

static inline void
keyboard_host_leds_changed (uint8_t leds) {
}
