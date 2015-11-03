#include "mkfs.h"
#include "disk_emulator.h"

void create_fs() {
	printf("Creating FSTR...\n");

	create_superblock();

	create_inodes();

	create_free_blocks();

	printf("Finished creating FSTR!\n");
}

// Write empty superblock to disk
int create_superblock() {
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

    printf("FS size: %lld\n", superblock.fs_size);
    printf("Block size: %d\n", BLOCK_SIZE);
    printf("No. of blocks: %d\n", NUM_BLOCKS);
    printf("inode size: %d\n", INODE_SIZE);
    printf("No. of inodes: %d\n", NUM_INODES);
    printf("No. of inode blocks: %lld\n", NUM_INODE_BLOCKS);
    printf("No. of free blocks: %lld\n", superblock.num_free_blocks);
    printf("Size of struct superblock: %ld\n", sizeof(struct superblock));
    printf("Size of struct inode: %ld\n", sizeof(struct inode));

    return write_block(0, &superblock, sizeof(struct superblock));
}

int create_inodes() {
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

int create_free_blocks() {
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