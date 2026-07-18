# avr-common.mk: Shared build rules for AVR MCU keyboards.
#
# Include this from your port .mk (e.g., xwhatsit_port.mk) or device .mk
# after setting device-specific variables.

ARCH = avr
CC = avr-gcc
OBJCOPY = avr-objcopy
SIZE = avr-size
AVRDUDE = avrdude

MCU ?= atmega32u4
F_CPU ?= 16000000UL
F_USB ?= $(F_CPU)
DFU_TARGET ?= dfuavr

AVR_FLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -DF_USB=$(F_USB)
CC_FLAGS += $(AVR_FLAGS) -fno-unroll-loops -mrelax
LD_FLAGS += $(AVR_FLAGS) -Wl,--gc-sections -Wl,--relax

PLATFORM_OBJS = avrusb.o
vpath %.c arch/avr
vpath %.h arch/avr

# AVR libc's default printf includes floating-point parsing (~1.5 KB).
# The minimal printf (%d, %u, %c, %s, %x) is sufficient for this firmware.
HEX ?= $(BIN:.bin=.hex)

$(HEX): $(BIN)
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

all: $(HEX)

# AVR link rule (prerequisites added by root rule after device .mk is included)
$(BIN): | $(BUILDDIR)
	@echo $(CC) $(LDFLAGS) -s -o $@ ...
	@$(CC) $(LDFLAGS) -s -o $@ $+ $(AVR_LIBS)
	@chmod a-x $@
	@$(SIZE) $@
	@boot=0; \
	 for f in $(DEVICE_FLAGS); do \
	   case "$$f" in -DBOOTLOADER_SIZE=*) boot=$${f#-DBOOTLOADER_SIZE=} ;; esac; \
	 done; \
	 case '$(MCU)' in \
	   atmega32u2|atmega32u4|atmega328p) total=32768 ;; \
	   atmega16u2) total=16384 ;; \
	   at90usb1286) total=131072 ;; \
	   at90usb646) total=65536 ;; \
	   *) total=0 ;; \
	 esac; \
	 [ "$$total" -gt 0 ] && { \
	   used=$$($(SIZE) $@ | tail -1 | awk '{print $$1+$$2}'); \
	   avail=$$((total - boot)); \
	   free=$$((avail - used)); \
	   echo "  $$used of $$avail bytes used ($$free bytes free)"; \
	   if [ "$$free" -lt 0 ]; then \
	     echo "  FIRMWARE_OVERFLOW: exceeds flash by $$((-free)) bytes"; \
	     exit 1; \
	   fi; \
	}

# LFUSE configures the clock speed; it is probably correct out of the box if
# you use a ready-made board. If not, `make fuses LFUSE=CE` for a 16 MHz
# crystal oscillator.
#LFUSE ?= CE
HFUSE ?= D0
EFUSE ?= FB
#EFUSE ?= F4 # HWBEN

#### BURNER ###################################################################
# Specify the burner on the command-line if you wish, e.g.,
#	make burn BURNER=avrisp2 PORT=/dev/ttyUSB0 BPS=115200

# Burner device
BURNER ?= dragon_isp
# Burner port
#PORT ?= /dev/ttyUSB0
# Burner speed
#BPS ?= 115200

# Protocol for the bootloader, e.g., make upload PORT=/dev/cu.usbmodem1401
UPLOAD_PROTOCOL ?= avr109
###############################################################################

burn: $(HEX)
	$(SUDO) $(AVRDUDE) -c $(BURNER) $(if $(PORT),-P $(PORT) ,)$(if $(BPS),-b $(BPS) ,)-p $(MCU) -U flash:w:$< -v

upload: $(HEX)
	$(SUDO) $(AVRDUDE) -c $(UPLOAD_PROTOCOL) $(if $(PORT),-P $(PORT),-P /dev/ttyACM0) $(if $(BPS),-b $(BPS),) -p $(MCU) -U flash:w:$< -v

dfuavr: $(HEX)
	$(SUDO) dfu-util -e && sleep 2 || true
	$(SUDO) dfu-programmer $(MCU) erase
	$(SUDO) dfu-programmer $(MCU) flash $<
	$(SUDO) dfu-programmer $(MCU) launch || $(SUDO) dfu-programmer $(MCU) reset

teensy: $(HEX)
	$(SUDO) dfu-util -e && sleep 2 || true
	$(SUDO) teensy_loader_cli -v -w -mmcu=$(MCU) $<

fuses:
	$(SUDO) $(AVRDUDE) -c $(BURNER) $(if $(PORT),-P $(PORT) ,)$(if $(BPS),-b (BPS) ,)-p $(MCU) -U efuse:w:0x$(EFUSE):m -U hfuse:w:0x$(HFUSE):m $(if $(LFUSE),-U lfuse:w:0x$(LFUSE):m,)

unlock:
	$(SUDO) $(AVRDUDE) -c $(BURNER) $(if $(PORT),-P $(PORT) ,)$(if $(BPS),-b $(BPS) ,)-p $(MCU) -U lock:w:0x3F:m -v

lock:
	$(SUDO) $(AVRDUDE) -c $(BURNER) $(if $(PORT),-P $(PORT) ,)$(if $(BPS),-b $(BPS) ,)-p $(MCU) -U lock:w:0x0F:m -v

.ccls: Makefile local.mk $(DEVICE)/local.mk
	@echo $(CC) >$@
	@echo --target=avr >>$@
	@echo $(CFLAGS) | awk '{ gsub(/["][^"]*["]/, "\"!SKIP!\""); for (i=1; i<=NF; i++) { if (!($$i ~ /!SKIP!/)) print $$i } }' >>$@
	@echo -nostdinc >>$@
	@[ -d /usr/lib/avr/include ] && echo -I/usr/lib/avr/include >>$@ || true
	@[ -d /usr/local/avr/include ] && echo -I/usr/local/avr/include >>$@ || true
	@echo | $(CC) $(CFLAGS) -E -Wp,-v - 2>&1 | awk '/#include .* search starts here:/ { output=1; next } !output { next } /^End/ || /^#/ { output=0 } output && $$1 ~ /^\// { sub(/^[ ]*/, ""); print "-I" $$0 }' >>$@
	@echo -Wno-attributes >>$@
	@echo -Wno-gnu-zero-variadic-macro-arguments >>$@
	@cat $@

.clangd: Makefile local.mk $(DEVICE)/local.mk
	@printf 'CompileFlags:\n  Compiler: %s\n  Add:\n' "$(CC)" >$@
	@echo '    - -xc' >>$@
	@echo $(CFLAGS) | awk '{ gsub(/["][^"]*["]/, "\"!SKIP!\""); for (i=1; i<=NF; i++) { if (!($$i ~ /!SKIP!/ || $$i == "-mcall-prologues")) printf "    - %s\n", $$i } }' >>$@
	@echo '    - --target=avr' >>$@
	@echo '    - -nostdinc' >>$@
	@[ -d /usr/lib/avr/include ] && printf '    - -I/usr/lib/avr/include\n' >>$@ || true
	@[ -d /usr/local/avr/include ] && printf '    - -I/usr/local/avr/include\n' >>$@ || true
	@echo | $(CC) $(CFLAGS) -E -Wp,-v - 2>&1 | awk '/#include .* search starts here:/ { output=1; next } !output { next } /^End/ || /^#/ { output=0 } output && $$1 ~ /^\// { sub(/^[ ]*/, ""); printf "    - -I%s\n", $$0 }' >>$@
	@echo '    - -Wno-attributes' >>$@
	@echo '    - -Wno-gnu-zero-variadic-macro-arguments' >>$@
	@cat $@

.PHONY: burn upload dfuavr teensy fuses unlock lock
