#include <stdio.h>
#include <stdlib.h>
#include "disk_emulator.h"

static char * disk;
static int block_size;
static int inode_block_nb;
static int data_block_nb;

int allocate_disk(int blck_size, int inode_blck_nb, int data_blck_nb) {
	block_size = blck_size;
	inode_block_nb = inode_blck_nb;
	data_block_nb = data_blck_nb;

	disk = (char *) malloc((inode_block_nb + data_block_nb + 1) * block_size * sizeof(char));
	if (disk) {
		return 0;
	}
	return -1;
}

void unallocate_disk() {
	if (disk) {
		free(disk);
	}
}

int write_block(int block_id, char * buffer) {
	if ((block_id >= 0) && (block_id <= (inode_block_nb + data_block_nb)) {
		memcpy(disk + block_id, buffer, block_size);
		return 0;
	}
	return -1;
}

int read_block(int block_id, char * buffer) {
	if ((block_id >= 0) && (block_id <= (inode_block_nb + data_block_nb)) {
		memcpy(buffer, disk + block_id, block_size);
		return 0;
	}
	return -1;
}
