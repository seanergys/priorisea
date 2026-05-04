SUBDIRS = src tests/integration

.PHONY: format

format:
	clang-format -i $(shell find $(SUBDIRS) -name "*.c" -o -name "*.h" -o -name "*.cpp" -o -name "*.hpp")
