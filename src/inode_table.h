#ifndef _INODE_TABLE_
#define _INODE_TABLE_

#include "common.h"

int get_inode(int inode_id, struct inode *inod);

int put_inode(struct inode *inod);

#endif
