.DEFAULT_GOAL := all

include mk/config.mk
include mk/build.mk
include mk/test.mk
include mk/docs.mk

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
	@echo 'make sanitize    Tests with AddressSanitizer and UBSan'
	@echo 'make docs        Doxygen HTML in build/docs/html'
	@echo 'make clean       Remove generated build output'
	@echo 'Options: CC=clang DEBUG=1 BUILD_DIR=build/custom BIN_DIR=bin/custom'
