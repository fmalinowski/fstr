#ifndef INODES_HANDLER
#define INODES_HANDLER

#include "common.h"

struct inode * ialloc(void);	// allocate an inode. // THIS DOES NOT SET THE FILETYPE OF INODE. MUST BE DONE AT LAYER 2
struct inode * iget(big_int inode_number); // read an inode
int next_free_inode_number(void);
int iput(struct inode *); // Update the inode and free inode and data blocks if link count reaches 0
int ifree(struct inode *); // free the inode and put it in the free inode list

void free_inode(struct inode * inod);

#endif
