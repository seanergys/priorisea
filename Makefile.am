SUBDIRS = external src tests/integration

.PHONY: format

format:
	clang-format -i $(shell find src -name "*.c" -o -name "*.h" -o -name "*.cpp" -o -name "*.hpp")
