SUBDIRS = external src tests
FORMAT_DIRS := src tests/scripts

.PHONY: format
format:
	clang-format -i $(shell find $(FORMAT_DIRS) -name "*.c" -o -name "*.h" -o -name "*.cpp" -o -name "*.hpp")

.PHONY: docs
docs:
	cd $(srcdir)/docs && \
	mkdir -p doxygen/xml && \
	doxygen Doxyfile && \
	sphinx-build -b html . build
