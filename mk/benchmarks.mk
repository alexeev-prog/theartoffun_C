BENCHMARK_TARGET := $(BIN_DIR)/benchmark
BENCHMARK_OUTPUT ?= results/benchmarks
PLOT_OUTPUT ?= results/plots
BENCHMARK_CONFIG ?= benchmarks/config.json
BENCH_PROFILE ?=

.PHONY: benchmark benchmark-report benchmark-build benchmark-list benchmark-plots benchmark-compare test-benchmarks clean-results
clean-results:
	$(RM) -r -- results

benchmark-build: $(BENCHMARK_TARGET)

$(BUILD_DIR)/benchmark-runner.o: benchmarks/runner.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -fno-lto -fno-builtin-pow -fno-builtin-sqrtf -MMD -MP -c $< -o $@

$(BUILD_DIR)/benchmark-examples.o: benchmarks/example_algorithms.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -fno-lto -fno-inline -MMD -MP -c $< -o $@

$(BENCHMARK_TARGET): $(BUILD_DIR)/benchmark-runner.o $(BUILD_DIR)/benchmark-examples.o $(LIB_OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) -fno-lto $^ $(LDLIBS) -o $@

benchmark: benchmark-build
	BENCH_CC='$(CC)' BENCH_CFLAGS='$(CFLAGS)' BENCH_CPPFLAGS='$(CPPFLAGS)' python3 benchmarks/run.py --runner "$(BENCHMARK_TARGET)" --output "$(BENCHMARK_OUTPUT)" --config "$(BENCHMARK_CONFIG)" $(if $(BENCH_PROFILE),--profile "$(BENCH_PROFILE)",) $(BENCH_ARGS)

benchmark-report: benchmark-build
	BENCH_CC='$(CC)' BENCH_CFLAGS='$(CFLAGS)' BENCH_CPPFLAGS='$(CPPFLAGS)' python3 benchmarks/run.py --runner "$(BENCHMARK_TARGET)" --output "$(BENCHMARK_OUTPUT)" --report-output "$(PLOT_OUTPUT)" --config "$(BENCHMARK_CONFIG)" $(if $(BENCH_PROFILE),--profile "$(BENCH_PROFILE)",) $(BENCH_ARGS)

benchmark-list: benchmark-build
	$(BENCHMARK_TARGET) --list

# Intentionally no build/measurement prerequisite: plotting is an offline step.
benchmark-plots:
	@test -n "$(BENCHMARK_INPUT)" || { echo 'Set BENCHMARK_INPUT to a saved run directory' >&2; exit 2; }
	python3 benchmarks/plot.py --input "$(BENCHMARK_INPUT)" --output "$(PLOT_OUTPUT)"

benchmark-compare:
	@test -n "$(BENCHMARK_INPUT)" || { echo 'Set BENCHMARK_INPUT to current run' >&2; exit 2; }
	@test -n "$(BENCHMARK_BASELINE)" || { echo 'Set BENCHMARK_BASELINE to baseline run' >&2; exit 2; }
	python3 benchmarks/compare.py --current "$(BENCHMARK_INPUT)" --baseline "$(BENCHMARK_BASELINE)" $(BENCH_COMPARE_ARGS)

test-benchmarks: benchmark-build
	python3 tests/test_benchmarks.py "$(BENCHMARK_TARGET)"

-include $(BUILD_DIR)/benchmark-runner.d
-include $(BUILD_DIR)/benchmark-examples.d
