# define the C compiler to use
CC = gcc -Wall

# define global compile-time flags
CFLAGS = `pkg-config fuse --cflags --libs`

# define sources
SRCS = fstr.c blocks_handler.c disk_emulator.c

BIN_DIR = ../bin
OBJS = $(SRCS:.c=.o)
TARGET = fstr

$(TARGET): $(OBJS)
	mkdir -p $(BIN_DIR)
	$(CC) -o $(BIN_DIR)/$@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -rf $(BIN_DIR)
