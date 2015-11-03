#include "mkfs.h"
#include "disk_emulator.h"

void create_fs(void) {
	LOGD("Creating FSTR...");

	create_superblock();

	create_inodes();

	create_free_blocks();

	LOGD("Finished creating FSTR!");
}

// Write empty superblock to disk
int create_superblock(void) {
    superblock.fs_size = FS_SIZE;

    superblock.num_free_blocks = ((NUM_BLOCKS - NUM_INODE_BLOCKS - 1) * (BLOCK_ID_LIST_LENGTH - 1)) / BLOCK_ID_LIST_LENGTH;
    int i;
    for(i = 0; i < FREE_BLOCKS_CACHE_SIZE; i++) {
        // empty cache list
    	superblock.free_blocks_cache[i] = -1;
    }
    superblock.next_free_block_list = 1 + NUM_INODE_BLOCKS; // starts right after inode blocks

    superblock.num_free_inodes = NUM_INODES;
    for(i = 0; i < FREE_INODES_CACHE_SIZE; i++) {
        // empty cache list
    	superblock.free_inodes_cache[i] = -1;
    }
    superblock.next_free_inode = 0; // first free inode number

    LOGD("FS size: %lld", superblock.fs_size);
    LOGD("Block size: %d", BLOCK_SIZE);
    LOGD("No. of blocks: %d", NUM_BLOCKS);
    LOGD("inode size: %d", INODE_SIZE);
    LOGD("No. of inodes: %d", NUM_INODES);
    LOGD("No. of inode blocks: %lld", NUM_INODE_BLOCKS);
    LOGD("No. of free blocks: %lld", superblock.num_free_blocks);
    LOGD("Size of struct superblock: %ld", sizeof(struct superblock));
    LOGD("Size of struct inode: %ld", sizeof(struct inode));

    return write_block(0, &superblock, sizeof(struct superblock));
}

int create_inodes(void) {
    struct inode inode;

    int i, j;
    for(i = 0; i < NUM_INODES; i++){
    	inode.inode_id = i;
		inode.uid = 0;
        inode.gid = 0;
		inode.type = TYPE_FREE;
		inode.last_modified_file = 0;
		inode.last_accessed_file = 0;
		inode.last_modified_inode = 0;
		inode.links_nb = 0;

		for(j = 0; j < NUM_DIRECT_BLOCKS; j++) {
			inode.direct_blocks[j] = -1;
		}
		inode.single_indirect_block = -1;
		inode.double_indirect_block = -1;
		inode.triple_indirect_block = -1;

        if(write_inode(&inode)) {
        	fprintf(stderr, "Failed to write inodes\n");
        	return -1;
        }
    }

    return 0;
}

int write_inode(struct inode *inode) {
	big_int block_id = (inode->inode_id / (BLOCK_SIZE / INODE_SIZE)) + 1;
	int offset = (inode->inode_id % (BLOCK_SIZE / INODE_SIZE)) * INODE_SIZE;
	return write_block_offset(block_id, inode, sizeof(struct inode), offset);
}

int create_free_blocks(void) {
    struct block_id_list block_id_list;
    big_int num_data_blocks = NUM_BLOCKS - NUM_INODE_BLOCKS - 1;

    big_int num_free_blocks = superblock.num_free_blocks;
    big_int num_free_block_lists = num_free_blocks / (BLOCK_ID_LIST_LENGTH - 1);

    big_int free_block_id = 1 + NUM_INODE_BLOCKS + num_free_block_lists;

    big_int i;
    for(i = 0; i < num_free_block_lists; i++) {
        big_int block_id = 1 + NUM_INODE_BLOCKS + i;
        big_int next_block_id = -1;
        if(i < num_free_block_lists - 1) {
            next_block_id = block_id + 1;
        }

        // Store block_id of next free list
        block_id_list.list[0] = next_block_id;

        int j;
        for(j = BLOCK_ID_LIST_LENGTH - 1; j > 0; j--) {
            if(free_block_id < num_data_blocks) {
                // Store block_id of free block
                block_id_list.list[j] = free_block_id++;
            } else {
                // No free block available
                block_id_list.list[j] = -1;
            }
        }

        if(write_block(block_id, &block_id_list, sizeof(struct block_id_list))) {
            fprintf(stderr, "Failed to write free blocks\n");
            return -1;
        }
    }

	return 0;
}