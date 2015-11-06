#ifndef _COMMON_H_
#define _COMMON_H_

#define FUSE_USE_VERSION 26

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <fuse.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

#ifdef DEBUG
#define LOGD(A, ...) printf("%s:%d " A "\n", __FILE__, __LINE__, ##__VA_ARGS__);
#else
#define LOGD(A, ...)
#endif

#define FREE_BLOCKS_CACHE_SIZE 0
#define FREE_INODES_CACHE_SIZE 0

#define FS_SIZE 512 * 1024 * 1024 // 512MB
#define BLOCK_SIZE 4096 // 4KB
#define INODE_SIZE 256
#define NUM_INODES ((int) (0.1 * NUM_BLOCKS))

#define NUM_DIRECT_BLOCKS 16

#define NUM_BLOCKS (FS_SIZE / BLOCK_SIZE)
#define NUM_INODE_BLOCKS ((big_int) ceil((NUM_INODES * INODE_SIZE) / (float) BLOCK_SIZE))
#define BLOCK_ID_LIST_LENGTH (BLOCK_SIZE / sizeof(big_int))

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
} /* shared superblock object */ superblock;

struct inode {
	int inode_id;
	uid_t uid;
	gid_t gid;
	file_type type;
	time_t last_modified_file;
	time_t last_accessed_file;
	time_t last_modified_inode;
	int links_nb;
	big_int direct_blocks[NUM_DIRECT_BLOCKS];
	big_int single_indirect_block;
	big_int double_indirect_block;
	big_int triple_indirect_block;
};

struct block_id_list {
	big_int list[BLOCK_ID_LIST_LENGTH];
};

struct data_block {
	big_int data_block_id;
	char block[BLOCK_SIZE];
};

int write_block_offset(big_int block_id, void *buffer, size_t buffer_size, int offset);

#endif
