#ifndef DISK_EMULATOR
#define DISK_EMULATOR

#include "common.h"
#include "uthash.h"

struct data_block_entry {
    big_int data_block_id;
    void *data_block;
    UT_hash_handle hh;
};

int init_disk_emulator(void);
void free_disk_emulator(void);

int read_block(big_int block_id, void* target);
int write_block(big_int block_id, void* buffer, size_t buffer_size);

void purge_data_block_cache(void);

// Internal use only
int get_data_block_from_cache(big_int data_block_id, void *data_block);
int put_data_block_in_cache(big_int data_block_id, void *data_block);
void free_data_block_entry(struct data_block_entry *data_block_entry);

#endif
