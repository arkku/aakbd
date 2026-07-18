# Makefile for Vial unit tests
CC = gcc
CFLAGS = -g -DTESTING -I.. -I. -I$(BUILD_DIR) -I../qmk_core/platforms/mock -I../arch/arm -I../qmk_core -std=gnu11 -Wall -Wno-unused-parameter
LDFLAGS = -lm
BUILD_DIR = build
GEN_RUNNER = ./generate_test_runner.sh

TRANSLATE_BIN = test_keycode_translate.bin
TRANSLATE_SRC = test_keycode_translate.c
TRANSLATE_RUNNER = $(BUILD_DIR)/translate_runner.c
TRANSLATE_DEPS = qmk_translate.c
TRANSLATE_HDRS = qmk_translate.h qmk_keycodes.h vial_magic.h \
                  ../keycodes.h ../usb_keys.h

TRANSLATE_EP_BIN = test_keycode_translate_ep.bin
TRANSLATE_EP_FLAGS = -DMEDIA_KEYS_ENDPOINT=1 -DMEDIA_KEYS_COUNT=22

KEYS_BIN = test_keys.bin
KEYS_SRC = test_keys.c
KEYS_RUNNER = $(BUILD_DIR)/keys_runner.c
KEYS_DEPS = ../keys.c qmk_translate.c dynamic_keymap.c vial_magic.c vial.c \
            test_layers.c test_macros.c
KEYS_HDRS = vial.h vial_keys.h vial_config.h dynamic_keymap.h \
            qmk_translate.h qmk_keycodes.h vial_magic.h \
            ../keycodes.h ../usb_keys.h ../usbkbd_config.h \
            ../usbkbd.h ../usb.h ../keys.h ../aakbd.h ../layers.h \
            ../macros.h ../usb_hardware.h

CONSUMER_BIN = test_consumer.bin
CONSUMER_8_BIN = test_consumer_8.bin
CONSUMER_8_FLAGS = -DMEDIA_KEYS_ENDPOINT=1 -DMEDIA_KEYS_COUNT=8 -DENABLE_MEDIA_KEYS=1
CONSUMER_SRC = test_consumer.c
CONSUMER_DEPS = ../usbkbd.c ../usb_keys.h ../usbkbd_config.h
CONSUMER_RUNNER = $(BUILD_DIR)/consumer_runner.c

.PHONY: all test tests clean distclean format coverage coverage-clean

all: test

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TRANSLATE_RUNNER): $(TRANSLATE_SRC) $(GEN_RUNNER) | $(BUILD_DIR)
	$(GEN_RUNNER) $< > $@

$(TRANSLATE_BIN): $(TRANSLATE_SRC) $(TRANSLATE_RUNNER) $(TRANSLATE_DEPS) $(TRANSLATE_HDRS)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

TRANSLATE_EP_RUNNER = $(BUILD_DIR)/translate_ep_runner.c
$(TRANSLATE_EP_RUNNER): $(TRANSLATE_SRC) $(GEN_RUNNER) | $(BUILD_DIR)
	$(GEN_RUNNER) $< > $@

$(TRANSLATE_EP_BIN): $(TRANSLATE_SRC) $(TRANSLATE_EP_RUNNER) $(TRANSLATE_DEPS) $(TRANSLATE_HDRS)
	$(CC) $(CFLAGS) $(TRANSLATE_EP_FLAGS) -o $@ $< $(LDFLAGS)

$(KEYS_RUNNER): $(KEYS_SRC) $(GEN_RUNNER) | $(BUILD_DIR)
	$(GEN_RUNNER) $< > $@

$(KEYS_BIN): $(KEYS_SRC) $(KEYS_RUNNER) $(KEYS_DEPS) $(KEYS_HDRS)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(CONSUMER_RUNNER): $(CONSUMER_SRC) $(GEN_RUNNER) | $(BUILD_DIR)
	$(GEN_RUNNER) $< > $@

$(CONSUMER_BIN): $(CONSUMER_SRC) $(CONSUMER_RUNNER) $(CONSUMER_DEPS)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(CONSUMER_8_BIN): $(CONSUMER_SRC) $(CONSUMER_RUNNER) $(CONSUMER_DEPS)
	$(CC) $(CFLAGS) $(CONSUMER_8_FLAGS) -o $@ $< $(LDFLAGS)

test: $(TRANSLATE_BIN) $(KEYS_BIN) $(CONSUMER_BIN) $(CONSUMER_8_BIN)
	@failed=0; \
	echo "=== Keycode translation tests ==="; ./$(TRANSLATE_BIN) || failed=1; \
	echo "=== Key processing tests ==="; ./$(KEYS_BIN) || failed=1; \
	echo "=== Consumer endpoint tests (usbkbd.c, 22 keys) ==="; ./$(CONSUMER_BIN) || failed=1; \
	echo "=== Consumer endpoint tests (usbkbd.c, 8 keys) ==="; ./$(CONSUMER_8_BIN) || failed=1; \
	exit $$failed

tests: $(TRANSLATE_BIN) $(KEYS_BIN) $(CONSUMER_BIN) $(CONSUMER_8_BIN)
	@failed=0; \
	echo "=== Keycode translation tests ==="; ./$(TRANSLATE_BIN) --verbose || failed=1; \
	echo "=== Key processing tests ==="; ./$(KEYS_BIN) --verbose || failed=1; \
	echo "=== Consumer endpoint tests (usbkbd.c, 22 keys) ==="; ./$(CONSUMER_BIN) --verbose || failed=1; \
	echo "=== Consumer endpoint tests (usbkbd.c, 8 keys) ==="; ./$(CONSUMER_8_BIN) --verbose || failed=1; \
	exit $$failed

COVERAGE_FLAGS = --coverage -O0
COVERAGE_DIR = $(BUILD_DIR)/coverage

COVERAGE_BINS = \
	$(COVERAGE_DIR)/$(TRANSLATE_BIN) \
	$(COVERAGE_DIR)/$(TRANSLATE_EP_BIN) \
	$(COVERAGE_DIR)/$(KEYS_BIN) \
	$(COVERAGE_DIR)/$(CONSUMER_BIN) \
	$(COVERAGE_DIR)/$(CONSUMER_8_BIN)

coverage: coverage-clean $(COVERAGE_BINS)
	@failed=0; \
	echo "=== Keycode translation tests ==="; \
	$(COVERAGE_DIR)/$(TRANSLATE_BIN) || failed=1; \
	echo "=== Keycode translation endpoint tests ==="; \
	$(COVERAGE_DIR)/$(TRANSLATE_EP_BIN) || failed=1; \
	echo "=== Key processing tests ==="; \
	$(COVERAGE_DIR)/$(KEYS_BIN) || failed=1; \
	echo "=== Consumer endpoint tests (22 keys) ==="; \
	$(COVERAGE_DIR)/$(CONSUMER_BIN) || failed=1; \
	echo "=== Consumer endpoint tests (8 keys) ==="; \
	$(COVERAGE_DIR)/$(CONSUMER_8_BIN) || failed=1; \
	exit $$failed
	@echo
	@echo "=== Coverage ==="
	@rm -f *.gcov
	@for data in $(COVERAGE_DIR)/*.gcda; do \
		[ -e "$$data" ] || continue; \
		gcov -b -c -o $(COVERAGE_DIR) "$$data" > /dev/null; \
	done
	@for src in \
		qmk_translate.c \
		dynamic_keymap.c \
		vial_keys.c \
		vial_magic.c \
		../keys.c; do \
		f="$$(basename "$$src").gcov"; \
		[ -f "$$f" ] || { \
			echo "WARNING: no report generated for $$src"; \
			continue; \
		}; \
		total=$$(grep -Ec '^[[:space:]]*([0-9]+|#####|=====):[[:space:]]*[0-9]+:' "$$f" || true); \
		uncovered=$$(grep -Ec '^[[:space:]]*(#####|=====):[[:space:]]*[0-9]+:' "$$f" || true); \
		if [ "$$total" -eq 0 ]; then \
			echo "WARNING: no executable lines found for $$src"; \
			continue; \
		fi; \
		covered=$$((total - uncovered)); \
		pct=$$(awk 'BEGIN { printf "%.2f", 100 * '"$$covered"' / '"$$total"' }'); \
		printf '\n%s: %s%%\n' "$$src" "$$pct"; \
		grep -E '^[[:space:]]*(#####|=====):[[:space:]]*[0-9]+:' "$$f" \
			| sed -E 's/^[[:space:]]*(#####|=====):[[:space:]]*//'; \
	done

coverage-clean:
	rm -rf $(COVERAGE_DIR)

$(COVERAGE_DIR):
	mkdir -p $@

$(COVERAGE_DIR)/$(TRANSLATE_BIN): $(TRANSLATE_SRC) $(TRANSLATE_RUNNER) \
		$(TRANSLATE_DEPS) $(TRANSLATE_HDRS) | $(COVERAGE_DIR)
	$(CC) $(CFLAGS) $(COVERAGE_FLAGS) -o $@ $< $(LDFLAGS) $(COVERAGE_FLAGS)

$(COVERAGE_DIR)/$(TRANSLATE_EP_BIN): $(TRANSLATE_SRC) $(TRANSLATE_EP_RUNNER) \
		$(TRANSLATE_DEPS) $(TRANSLATE_HDRS) | $(COVERAGE_DIR)
	$(CC) $(CFLAGS) $(COVERAGE_FLAGS) $(TRANSLATE_EP_FLAGS) \
		-o $@ $< $(LDFLAGS) $(COVERAGE_FLAGS)

$(COVERAGE_DIR)/$(KEYS_BIN): $(KEYS_SRC) $(KEYS_RUNNER) \
		$(KEYS_DEPS) $(KEYS_HDRS) | $(COVERAGE_DIR)
	$(CC) $(CFLAGS) $(COVERAGE_FLAGS) -o $@ $< $(LDFLAGS) $(COVERAGE_FLAGS)

$(COVERAGE_DIR)/$(CONSUMER_BIN): $(CONSUMER_SRC) $(CONSUMER_RUNNER) \
		$(CONSUMER_DEPS) | $(COVERAGE_DIR)
	$(CC) $(CFLAGS) $(COVERAGE_FLAGS) -o $@ $< $(LDFLAGS) $(COVERAGE_FLAGS)

$(COVERAGE_DIR)/$(CONSUMER_8_BIN): $(CONSUMER_SRC) $(CONSUMER_RUNNER) \
		$(CONSUMER_DEPS) | $(COVERAGE_DIR)
	$(CC) $(CFLAGS) $(COVERAGE_FLAGS) $(CONSUMER_8_FLAGS) \
		-o $@ $< $(LDFLAGS) $(COVERAGE_FLAGS)

clean: coverage-clean
	rm -rf $(BUILD_DIR) *.bin *.gcda *.gcno *.gcov

format:
	clang-format --style=file -i $(KEYS_SRC) $(TRANSLATE_SRC) $(CONSUMER_SRC)

distclean: clean
	$(MAKE) -C .. distclean
