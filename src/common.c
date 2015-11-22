#include "common.h"
#include "disk_emulator.h"
#include "data_blocks_handler.h"
#include "block_utils.h"
#include "namei.h"

int init_superblock(void) {
	struct data_block block;
	if(read_block(0, &block.block)) {
		fprintf(stderr, "Failed to read superblock\n");
		return -1;
	}

	memcpy(&superblock, &block.block, sizeof(struct superblock));
	return 0;
}

int commit_superblock(void) {
	return write_block(0, &superblock, sizeof(struct superblock));
}
	
int write_block_offset(big_int block_id, void *buffer, size_t buffer_size, int offset) {
	if(offset + buffer_size > BLOCK_SIZE) {
		fprintf(stderr, "Data too big for a block. offset: %d, buffer_size: %ld\n", offset, buffer_size);
		return -1;
	}

	char *target = malloc(sizeof(char) * BLOCK_SIZE);
	read_block(block_id, target);
	memcpy(target + offset, buffer, buffer_size);
	write_block(block_id, target, BLOCK_SIZE);
	free(target);

	return 0;
}

int init_dir_block(struct dir_block *dir_block, int dir_inode_id, int parent_inode_id) {
	int len = BLOCK_SIZE / NAMEI_ENTRY_SIZE;
	int i;
	for(i = 0; i < len; ++i) {
		dir_block->inode_ids[i] = 0;
		strcpy(dir_block->names[i], "");
	}

	add_entry_to_dir_block(dir_block, dir_inode_id, ".");
	add_entry_to_dir_block(dir_block, parent_inode_id, "..");

	return 0;
}

int add_entry_to_parent(struct inode *parent_inode, int inode_id, const char *name) {
	// Try adding entry to existing block if possible
	if(parent_inode->num_blocks > 0) {
		big_int block_id = get_block_id(parent_inode, parent_inode->num_blocks-1);
		if(block_id == 0) {
			fprintf(stderr, "could not fetch block id from inode\n");
			return -1;
		}

		struct dir_block dir_block;
		read_block(block_id, &dir_block);

		if(add_entry_to_dir_block(&dir_block, inode_id, name) == 0) {
			return write_block(block_id, &dir_block, sizeof(struct dir_block));
		}		
	}

	// Alloc a new block and add entry there
	struct data_block *block = data_block_alloc();
	if(block == NULL) {
		fprintf(stderr, "could not alloc data block\n");
		return -1;
	}

	// Add this block to inode
	if(set_block_id(parent_inode, parent_inode->num_blocks, block->data_block_id) == -1) {
		fprintf(stderr, "could not add newly allocated block to inode\n");
		return -1;
	}
	parent_inode->num_blocks++;

	struct dir_block *dir_block = (struct dir_block*) block->block;
	add_entry_to_dir_block(dir_block, inode_id, name);
	return write_block(block->data_block_id, dir_block, sizeof(struct dir_block));
}

int add_entry_to_dir_block(struct dir_block *dir_block, int inode_id, const char *name) {
	int len = BLOCK_SIZE / NAMEI_ENTRY_SIZE;
	int i;
	for(i = 0; i < len; ++i) {
		if(dir_block->inode_ids[i] == 0) {
			dir_block->inode_ids[i] = inode_id;
			strcpy(dir_block->names[i], name);
			break;
		}
	}

	if(i == len) {
		// This block is full
		return -1;
	}
	return 0;
}

int remove_entry_from_dir_block(struct dir_block *dir_block, int inode_id) {
	int len = BLOCK_SIZE / NAMEI_ENTRY_SIZE;
	int i;
	for(i = 0; i < len; ++i) {
		if(dir_block->inode_ids[i] == inode_id) {
			dir_block->inode_ids[i] = 0;
			strcpy(dir_block->names[i], "");
			return 0;
		}
	}
	return -1;
}

int get_parent_inode_id(const char *path) {
	if(strcmp(PATH_DELIMITER, path) == 0) {
		return ROOT_INODE_NUMBER;
	}
	char *dir = dirname(strdup(path));
	return namei(dir);
}
