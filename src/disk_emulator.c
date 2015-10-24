#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include "disk_emulator.h"

static void** BLOCK_DATA;

void init() {
	if(BLOCK_DATA == NULL) {
		BLOCK_DATA = malloc(sizeof(void*)*TOTAL_BLOCKS);
	}
}

int read_block(int block_id, void** target) {
	init();

	void *p = BLOCK_DATA[block_id];

	if(p == NULL) {
		// No data found
		return -1;
	}

	*target = p;

	// Success
	return 0;
}

int write_block(int block_id, void* buffer) {
	init();

	void *p = BLOCK_DATA[block_id];

	if(p != NULL) {
		free(p);
	}

	p = malloc(sizeof(void*) * DATA_BLOCK_SIZE);
	memcpy(p, buffer, DATA_BLOCK_SIZE);

	// Success
	return 0;
}
