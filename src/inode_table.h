#ifndef _INODE_TABLE_
#define _INODE_TABLE_

#include "common.h"
#include "uthash.h"

struct inode_entry {
    int inode_id;
    struct inode *inode;
    UT_hash_handle hh;
};

int get_inode(int inode_id, struct inode *inode);
int put_inode(struct inode *inode);
void purge_inode_table(void);

// Internal use only
int get_inode_from_cache(int inode_id, struct inode *inode);
int put_inode_in_cache(struct inode *inode);
void free_inode_entry(struct inode_entry *inode_entry);

#endif
