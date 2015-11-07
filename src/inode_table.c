#include "inode_table.h"
#include "inodes_handler.h"

// Here we don't implement an inode table yet. We just call iget and iput to get and put inodes on disk.
// Optimizations will be done later if time permits

struct inode* get_inode(big_int inode_id) {
	return iget(inode_id);
}

int put_inode(struct inode *inod) {
	return iput(inod);
}
