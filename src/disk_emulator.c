#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include "disk_emulator.h"

static char ** block_data;

int init_disk_emulator(void) {
	if (block_data) {
		return -1; // Disk already allocated
	}

	block_data = malloc(TOTAL_BLOCKS * sizeof(char*));
	for (int i = 0; i < TOTAL_BLOCKS; i++) {
		block_data[i] = malloc(DATA_BLOCK_SIZE * sizeof(char));
	}

	// If malloc didn't work well
	if (block_data) {
		return 0;
	}
	return -1;
}

void free_disk_emulator(void) {
	if (block_data) {
		for (int i = 0; i < TOTAL_BLOCKS; i++) {
			free(block_data[i]);
		}
		free(block_data);
		block_data = NULL;
	}
}

int read_block(int block_id, void * target) {
	if ((block_id >= 0) && (block_id < TOTAL_BLOCKS)) {
		memcpy(target, block_data[block_id], DATA_BLOCK_SIZE);
		return 0;
	}
	return -1;
}

int write_block(int block_id, void * buffer, size_t buffer_size) {
	if ((block_id >= 0) && (block_id < TOTAL_BLOCKS)) {

		size_t copy_size = buffer_size < DATA_BLOCK_SIZE ? buffer_size : DATA_BLOCK_SIZE;
		memcpy(block_data[block_id], buffer, copy_size);
		return 0;
	}
	return -1;
}

