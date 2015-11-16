#ifndef _SYSCALLS_2_
#define _SYSCALLS_2_

#include <sys/types.h>

#include "uthash.h"
#include "common.h"

// The methods below use user file descriptor tables

#define FD_NOT_USED -1

#ifdef SYSCALL2__TEST
	int syscall2__pid;
	int syscall2__namei;
	int syscall2__mknod;
#endif

typedef enum {
	READ,
	WRITE, 
	READ_WRITE
} access_mode;

struct file_descriptor_entry {
	int fd;
	access_mode mode;
	int inode_number;
	int byte_offset;
};

struct file_descriptor_table {
	int pid;
	int total_descriptors;
	int used_descriptors;
	struct file_descriptor_entry *entries;
	UT_hash_handle hh;
}; // We need to use dynamic allocation with that hash table otherwise it doesn't work correctly


ssize_t pread(int fildes, void *buf, size_t nbyte, off_t offset); 
ssize_t pwrite(int fildes, const void *buf, size_t nbyte, off_t offset);

int open(const char *path, int oflag, ... ); // Supports O_RDONLY, O_RDWR, O_WRONLY. O_APPEND, O_CREAT and O_TRUNC are not yet supported
int close(int fildes);


// UTILITIES
struct file_descriptor_table * allocate_file_descriptor_table(int pid);
struct file_descriptor_table * get_file_descriptor_table(int pid);
void delete_file_descriptor_table(int pid);

int find_available_fd(int pid);

struct file_descriptor_entry * allocate_file_descriptor_entry(int pid);
struct file_descriptor_entry * get_file_descriptor_entry(int pid, int fd);
void delete_file_descriptor_entry(int pid, int fd);

int syscall2__get_pid(void);

int get_size_of_file(big_int num_used_blocks, int last_byte_offset);

int is_ith_block_in_range_of_direct_and_indirect_blocks(big_int ith_block); // Return 1 if true, 0 otherwise
big_int get_ith_datablock_number(struct inode * inod, big_int ith_block); // ith block starts from 1 (1st datablock is in direct block)
int set_ith_datablock_number(struct inode * inod, big_int ith_block, big_int block_number); // ith block starts from 1 (1st datablock is in direct block)
big_int convert_byte_offset_to_ith_datablock(off_t offset); // returns the ith datablock that contains the offset (i starts from 1 and is a direct block)

#endif
