#include "disk_emulator.h"

static char **block_data;

int init_disk_emulator(void) { // just for in memory simulation; substitue code for opening disk here
	if (block_data) {
		return -1; // Disk already allocated
	}

	block_data = malloc(NUM_BLOCKS * sizeof(char*));
	int i;
	for (i = 0; i < NUM_BLOCKS; i++) {
		block_data[i] = malloc(BLOCK_SIZE * sizeof(char));
	}

	// If malloc didn't work well
	if (block_data) {
		return 0;
	}
	return -1;
}

void free_disk_emulator(void) { // just for in memory simulation; substitue code for closing disk here
	if (block_data) {
		int i;
		for (i = 0; i < NUM_BLOCKS; i++) {
			free(block_data[i]);
		}
		free(block_data);
		block_data = NULL;
	}
}

int read_block(big_int block_id, void * target) {
	if ((block_id >= 0) && (block_id < NUM_BLOCKS)) {
		memcpy(target, block_data[block_id], BLOCK_SIZE);
		return 0;
	}
	return -1;
}

int write_block(big_int block_id, void * buffer, size_t buffer_size) {
	if ((block_id >= 0) && (block_id < NUM_BLOCKS)) {

		size_t copy_size = buffer_size < BLOCK_SIZE ? buffer_size : BLOCK_SIZE;
		memcpy(block_data[block_id], buffer, copy_size);
		if(copy_size < BLOCK_SIZE) {
			memset(block_data[block_id] + copy_size, 0, BLOCK_SIZE - copy_size);
		}
		return 0;
	}
	return -1;
}

