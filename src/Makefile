# define the C compiler to use
CC = gcc -Wall

# define global compile-time flags
CFLAGS = -g `pkg-config fuse --cflags --libs`

# define sources
FSTR_SRCS = fstr.c mkfs.c common.c block_utils.c disk_emulator.c data_blocks_handler.c inodes_handler.c inode_table.c namei.c syscalls1.c syscalls2.c

BIN_DIR = ../bin
FSTR_OBJS = $(FSTR_SRCS:.c=.o)
FSTR_TARGET = fstr

$(FSTR_TARGET): $(FSTR_OBJS)
	mkdir -p $(BIN_DIR)
	$(CC) -o $(BIN_DIR)/$@ $^ $(CFLAGS)
	rm -f $^

.PHONY: clean

clean:
	rm -rf $(FSTR_OBJS) $(BIN_DIR)
