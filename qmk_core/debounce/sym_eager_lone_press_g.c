/**
 * sym_eager_lone_press.g: Same as sym_eager_first_press_g, i.e., eagerly
 * report a single new keypress but debounce key releases and multiple changes
 * in the same report. However, an additional condition is to only eagerly
 * debounce when no keys are held down at the same time. This improves noise
 * immunity in case where holding keys causes crosstalk to other keys, but TBH
 * the practical benefit may be tiny since this means most keypresses while
 * gaming etc. will be debounced anyway. Thus the only benefit is more
 * responsiveness for typing single keys withouth holding down a modifier.
 */
#define EAGER_DEBOUNCE_ONLY_LONE_KEY
#include "sym_eager_first_press_g.c"
