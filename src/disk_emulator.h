#ifndef DISK_EMULATOR
#define DISK_EMULATOR

void disk_init();
void disk_destroy();

int read_block(int block_id, void** target);
int write_block(int block_id, void* buffer, int nbytes);

#endif
