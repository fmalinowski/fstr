#ifndef BLOCKS_HANDLER
#define BLOCKS_HANDLER

#include <time.h>

#define INODE_LIST_SIZE 1024 // WE NEED TO FIGURE OUT THIS NUMBER
#define INODE_SIZE 256 /* WE NEED TO FIGURE OUT THIS NUMBER. 
						  I have defined this here, instead of using sizeof(struct inode),
						  because we want to make sure that this size 
						  is a power of two.
						*/
#define DATA_BLOCK_LIST_SIZE 4096 // WE NEED TO FIGURE OUT THIS NUMBER
#define ILIST_BEGIN ? // FIX BEGINNING POSITION OF I-LIST
#define DATA_BLOCK_SIZE 4096

struct superbock {
	int total_data_blocks;
	short block_size;
	short number_of_inodes;
	unsigned char inode_size;
	int free_data_blocks;
	int free_inodes_list_size;
	int * free_inodes_list;
};

typedef enum {
	ordinary,
	directory,
	free
} File_Type;

struct inode {
	int inode_id;
	int file_owner_id;
	File_Type file_type;
	char permissions;
	time_t last_modified_file;
	time_t last_accessed_file;
	time_t last_modified_inode;
	int links_nb;
	int direct_blocks[16];
	int single_indirect_block;
	int double_indirect_block;
	int triple_indirect_block;
};

struct data_block {
	int data_block_id;
	char block[DATA_BLOCK_SIZE];
};

int make_fs(void);
struct inode * ialloc(void);	// allocate an inode
struct inode * iget(int inode_number); // read an inode
int iput(struct inode *); // Update the inode and free inode and data blocks if link count reaches 0
int ifree(struct inode *); // free the inode and put it in the free inode list

struct data_block * data_block_alloc(); // Will make calls to bread, brelse, getblk
struct data_block * bread(int data_block_nb); // Read the data block
int bwrite(struct data_block *); // Write the data block
int data_block_free(struct data_block *);


#endif
