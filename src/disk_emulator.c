#include "disk_emulator.h"
#include "mkfs.h"
#include "common.h"
#include "inode_table.h"
#include "uthash.h"
// in memory emulation of file system
// #define IN_MEMORY_FS

#define MAX_DATA_BLOCK_CACHE_SIZE 1024

struct data_block_entry *data_block_cache = NULL;

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
		purge_data_block_cache();
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
		disk_store = open(DISK_STORE_PATH, O_CREAT|O_RDWR);
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
	purge_data_block_cache();
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

	// Check the cache
	if(get_data_block_from_cache(block_id, target) == 0) {
		return 0;
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
		LOGD("read returned less than expected bytes: %" PRId64, read_bytes);
	}

	put_data_block_in_cache(block_id, target);
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
		memcpy(data, buffer, buffer_size);
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

	put_data_block_in_cache(block_id, buffer);
	return 0;
}

#endif

int get_data_block_from_cache(big_int data_block_id, void *data_block) {
    struct data_block_entry *data_block_entry;

    HASH_FIND(hh, data_block_cache, &data_block_id, sizeof(big_int), data_block_entry);
    if (data_block_entry) {
        LOGD("Cache hit for data block %" PRIu64, data_block_id);
        // Found in cache. Move it to front.
        HASH_DEL(data_block_cache, data_block_entry);
        HASH_ADD(hh, data_block_cache, data_block_id, sizeof(big_int), data_block_entry);

        memcpy(data_block, data_block_entry->data_block, BLOCK_SIZE);
        return 0;
    }
    LOGD("Cache miss for data block %" PRIu64, data_block_id);
    return -1;
}

int put_data_block_in_cache(big_int data_block_id, void *data_block) {
    if(data_block == NULL) {
        return -1;
    }

    struct data_block_entry *data_block_entry, *tmp;

    data_block_entry = (struct data_block_entry*) malloc(sizeof(struct data_block_entry));
    data_block_entry->data_block_id = data_block_id;
    data_block_entry->data_block = malloc(BLOCK_SIZE);;
    memcpy(data_block_entry->data_block, data_block, BLOCK_SIZE);

    LOGD("Adding to cache data block %" PRIu64, data_block_id);
    HASH_REPLACE(hh, data_block_cache, data_block_id, sizeof(big_int), data_block_entry, tmp);
    free_data_block_entry(tmp);
    tmp = NULL;
    
    // prune the cache to MAX_DATA_BLOCK_CACHE_SIZE
    if (HASH_COUNT(data_block_cache) >= MAX_DATA_BLOCK_CACHE_SIZE) {
        HASH_ITER(hh, data_block_cache, data_block_entry, tmp) {
            // prune the first entry (loop is based on insertion order so this deletes the oldest item)
            LOGD("Removing from cache data block %" PRIu64, data_block_entry->data_block_id);
            HASH_DEL(data_block_cache, data_block_entry);
            free_data_block_entry(data_block_entry);
            break;
        }
    }
    return 0;
}

void purge_data_block_cache(void) {
    struct data_block_entry *data_block_entry, *tmp;

    HASH_ITER(hh, data_block_cache, data_block_entry, tmp) {
        HASH_DEL(data_block_cache, data_block_entry);
        free_data_block_entry(data_block_entry);
    }
}

void free_data_block_entry(struct data_block_entry *data_block_entry) {
    if(data_block_entry != NULL) {
        free(data_block_entry->data_block);
        free(data_block_entry);
    }
}

