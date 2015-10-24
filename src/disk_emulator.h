#ifndef DISK_EMULATOR
#define DISK_EMULATOR

int read_block(int block_id, void** target);
int write_block(int block_id, void* buffer);

#endif
