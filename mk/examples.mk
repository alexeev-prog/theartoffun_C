# Only finite, non-interactive educational programs belong in this list.
EXAMPLE_NAMES := bitboard_example boyermoore bresenham_circle bresenham_line \
    fletcher32_checksum float_introspection floyd_warshall FTaH fuzzy_float_compare \
    gjf_example karatsuba kmp mortoncode pi_leibniz q_rsqrt secret tyche_example xor_fold xxtea_example
EXAMPLE_BINS := $(addprefix $(BIN_DIR)/examples/,$(EXAMPLE_NAMES))
SHARED_EXAMPLES := mortoncode fletcher32_checksum pi_leibniz

.PHONY: examples list-examples run-example test-examples demo
examples: $(EXAMPLE_BINS)

list-examples:
	@printf '%s\n' $(EXAMPLE_NAMES)

ifneq ($(filter $(EXAMPLE),$(EXAMPLE_NAMES)),)
run-example: $(BIN_DIR)/examples/$(EXAMPLE)
	$(BIN_DIR)/examples/$(EXAMPLE) $(ARGS)
else
run-example:
	@echo 'Choose EXAMPLE from make list-examples (for example EXAMPLE=mortoncode)' >&2
	@exit 2
endif

$(addprefix $(BIN_DIR)/examples/,$(SHARED_EXAMPLES)): $(BIN_DIR)/examples/%: examples/%.c $(LIB_OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP $(LDFLAGS) $< $(LIB_OBJECTS) $(LDLIBS) -o $@

$(BIN_DIR)/examples/%: examples/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP $(LDFLAGS) $< $(LDLIBS) -o $@

-include $(EXAMPLE_BINS:=.d)

test-examples: examples
	python3 tests/test_examples.py $(BIN_DIR)/examples $(EXAMPLE_NAMES)

demo: $(TARGET)
	python3 demos/tour.py $(TARGET)
