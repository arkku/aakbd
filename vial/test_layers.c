// AI-generated test layers for test_keys.c

#include <layers.h>

// Static (PROGMEM) layers for tests. STATIC_LAYER_1 = VIAL_LAYER_COUNT + 1.
// Currently VIAL_LAYER_COUNT = 4, so STATIC_LAYER_1 = 5.
_Static_assert(
    STATIC_LAYER_1 == VIAL_LAYER_COUNT + 1, "STATIC_LAYER_1 must equal VIAL_LAYER_COUNT + 1");

#define OSM_OSL_LAYER STATIC_LAYER_1

DEFINE_LAYER(OSM_OSL_LAYER){
    [KEY(F13)] = MOD_ONESHOT(CTRL_BIT),
    [KEY(F14)] = LAYER_ONESHOT(2),
};

#undef TEST_OSM_LAYER
#define TEST_OSM_LAYER STATIC_LAYER_1

// Layers 2-5: layer resolution tests
DEFINE_LAYER(STATIC_LAYER_2){
    [KEY(A)] = KEY(B),
    [KEY(C)] = KEY(D),
    [KEY(F15)] = LAYER_SET_BASE(4),
};

DEFINE_LAYER(STATIC_LAYER_3){
    [KEY(A)] = KEY(C),
};

DEFINE_LAYER(STATIC_LAYER_4){
    [KEY(B)] = KEY(D),
};

DEFINE_LAYER(STATIC_LAYER_5){
    [KEY(A)] = KEY(D),
};

#define LAYER_COUNT STATIC_LAYER_5

enum macro {
    MACRO_NOP,
};
