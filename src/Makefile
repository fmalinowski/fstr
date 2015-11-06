# define the C compiler to use
CC = gcc -Wall

# define global compile-time flags
CFLAGS = `pkg-config fuse --cflags --libs`

# define sources
SRCS = fstr.c disk_emulator.c common.c data_blocks_handler.c mkfs.c inodes_handler.c

BIN_DIR = ../bin
OBJS = $(SRCS:.c=.o)
TARGET = fstr

$(TARGET): $(OBJS)
	mkdir -p $(BIN_DIR)
	$(CC) -o $(BIN_DIR)/$@ $^ $(CFLAGS)
	rm -f $^

.PHONY: clean

clean:
	rm -rf $(OBJS) $(BIN_DIR)
