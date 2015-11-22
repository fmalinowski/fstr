#include "block_utils.h"
#include "common.h"
#include "disk_emulator.h"
#include "data_blocks_handler.h"
#include "inode_table.h"

big_int get_single_indirect_block_id(big_int single_indirect_block_id, big_int index) {
	if(single_indirect_block_id == 0) {
		fprintf(stderr, "single indirect block id not set\n");
		return 0;
	}

	struct block_id_list single_indirect_block;
	if(read_block(single_indirect_block_id, &single_indirect_block) == 0) {
		return single_indirect_block.list[index];
	}
	return 0;
}

big_int get_double_indirect_block_id(big_int double_indirect_block_id, big_int index) {
	if(double_indirect_block_id == 0) {
		fprintf(stderr, "double indirect block id not set\n");
		return 0;
	}

	struct block_id_list double_indirect_block;
	if(read_block(double_indirect_block_id, &double_indirect_block) == 0) {

		int bucket = index / BLOCK_ID_LIST_LENGTH;
		int offset = index % BLOCK_ID_LIST_LENGTH;
		return get_single_indirect_block_id(double_indirect_block.list[bucket], offset);
	}
	return 0;
}

big_int get_triple_indirect_block_id(big_int triple_indirect_block_id, big_int index) {
	if(triple_indirect_block_id == 0) {
		fprintf(stderr, "triple indirect block id not set\n");
		return 0;
	}

	struct block_id_list triple_indirect_block;
	if(read_block(triple_indirect_block_id, &triple_indirect_block) == 0) {

		int bucket = index / (BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH);
		int offset = index % (BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH);
		return get_double_indirect_block_id(triple_indirect_block.list[bucket], offset);
	}
	return 0;
}

big_int get_block_id(struct inode *inode, big_int index) {
	// Check for direct block
	if(index < NUM_DIRECT_BLOCKS) {
		return inode->direct_blocks[index];
	}

	index -= NUM_DIRECT_BLOCKS;

	if(index < BLOCK_ID_LIST_LENGTH) {
		return get_single_indirect_block_id(inode->single_indirect_block, index);
	}

	index -= BLOCK_ID_LIST_LENGTH;

	if(index < BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH) {
		return get_double_indirect_block_id(inode->double_indirect_block, index);
	}

	index -= BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH;

	if(index < BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH) {
		return get_triple_indirect_block_id(inode->triple_indirect_block, index);
	}

	fprintf(stderr, "index is out of range for get_block_id\n");
	return 0;
}

int set_single_indirect_block_id(big_int single_indirect_block_id, big_int index, big_int block_id) {
	if(single_indirect_block_id == 0) {
		fprintf(stderr, "single indirect block id not set\n");
		return -1;
	}

	struct block_id_list single_indirect_block;
	if(read_block(single_indirect_block_id, &single_indirect_block) == 0) {
		single_indirect_block.list[index] = block_id;
		return write_block(single_indirect_block_id, &single_indirect_block, sizeof(struct block_id_list));
	}
	return -1;
}

int set_double_indirect_block_id(big_int double_indirect_block_id, big_int index, big_int block_id) {
	if(double_indirect_block_id == 0) {
		fprintf(stderr, "double indirect block id not set\n");
		return 0;
	}

	struct block_id_list double_indirect_block;
	if(read_block(double_indirect_block_id, &double_indirect_block) == 0) {

		int bucket = index / BLOCK_ID_LIST_LENGTH;
		int offset = index % BLOCK_ID_LIST_LENGTH;

		// Make sure we have valid block
		if(double_indirect_block.list[bucket] == 0) {
			struct data_block data_block;
			if(data_block_alloc(&data_block) == -1) {
				fprintf(stderr, "Failed to alloc data block\n");
				return -1;
			}
			double_indirect_block.list[bucket] = data_block.data_block_id;
			write_block(double_indirect_block_id, &double_indirect_block, sizeof(struct block_id_list));
		}
		return set_single_indirect_block_id(double_indirect_block.list[bucket], offset, block_id);
	}
	return -1;
}

int set_triple_indirect_block_id(big_int triple_indirect_block_id, big_int index, big_int block_id) {
	if(triple_indirect_block_id == 0) {
		fprintf(stderr, "triple indirect block id not set\n");
		return 0;
	}

	struct block_id_list triple_indirect_block;
	if(read_block(triple_indirect_block_id, &triple_indirect_block) == 0) {

		int bucket = index / (BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH);
		int offset = index % (BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH);

		// Make sure we have valid block
		if(triple_indirect_block.list[bucket] == 0) {
			struct data_block data_block;
			if(data_block_alloc(&data_block) == -1) {
				fprintf(stderr, "Failed to alloc data block\n");
				return -1;
			}
			triple_indirect_block.list[bucket] = data_block.data_block_id;
			write_block(triple_indirect_block_id, &triple_indirect_block, sizeof(struct block_id_list));
		}
		return set_double_indirect_block_id(triple_indirect_block.list[bucket], offset, block_id);
	}
	return -1;
}

int set_block_id(struct inode *inode, big_int index, big_int block_id) {
	// Check for direct block
	if(index < NUM_DIRECT_BLOCKS) {
		inode->direct_blocks[index] = block_id;
		return 0;
	}

	index -= NUM_DIRECT_BLOCKS;

	if(index < BLOCK_ID_LIST_LENGTH) {
		// Make sure we have valid block
		if(inode->single_indirect_block == 0) {
			struct data_block data_block;
			if(data_block_alloc(&data_block) == -1) {
				fprintf(stderr, "Failed to alloc data block\n");
				return -1;
			}
			inode->single_indirect_block = data_block.data_block_id;
		}
		return set_single_indirect_block_id(inode->single_indirect_block, index, block_id);
	}

	index -= BLOCK_ID_LIST_LENGTH;

	if(index < BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH) {
		// Make sure we have valid block
		if(inode->double_indirect_block == 0) {
			struct data_block data_block;
			if(data_block_alloc(&data_block) == -1) {
				fprintf(stderr, "Failed to alloc data block\n");
				return -1;
			}
			inode->double_indirect_block = data_block.data_block_id;
		}
		return set_double_indirect_block_id(inode->double_indirect_block, index, block_id);
	}

	index -= BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH;

	if(index < BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH) {
		// Make sure we have valid block
		if(inode->triple_indirect_block == 0) {
			struct data_block data_block;
			if(data_block_alloc(&data_block) == -1) {
				fprintf(stderr, "Failed to alloc data block\n");
				return -1;
			}
			inode->triple_indirect_block = data_block.data_block_id;
		}
		return set_triple_indirect_block_id(inode->triple_indirect_block, index, block_id);
	}

	fprintf(stderr, "index is out of range for set_block_id\n");
	return -1;
}
