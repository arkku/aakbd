DEVICE_OBJS = kk_ps2.o ps2usb_keys.o
DEVICE_FLAGS += -DGENERIC_HID_REPORT_SIZE=8 -DGENERIC_HID_FEATURE_SIZE=1

$(BUILDDIR)/ps2usb_keys.o: ps2usb_keys.h ps2_keys.h usb_keys.h
$(BUILDDIR)/ps2usb.o: kk_ps2.h led.h avrtimer.h ps2usb_keys.h usbkbd.h main.h keys.h
$(BUILDDIR)/kk_ps2.o: kk_ps2.h
