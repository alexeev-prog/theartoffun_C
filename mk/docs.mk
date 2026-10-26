DOXYGEN ?= doxygen

.PHONY: docs
docs:
	@mkdir -p build/docs
	$(DOXYGEN) Doxyfile
