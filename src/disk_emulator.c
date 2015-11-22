#include "disk_emulator.h"
#include "mkfs.h"
#include "common.h"
// in memory emulation of file system
//#define IN_MEMORY_FS


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
    		return -ENODEV; // failure
  		}
  		LOGD("DISK_EMULATOR: File descriptor of disk: %d", disk_store);
		disk_created = 0;
		return 0; // success
	}
	else{
		LOGD("DISK_EMULATOR: disk store already initialised");
		return -1;	
	}
}

void free_disk_emulator(void){
	if(disk_created == 0 && close(disk_store) == 0){
    	disk_created = -1;
  		LOGD("FREE_DISK_EMULATOR: Disk successfully closed.");  	
  	}
  	else{
  		LOGD("FREE_DISK_EMULATOR: Did not close disk.");
  		fprintf(stderr, "Error closing disk\n");
    }
}

int read_block(big_int block_id, void * target) {
	if(block_id < NUM_BLOCKS){
		if(lseek(disk_store, block_id * BLOCK_SIZE, SEEK_SET) != -1){ // seek to position of block id
			if(read(disk_store, target, BLOCK_SIZE) == BLOCK_SIZE){
				return 0; // read successful
			}
		}
	}
	LOGD("READ_BLOCK: block read unsuccessful");
	return -1;
}

int write_block(big_int block_id, void * buffer, size_t buffer_size) {
	if(block_id < NUM_BLOCKS){
		if(lseek(disk_store, block_id * BLOCK_SIZE, SEEK_SET) != -1){
			int bytes_written;
			size_t copy_size = buffer_size < BLOCK_SIZE ? buffer_size : BLOCK_SIZE;
			if((bytes_written = write(disk_store, buffer, copy_size)) != -1){

				if(copy_size < BLOCK_SIZE){
					void *temp = calloc(BLOCK_SIZE - copy_size, sizeof(int));
					
					if(temp && lseek(disk_store, (block_id * BLOCK_SIZE) + copy_size, SEEK_SET) != -1){
						if(write(disk_store, temp, BLOCK_SIZE - copy_size) != -1){
							free(temp);
							return 0; // Success				
						}
					}
					if(temp){
						free(temp);
					}
					LOGD("something went wrong");
					return -1;
				}
				return 0; // write successful	
			}
		}
	}
	LOGD("WRITE_BLOCK: block write unsuccessful");
	return -1;
}

#endif



