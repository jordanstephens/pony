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

default: lib

lib: $(BUILD_DIR)/libpony(${OBJS})

$(BUILD_DIR)/libpony.${SHARED_LIBRARY_EXTENSION}: ${OBJS}
	${CC} ${SHARED_LIBRARY_FLAG} -o $@ $^

TEST_SRCS := $(shell find $(TEST_DIRS) -name *.c)
TEST_OBJS := $(TEST_SRCS:.c=.o)

test: $(BUILD_DIR)/test
	./build/test

$(BUILD_DIR)/test: ${TEST_OBJS} ${OBJS}
	${CC} -o $@ $^

clean:
	-rm -rf $(BUILD_DIR)/* $(SRC_DIRS)/*.o $(SRC_DIRS)/*.d $(TEST_DIRS)/*.o $(TEST_DIRS)/*.d $(TEST_DIRS)/*.o

help:
	-@echo "make lib:   build libpony.${SHARED_LIBRARY_EXTENSION}"
	-@echo "make test:  run unit tests"
	-@echo "make clean: remove all build files"

.PHONY: help test clean