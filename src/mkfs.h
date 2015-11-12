#ifndef _MKFS_H_
#define _MKFS_H_

#include "common.h"

int create_fs(void);

int create_superblock(void);

int create_inodes(void);

int write_inode(struct inode *inode);

int create_free_blocks(void);

int create_root_inode(void);

#endif