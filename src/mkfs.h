#ifndef _MKFS_H_
#define _MKFS_H_

#include "common.h"

void create_fs();

int create_superblock();

int create_inodes();

int write_inode(struct inode *inode);

int create_free_blocks();

#endif