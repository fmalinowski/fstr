#include "common.h"
#include "disk_emulator.h"

int write_block_offset(big_int block_id, void *buffer, size_t buffer_size, int offset) {
	if(offset + buffer_size > BLOCK_SIZE) {
		fprintf(stderr, "Data too big for a block. offset: %d, buffer_size: %ld\n", offset, buffer_size);
		return -1;
	}

	char *target = malloc(sizeof(char) * BLOCK_SIZE);
	read_block(block_id, target);
	memcpy(target + offset, buffer, buffer_size);
	write_block(block_id, target, BLOCK_SIZE);
	free(target);

	return 0;
}
