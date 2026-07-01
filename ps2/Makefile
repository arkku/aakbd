# Makefile for PS/2 device mode keyboard logic tests

#CC = gcc
CFLAGS = -DTESTING -I. -I.. -I$(BUILD_DIR) -std=gnu11 -Wall
LDFLAGS = -lm
TEST_BIN = ps2_output_test.bin
BUILD_DIR = build
RUNNER = $(BUILD_DIR)/test_runner.c
GEN_RUNNER = ./gen_test_runner.sh

TEST_SRC = ps2_output_test.c
TEST_DEPS = ps2_output.c usb2ps2_keys.c
TEST_HDRS = ps2_output.h usb2ps2_keys.h ps2_keys.h kk_ps2.h \
            ../usb_keys.h ../usbkbd_config.h ../progmem.h

DEVICE_TEST_BIN = kk_ps2_device_test.bin
DEVICE_TEST_SRC = kk_ps2_device_test.c
DEVICE_TEST_RUNNER = $(BUILD_DIR)/device_test_runner.c

.PHONY: all test tests device_test device_tests coverage clean distclean format

all: device_test test

test: $(TEST_BIN)
	@./$(TEST_BIN)

tests: $(TEST_BIN)
	@./$(TEST_BIN) --verbose

device_test: $(DEVICE_TEST_BIN)
	@./$(DEVICE_TEST_BIN)

device_tests: $(DEVICE_TEST_BIN)
	@./$(DEVICE_TEST_BIN) --verbose

coverage: $(TEST_BIN)
	@./$(TEST_BIN) 2>&1 || true
	@mkdir -p $(BUILD_DIR)
	@gcov -o $(TEST_BIN)-$(TEST_SRC) $(TEST_SRC) 2>/dev/null | grep 'ps2_output.c' -A1
	@uc=$$(grep '#####' ps2_output.c.gcov 2>/dev/null | grep -v '^        -:' | head -20); \
	  if [ -n "$$uc" ]; then \
	    echo "--- Uncovered executable lines (top 20): ---"; \
	    echo "$$uc"; \
	  else \
	    echo "--- All executable lines covered ---"; \
	  fi
	@mv -f *.gcov $(BUILD_DIR) 2>/dev/null || true

$(RUNNER): $(TEST_SRC) $(GEN_RUNNER)
	@mkdir -p $(BUILD_DIR)
	$(GEN_RUNNER) $(TEST_SRC) > $(RUNNER)

$(TEST_BIN): $(RUNNER) $(TEST_SRC) $(TEST_DEPS) $(TEST_HDRS)
	rm -f $(TEST_BIN)-$(TEST_SRC).gcda $(TEST_BIN)-$(TEST_SRC).gcno
	$(CC) $(CFLAGS) --coverage -o $@ $(TEST_SRC) $(LDFLAGS)

$(DEVICE_TEST_RUNNER): $(DEVICE_TEST_SRC) $(GEN_RUNNER)
	@mkdir -p $(BUILD_DIR)
	$(GEN_RUNNER) $(DEVICE_TEST_SRC) > $(DEVICE_TEST_RUNNER)

$(DEVICE_TEST_BIN): $(DEVICE_TEST_SRC) $(GEN_RUNNER) $(DEVICE_TEST_RUNNER) \
                    kk_ps2_device.c kk_ps2_device.h kk_ps2.h kk_ps2_avr.h ../usbkbd_config.h
	$(CC) $(CFLAGS) -o $@ $(DEVICE_TEST_SRC) $(LDFLAGS)

format:
	clang-format --style=file -i $(TEST_SRC) $(DEVICE_TEST_SRC)

clean:
	rm -rf $(BUILD_DIR) $(TEST_BIN) $(DEVICE_TEST_BIN) $(RUNNER) *.gcda *.gcno

distclean: clean
	$(MAKE) -C .. distclean DEVICE=ps2usb
