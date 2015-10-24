# define the C compiler to use
CC = gcc -Wall

# define global compile-time flags
CFLAGS = `pkg-config fuse --cflags --libs`

# define sources
SRCS = fstr.c blocks_handler.c disk_emulator.c

BUILD_DIR = ../build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin
OBJS = $(SRCS:.c=.o)
TARGET = fstr

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(TARGET) $(OBJS) core
