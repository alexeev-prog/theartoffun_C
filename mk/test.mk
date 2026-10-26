TEST_SOURCES := $(wildcard tests/test_*.c)
TEST_BINS := $(patsubst tests/%.c,$(BUILD_DIR)/tests/%,$(TEST_SOURCES))

.PHONY: test sanitize
test: $(TARGET) $(TEST_BINS)
	@set -e; for test in $(TEST_BINS); do "$$test"; done
	python3 tests/test_cli.py $(TARGET)

$(BUILD_DIR)/tests/%: tests/%.c $(LIB_OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP $(LDFLAGS) $< $(LIB_OBJECTS) $(LDLIBS) -o $@

sanitize:
	ASAN_OPTIONS=halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 $(MAKE) SANITIZE=1 test

-include $(TEST_BINS:=.d)
