CXX      ?= g++
CC       ?= gcc
OPT      ?= -O2
CXXFLAGS := $(OPT) -std=c++17 -Wall -Wextra -Iinclude
CFLAGS   := $(OPT) -Wall -Wextra

BUILD := build

all: $(BUILD)/benchmark $(BUILD)/matmul_basic

$(BUILD)/benchmark: bench/benchmark.cpp src/matmul_kernels.cpp include/matmul.h | $(BUILD)
	$(CXX) $(CXXFLAGS) bench/benchmark.cpp src/matmul_kernels.cpp -o $@

$(BUILD)/matmul_basic: src/matmul_basic.c | $(BUILD)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD):
	mkdir -p $@

run: $(BUILD)/benchmark
	./$(BUILD)/benchmark $(SIZE)

clean:
	rm -rf $(BUILD)

.PHONY: all run clean
