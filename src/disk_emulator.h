#ifndef DISK_EMULATOR
#define DISK_EMULATOR

#include "common.h"

int init_disk_emulator(void);
void free_disk_emulator(void);

int read_block(big_int block_id, void* target);
int write_block(big_int block_id, void* buffer, size_t buffer_size);

#endif
