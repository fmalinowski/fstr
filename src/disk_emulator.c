#include "disk_emulator.h"
#include "mkfs.h"
#include "common.h"
#include "inode_table.h"
// in memory emulation of file system
// #define IN_MEMORY_FS


#ifdef IN_MEMORY_FS

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
		// Only for in-memory emulation
		return create_fs();
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
		purge_inode_table();
	}
}

int read_block(big_int block_id, void * target) {
	if (block_id < NUM_BLOCKS) {
		memcpy(target, block_data[block_id], BLOCK_SIZE);
		return 0;
	}
	return -1;
}

int write_block(big_int block_id, void * buffer, size_t buffer_size) {
	if (block_id < NUM_BLOCKS) {

		size_t copy_size = buffer_size < BLOCK_SIZE ? buffer_size : BLOCK_SIZE;
		memcpy(block_data[block_id], buffer, copy_size);
		if(copy_size < BLOCK_SIZE) {
			memset(block_data[block_id] + copy_size, 0, BLOCK_SIZE - copy_size);
		}
		return 0;
	}
	return -1;
}

#else

static int disk_store;
int disk_created = -1;

int init_disk_emulator(void) { 
	
	if(disk_created == -1){
		disk_store = open(DISK_STORE_PATH, O_RDWR);
		if (disk_store == -1){
    		fprintf(stderr, "Error opening disk store\n");
    		errno = ENODEV;
    		return -1; // failure
  		}
  		LOGD("file descriptor of disk: %d", disk_store);
		disk_created = 0;
		return 0; // success
	}

	LOGD("disk store already initialised");
	return -1;
}

void free_disk_emulator(void){

	if(disk_created == -1) {
		LOGD("no disk store opened");
		return;
	}

	disk_created = -1;
	purge_inode_table();
	if(close(disk_store) == 0) {
		LOGD("disk store successfully closed");
	} else {
		fprintf(stderr, "failed to close disk store\n");
	}
}

int read_block(big_int block_id, void * target) {

	if(block_id >= NUM_BLOCKS) {
		fprintf(stderr, "cannot read block id outside range\n");
		return -1;
	}

	off_t seek_pos = block_id * BLOCK_SIZE;
	if(lseek(disk_store, seek_pos, SEEK_SET) == -1) {
		fprintf(stderr, "failed to seek to %" PRId64 " for block %" PRIu64 "\n", seek_pos, block_id);
		return -1;
	}

	ssize_t read_bytes = read(disk_store, target, BLOCK_SIZE);
	if(read_bytes == -1) {
		fprintf(stderr, "failed to read block %" PRIu64 " with seek_pos %" PRId64 "\n", block_id, seek_pos);
		return -1;
	}

	if((big_int) read_bytes != BLOCK_SIZE) {
		fprintf(stderr, "read returned less than expected bytes (%" PRId64 ")\n", read_bytes);
	}
	return 0;
}

int write_block(big_int block_id, void * buffer, size_t buffer_size) {

	if(block_id >= NUM_BLOCKS) {
		fprintf(stderr, "cannot write block id outside range\n");
		return -1;
	}

	off_t seek_pos = block_id * BLOCK_SIZE;
	if(lseek(disk_store, seek_pos, SEEK_SET) == -1) {
		fprintf(stderr, "failed to seek to %" PRId64 " for block %" PRIu64 "\n", seek_pos, block_id);
		return -1;
	}

	if(buffer_size > BLOCK_SIZE) {
		fprintf(stderr, "not writing data beyond block size\n");
	}

	if(buffer_size < BLOCK_SIZE) {
		char data[BLOCK_SIZE];
		memcpy(data, buffer, buffer_size);tan
		memset(data + buffer_size, 0, BLOCK_SIZE - buffer_size);
		buffer = data;
	}

	ssize_t write_bytes = write(disk_store, buffer, BLOCK_SIZE);
	if(write_bytes == -1) {
		fprintf(stderr, "failed to write block %" PRIu64 " with seek_pos %" PRId64 "\n", block_id, seek_pos);
		return -1;
	}

	if((big_int) write_bytes != BLOCK_SIZE) {
		fprintf(stderr, "write returned less than expected bytes (%" PRId64 ")\n", write_bytes);
	}
	return 0;
}

#endif



