CC = gcc -Wall
CFLAGS = `pkg-config fuse --cflags --libs`
SRC_DIR = src
BIN_DIR = bin

all:
	mkdir -p $(BIN_DIR)
	$(CC) $(SRC_DIR)/fstr.c $(CFLAGS) -o $(BIN_DIR)/fstr

clean:
	rm -rf $(BIN_DIR)