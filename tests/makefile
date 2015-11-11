
.PHONY: clean default tests

CC = gcc

CFLAGS += -Wall
CFLAGS += -g `pkg-config fuse --cflags --libs`

CFLAGS += -Wextra
CFLAGS += -Werror 
CFLAGS += -Wpointer-arith
CFLAGS += -Wcast-align
CFLAGS += -Wwrite-strings
CFLAGS += -Wswitch-default
CFLAGS += -Wunreachable-code
CFLAGS += -Winit-self
CFLAGS += -Wmissing-field-initializers
CFLAGS += -Wno-unknown-pragmas
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wundef
CFLAGS += -Wold-style-definition
CFLAGS += -Wmissing-prototypes
CFLAGS += -Wmissing-declarations
CFLAGS += -DUNITY_FIXTURES


INCLUDES = -I./../src -Iinclude -Iinclude/fixture

SRC_DIR = src
BIN_DIR = bin

TESTS = include/unity.c include/fixture/unity_fixture.c all_tests.c test_disk_emulator.c test_data_blocks_handler.c test_mkfs.c test_inodes_handler.c test_syscalls2.c
SRC_FILES_USED_IN_TESTS = ../src/disk_emulator.c ../src/data_blocks_handler.c ../src/common.c ../src/mkfs.c ../src/inodes_handler.c ../src/syscalls2.c

all: clean tests

tests:
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC_FILES_USED_IN_TESTS) $(TESTS) -o $(BIN_DIR)/tests
	$(BIN_DIR)/tests -v

clean:
	rm -rf $(BIN_DIR)/
	