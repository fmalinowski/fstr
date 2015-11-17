#include "mkfs.h"
#include "disk_emulator.h"
#include "data_blocks_handler.h"
#include "inode_table.h"
#include "inodes_handler.h"

int create_fs(void) {
    LOGD("Creating FSTR...");

    if(create_superblock() != 0) {
        fprintf(stderr, "failed to create superblock\n");
        return -1;
    }

    if(create_inodes() != 0) {
        fprintf(stderr, "failed to create inodes\n");
        return -1;   
    }

    if(create_free_blocks() != 0)  {
        fprintf(stderr, "failed to create free block lists\n");
        return -1;   
    }

    if(create_root_dir() != 0)  {
        fprintf(stderr, "failed to create mkdir root\n");
        return -1;   
    }

    LOGD("Finished creating FSTR!");
    return 0;
}

// Write empty superblock to disk
int create_superblock(void) {
    superblock.fs_size = FS_SIZE;

    superblock.num_free_blocks = ((NUM_BLOCKS - NUM_INODE_BLOCKS - 1) * (BLOCK_ID_LIST_LENGTH - 1)) / BLOCK_ID_LIST_LENGTH;
    int i;
    for(i = 0; i < FREE_BLOCKS_CACHE_SIZE; i++) {
        // empty cache list
        superblock.free_blocks_cache[i] = 0;
    }
    superblock.next_free_block_list = 1 + NUM_INODE_BLOCKS; // starts right after inode blocks

    superblock.num_free_inodes = NUM_INODES;
    for(i = 0; i < FREE_INODES_CACHE_SIZE; i++) {
        // empty cache list
        superblock.free_inodes_cache[i] = 0;
    }
    superblock.next_free_inode = 1; // first free inode number

    LOGD("FS size: %" PRIu64 "", superblock.fs_size);
    LOGD("Block size: %d", BLOCK_SIZE);
    LOGD("No. of blocks: %d", NUM_BLOCKS);
    LOGD("inode size: %d", INODE_SIZE);
    LOGD("No. of inodes: %d", NUM_INODES);
    LOGD("No. of inode blocks: %lu", NUM_INODE_BLOCKS);
    LOGD("No. of free blocks: %" PRIu64 "", superblock.num_free_blocks);
    LOGD("Size of struct superblock: %ld", sizeof(struct superblock));
    LOGD("Size of struct inode: %ld", sizeof(struct inode));

    return write_block(0, &superblock, sizeof(struct superblock));
}

int create_inodes(void) {
    int i;
    for(i = 0; i < NUM_INODES; i++){
        struct inode inode = {
            .inode_id = i + 1,
            .type = TYPE_FREE
        };

        if(write_inode(&inode)) {
            fprintf(stderr, "Failed to write inodes\n");
            return -1;
        }
    }

    return 0;
}

int write_inode(struct inode *inode) {
    big_int block_id = ((inode->inode_id - 1) / (BLOCK_SIZE / INODE_SIZE)) + 1;
    int offset = ((inode->inode_id - 1) % (BLOCK_SIZE / INODE_SIZE)) * INODE_SIZE;
    return write_block_offset(block_id, inode, sizeof(struct inode), offset);
}

int create_free_blocks(void) {
    struct block_id_list block_id_list;

    big_int num_free_block_lists = NUM_BLOCKS - (1 + NUM_INODE_BLOCKS + superblock.num_free_blocks);

    big_int free_block_id = 1 + NUM_INODE_BLOCKS + num_free_block_lists;

    big_int i;
    for(i = 0; i < num_free_block_lists; i++) {
        big_int block_id = 1 + NUM_INODE_BLOCKS + i;
        big_int next_block_id = 0;
        if(i < num_free_block_lists - 1) {
            next_block_id = block_id + 1;
        }

        // Store block_id of next free list
        block_id_list.list[0] = next_block_id;

        int j;
        for(j = BLOCK_ID_LIST_LENGTH - 1; j > 0; j--) {
            if(free_block_id < NUM_BLOCKS) {
                // Store block_id of free block
                block_id_list.list[j] = free_block_id++;
            } else {
                // No free block available
                block_id_list.list[j] = 0;
            }
        }

        if(write_block(block_id, &block_id_list, sizeof(struct block_id_list))) {
            fprintf(stderr, "Failed to write free blocks\n");
            return -1;
        }
    }

    return 0;
}

int create_root_dir(void) {
    struct data_block *block = data_block_alloc();
    if(block == NULL) {
        fprintf(stderr, "could not find a free data block\n");
        errno = EDQUOT;
        return -1;
    }

    struct inode *inode = ialloc();
    if(inode == NULL) {
        fprintf(stderr, "could not find a free inode\n");
        errno = EDQUOT;
        return -1;
    }

    inode->type = TYPE_DIRECTORY;
    inode->mode = S_IFDIR;

    // Init the new dir block
    big_int block_id = block->data_block_id;
    struct dir_block dir_block;
    if(init_dir_block(&dir_block, inode->inode_id, ROOT_INODE_NUMBER) == -1) {
        fprintf(stderr, "failed to format a dir block\n");
        return -1;
    }
    write_block(block_id, &dir_block, sizeof(struct dir_block));
    inode->direct_blocks[0] = block_id;
    inode->num_blocks++;
    put_inode(inode);
    return 0;
}
