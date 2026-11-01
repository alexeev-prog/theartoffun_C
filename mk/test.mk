TEST_SOURCES := $(wildcard tests/test_*.c)
TEST_BINS := $(patsubst tests/%.c,$(BUILD_DIR)/tests/%,$(TEST_SOURCES))

.PHONY: test test-unit list-tests run-test check sanitize
test: test-unit $(TARGET)
	python3 tests/test_cli.py $(TARGET)

test-unit: $(TEST_BINS)
	@set -e; for test in $(TEST_BINS); do "$$test"; done

list-tests:
	@printf '%s\n' $(patsubst tests/%.c,%,$(TEST_SOURCES))

ifneq ($(filter $(TEST),$(patsubst tests/%.c,%,$(TEST_SOURCES))),)
run-test: $(BUILD_DIR)/tests/$(TEST)
	$(BUILD_DIR)/tests/$(TEST)
else
run-test:
	@echo 'Choose TEST from make list-tests (for example TEST=test_algorithms)' >&2
	@exit 2
endif

check: test test-examples demo

$(BUILD_DIR)/tests/%: tests/%.c $(LIB_OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP $(LDFLAGS) $< $(LIB_OBJECTS) $(LDLIBS) -o $@

sanitize:
	ASAN_OPTIONS=halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 $(MAKE) SANITIZE=1 test

-include $(TEST_BINS:=.d)
