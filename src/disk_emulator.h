#ifndef DISK_EMULATOR
#define DISK_EMULATOR

int allocate_disk(int blck_size, int inode_blck_nb, int data_blck_nb);
void unallocate_disk(void);
int write_block(int block_id, char * buffer);
int read_block(int block_id, char * buffer);

#endif