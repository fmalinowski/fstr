#ifndef _BLOCK_UTILS_
#define _BLOCK_UTILS_

#include "common.h"

big_int get_single_indirect_block_id(big_int single_indirect_block_id, big_int index);

big_int get_double_indirect_block_id(big_int double_indirect_block_id, big_int index);

big_int get_triple_indirect_block_id(big_int triple_indirect_block_id, big_int index);

big_int get_block_id(struct inode *inode, big_int index);

int set_single_indirect_block_id(big_int single_indirect_block_id, big_int index, big_int block_id);

int set_double_indirect_block_id(big_int double_indirect_block_id, big_int index, big_int block_id);

int set_triple_indirect_block_id(big_int triple_indirect_block_id, big_int index, big_int block_id);

int set_block_id(struct inode *inode, big_int index, big_int block_id);

#endif
