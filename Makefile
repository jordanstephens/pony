CC                       = cc
CFLAGS                   = -g -std=gnu17 -fPIC -Wall -Wextra
LDFLAGS                  = -lm
OUTPUT_OPTION            = -MMD -MP -o $@
SHARED_LIBRARY_EXTENSION = dylib
SHARED_LIBRARY_FLAG      = -dynamiclib

BUILD_DIR := ./build
SRC_DIRS := ./src
TEST_DIRS := ./test

SRCS := $(shell find $(SRC_DIRS) -name *.c)
OBJS := $(SRCS:.c=.o)

default: help

all: check lib test

lib: $(BUILD_DIR)/libpony.${SHARED_LIBRARY_EXTENSION}(${OBJS})

$(BUILD_DIR)/libpony.${SHARED_LIBRARY_EXTENSION}: ${OBJS}
	${CC} ${SHARED_LIBRARY_FLAG} -o $@ $^

check:
	cppcheck --suppress='*:include/*' --force ${SRCS} ${TEST_SRCS}

TEST_SRCS := $(shell find $(TEST_DIRS) -name *.c)
TEST_OBJS := $(TEST_SRCS:.c=.o)

$(BUILD_DIR)/test: ${TEST_OBJS}
	${CC} -Lbuild -lpony -o $@ $^

build_test: lib $(BUILD_DIR)/test

test: build_test
	rm -rf tmp/*
	./build/test

format:
	clang-format -i src/*.{h,c}

BENCH_SRCS := $(shell find ./bench -name *.c)
BENCH_OBJS := $(BENCH_SRCS:.c=.o)

build_bench: lib $(BUILD_DIR)/bench

bench: build_bench
	mkdir -p tmp/bench
	rm -rf tmp/bench/*
	./build/bench -c 8192 > tmp/bench/timings.tsv
	./bench/plot.sh

$(BUILD_DIR)/bench: ${BENCH_OBJS}
	${CC} -Lbuild -lpony -o $@ $^

clean:
	-rm -rf $(BUILD_DIR)/* $(SRC_DIRS)/*.o $(SRC_DIRS)/*.d $(TEST_DIRS)/*.o $(TEST_DIRS)/*.d $(TEST_DIRS)/*.o bench/*.o bench/*.d

help:
	-@echo "make lib:    build libpony.${SHARED_LIBRARY_EXTENSION}"
	-@echo "make test:   run unit tests"
	-@echo "make check:  run cppcheck"
	-@echo "make bench:  run benchmarks"
	-@echo "make format: run clang-format -i"
	-@echo "make clean:  remove all build files"

.PHONY: help test clean all bench format
