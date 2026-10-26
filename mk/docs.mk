DOXYGEN ?= doxygen

.PHONY: docs
docs:
	$(DOXYGEN) Doxyfile
