#include "inode_table.h"
#include "inodes_handler.h"

// Here we don't implement an inode table yet. We just call iget and iput to get and put inodes on disk.
// Optimizations will be done later if time permits

int get_inode(int inode_id, struct inode *inod) {
	return iget(inode_id, inod);
}

int put_inode(struct inode *inod) {
	return iput(inod);
}
