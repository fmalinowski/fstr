#ifndef _INODE_TABLE_
#define _INODE_TABLE_

struct inode* get_inode(int inode_id);

int put_inode(struct inode *inode);

#endif