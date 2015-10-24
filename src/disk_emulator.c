#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include "disk_emulator.h"

static void* BLOCK_DATA[TOTAL_BLOCKS];

// Should be called before any disk operation
void disk_init() {
}

// Should be called at disk shutdown
void disk_destroy() {

	// Free in-memory emulated disk
	for(int i=0; i<TOTAL_BLOCKS; i++) {
		free(BLOCK_DATA[i]);
	}
}

/**
 * Reads data from the block specified by block_id,
 * allocates (or reuse from a pool) sufficient memory
 * and points the target pointer to this memory.
 *
 * Note: For in-memory emulation, we create a copy of data
 */
int read_block(int block_id, void** target) {

	void* copy = malloc(DATA_BLOCK_SIZE);

	void* data = BLOCK_DATA[block_id];
	if(data != NULL) {
		memcpy(copy, BLOCK_DATA[block_id], DATA_BLOCK_SIZE);
	}
	*target = copy;

	// Success
	return 0;
}

/**
 * Writes data at the block specified by block_id.
 * There's no need to make a copy of data because this
 * method will just write all the bytes to disk.
 * Caller doesn't need to create a copy of data before
 * calling this method.
 *
 * Note: For in-memory emulation, we create a copy of data
 */
int write_block(int block_id, void* buffer, int nbytes) {
	
	if(nbytes > DATA_BLOCK_SIZE) {
		// Data too big for a block
		// TODO: Throw a warning?
		nbytes = DATA_BLOCK_SIZE;
	}

	void* copy = malloc(DATA_BLOCK_SIZE);
	memcpy(copy, buffer, nbytes);
	BLOCK_DATA[block_id] = copy;

	// Success
	return 0;
}
