.DEFAULT_GOAL := all

include mk/config.mk
include mk/build.mk
include mk/test.mk
include mk/docs.mk
include mk/examples.mk
include mk/benchmarks.mk

.PHONY: all run clean help
all: $(TARGET)

run: $(TARGET)
	$(TARGET) $(ARGS)

clean:
	$(RM) -r -- build bin

help:
	@echo 'make [all]       Build bin/theartoffun'
	@echo 'make run ARGS="--help"'
	@echo 'make test        Unit/property tests and CLI integration tests'
	@echo 'make test-unit   C unit/property tests only (auto-discovered)'
	@echo 'make list-tests  List available C tests'
	@echo 'make run-test TEST=test_algorithms'
	@echo 'make check       Tests, supported examples, and checked CLI tour'
	@echo 'make sanitize    Tests with AddressSanitizer and UBSan'
	@echo 'make docs        Doxygen HTML in build/docs/html'
	@echo 'make examples    Build finite educational examples'
	@echo 'make list-examples  List supported examples'
	@echo 'make run-example EXAMPLE=mortoncode ARGS="10 13"'
	@echo 'make test-examples  Build and smoke-test all supported examples'
	@echo 'make demo        Run a checked CLI tour'
	@echo 'make benchmark   Collect CSV samples (BENCH_ARGS="--samples 7 --sizes 64 256")'
	@echo 'make benchmark-list  List benchmark algorithms'
	@echo 'make benchmark-plots BENCHMARK_INPUT=results/benchmarks/RUN PLOT_OUTPUT=results/plots/RUN'
	@echo 'make test-benchmarks  Validate benchmark collection and offline plotting'
	@echo 'make clean       Remove generated build output'
	@echo 'make clean-results  Delete all saved runs and reports in results/'
	@echo 'Options: CC=clang DEBUG=1 BUILD_DIR=build/custom BIN_DIR=bin/custom'
