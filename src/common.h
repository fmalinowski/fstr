#ifndef _COMMON_H_
#define _COMMON_H_

#define FUSE_USE_VERSION 26
#define __STDC_FORMAT_MACROS 1

#include <stdio.h>
#include <string.h>
#include <fuse.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <inttypes.h>
#include <libgen.h>
// disk access headers
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define DISK_STORE_PATH "/dev/vdc"

// #define DEBUG
#ifdef DEBUG
#define LOGD(A, ...) printf("%s:%d " A "\n", __FILE__, __LINE__, ##__VA_ARGS__);
#else
#define LOGD(A, ...)
#endif

#define FREE_BLOCKS_CACHE_SIZE 0
#define FREE_INODES_CACHE_SIZE 0

#define FS_SIZE ((big_int) 30 * 1024 * 1024 * 1024) // 30GB
#define BLOCK_SIZE 4096 // 4KB
#define INODE_SIZE 256
#define NUM_INODES ((int) (0.1 * NUM_BLOCKS))
#define ILIST_BEGIN 1

#define NUM_DIRECT_BLOCKS 16

#define NUM_BLOCKS (FS_SIZE / BLOCK_SIZE)
#define NUM_INODE_BLOCKS ((big_int) ceil((NUM_INODES * INODE_SIZE) / (float) BLOCK_SIZE))
#define BLOCK_ID_LIST_LENGTH (BLOCK_SIZE / sizeof(big_int))

#define NAMEI_ENTRY_SIZE 64
#define PATH_DELIMITER "/"
#define ROOT_INODE_NUMBER 1

typedef uint64_t big_int;

typedef enum {
	TYPE_ORDINARY,
	TYPE_DIRECTORY,
	TYPE_FREE
} file_type;

struct superblock {
	// General
	big_int fs_size;

	// Free blocks management stuff
	big_int num_free_blocks;
	big_int free_blocks_cache[FREE_BLOCKS_CACHE_SIZE];
	big_int next_free_block_list;

	// Free inodes management stuff
	big_int num_free_inodes;
	big_int free_inodes_cache[FREE_INODES_CACHE_SIZE];
	big_int next_free_inode;

} superblock;

struct inode {
	int inode_id;
	uid_t uid;
	gid_t gid;
	mode_t mode;
	file_type type;
	time_t last_modified_file;
	time_t last_accessed_file;
	time_t last_modified_inode;
	int links_nb;
	big_int direct_blocks[NUM_DIRECT_BLOCKS];
	big_int single_indirect_block;
	big_int double_indirect_block;
	big_int triple_indirect_block;
	big_int num_blocks; // Number of blocks that are used (if there are empty blocks before a filled block, 
						// the empty blocks are also considered. The (num_blocks)th block can be partially filled)
	int num_used_bytes_in_last_block; // Number of bytes used in last datablock for the file
	big_int num_allocated_blocks;
};

struct block_id_list {
	big_int list[BLOCK_ID_LIST_LENGTH];
};

struct data_block {
	big_int data_block_id;
	char block[BLOCK_SIZE];
};

struct dir_block {
    int inode_ids[BLOCK_SIZE / NAMEI_ENTRY_SIZE];
    char names[BLOCK_SIZE / NAMEI_ENTRY_SIZE][NAMEI_ENTRY_SIZE - sizeof(int)];
};

int init_superblock(void);

int commit_superblock(void);

int write_block_offset(big_int block_id, void *buffer, size_t buffer_size, int offset);

// Format dir_block and add entries for '.' and '..'
int init_dir_block(struct dir_block *dir_block, int dir_inode_id, int parent_inode_id);

// Insert inode entry in parent's inode. Might alloc dir_block if required
int add_entry_to_parent(struct inode *parent_inode, int inode_id, const char *name);

// Remove inode entry from parent's inode.
int remove_entry_from_parent(struct inode *parent_inode, int inode_id);

// Insert inode entry to first available position in dir_block
int add_entry_to_dir_block(struct dir_block *dir_block, int inode_id, const char *name);

// Remove inode entry from dir_block
int remove_entry_from_dir_block(struct dir_block *dir_block, int inode_id);

int get_parent_inode_id(const char *path);

#endif
