## aakbd/lib

This subdirectory of [AAKBD](https://github.com/arkku/aakbd) contains
unmodified vendor and third-party libraries, each with its own license:

* [CMSIS](CMSIS/) — ARM Cortex Microcontroller Software Interface Standard
  headers
* [STM32F3xx](STM32F3xx/) — STM32F3 standard peripheral library headers
* [tinyusb](tinyusb/) — TinyUSB embedded USB stack.

Run `git submodule update --init` to actually populate the git
submodules. (Currently only needed on ARM-based keyboards.)
