#ifndef _INODE_TABLE_
#define _INODE_TABLE_

#include "common.h"

struct inode* get_inode(big_int inode_id);

int put_inode(struct inode *inod);

#endif
