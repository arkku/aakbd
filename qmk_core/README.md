Quantum Mechanical Keyboard Firmware
====================================
This is a keyboard firmware library with some useful features for Atmel AVR and Cortex-M.

The full source code is available here: <https://github.com/qmk/qmk_firmware>

License
-------
**GPLv2** or later.

AAKBD Note
----------

AAKBD does not use QMK for the actual key processing, but parts of the firmware
(and the underlying [TMK firmware](../tmk_core/)) are included to ease porting
matrix-based keyboards from QMK to AAKBD. This is done mostly "for science",
since QMK is an established system that can do pretty much anything one could
ever want a keyboard to do, and has a large community and relatively
easy-to-use tooling going for it. I highly recommend using QMK instead of AAKBD
unless you happen to be of the very small minority that actually prefers to
define their keyboard by writing custom C code. In that case, AAKBD _may_ be
easier to start developing with, specifically because it is not as full of
features to begin with.
