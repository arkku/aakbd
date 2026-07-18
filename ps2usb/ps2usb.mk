include arch/avr/avr-common.mk

DEVICE_OBJS = kk_ps2_host.o ps2usb_keys.o
DEVICE_FLAGS += -DGENERIC_HID_REPORT_SIZE=8 -DGENERIC_HID_FEATURE_SIZE=1 -Ips2

# PS/2 pin configuration (override via local.mk or command line):
PS2_PORT ?= D
PS2_DATA_PIN ?= 1
PS2_CLK_PIN ?= 0
PS2_CLK_INT_NUM ?= 0
DEVICE_FLAGS += -DPS2_PORT=$(PS2_PORT) -DPS2_DATA_PIN=$(PS2_DATA_PIN) \
                -DPS2_CLK_PIN=$(PS2_CLK_PIN) \
                -DPS2_CLK_INT_NUM=$(PS2_CLK_INT_NUM)

MANUFACTURER ?= "AAKBD"
PRODUCT ?= "PS/2 Keyboard"

$(BUILDDIR)/ps2usb_keys.o: ps2usb_keys.h ps2_keys.h usb_keys.h
$(BUILDDIR)/ps2usb.o: ps2usb.h kk_ps2_host.h led.h avrtimer.h ps2usb_keys.h usbkbd.h usbkbd_config.h usb_hardware.h aakbd.h keys.h
$(BUILDDIR)/kk_ps2_host.o: ps2/kk_ps2_host.c ps2/kk_ps2_host.h ps2/kk_ps2.h ps2/kk_ps2_avr.h usbkbd_config.h $(COMMON_HEADERS)
