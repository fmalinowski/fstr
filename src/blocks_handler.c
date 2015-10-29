#include "blocks_handler.h"

int superblock_to_disk_format(struct superblock * sp_block, void * block);
int superblock_from_disk_format(struct superblock * sp_block, void * block);


int inode_to_disk_format(struct inode * inod, void * inode_block_format);
int inode_from_disk_format(struct inode * inod, void * inode_block_format);


int datablock_to_disk_format(struct data_block * data_b, void * block);
int datablock_from_disk_format(struct data_block * data_b, void * block);
// CODE TO BE ADDED