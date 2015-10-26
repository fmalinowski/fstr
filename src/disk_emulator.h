#ifndef DISK_EMULATOR
#define DISK_EMULATOR

int init_disk_emulator(void);
void free_disk_emulator(void);

int read_block(int block_id, void* target);
int write_block(int block_id, void* buffer, size_t buffer_size);

#endif
