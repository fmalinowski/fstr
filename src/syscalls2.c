#include <stdarg.h>

#include "syscalls2.h"
#include "common.h"
#include "block_utils.h"
#include "inode_table.h"
#include "data_blocks_handler.h"

#ifdef SYSCALL2__TEST
	static int namei(const char *path) {
		if (path) return syscall2__namei;
		return -1;
	}

	static int syscalls1__mknod(const char *path, mode_t mode, dev_t dev) {
		syscall2__mknod = mode; // We set the value of mode to make sure the call to mknod was done correctly
		if ((path && mode) || (path && mode && dev)) { // This is just to avoid the warning when compiling
			return 0;
		}
		return -1;
	}
#else
	#include "namei.h"
	#include "syscalls1.h"
#endif


// UTILITIES

static int min(int x, int y) {
  return (x < y) ? x : y;
}

struct file_descriptor_table *file_descriptor_tables = NULL;

struct file_descriptor_table * allocate_file_descriptor_table(int pid) {
	struct file_descriptor_table * result_table;

	HASH_FIND_INT(file_descriptor_tables, &pid, result_table);
    
    if (result_table == NULL) {
    	int i;

    	struct file_descriptor_table * table = (struct file_descriptor_table *) malloc(sizeof(struct file_descriptor_table));
    	table->pid = pid;

    	table->entries = (struct file_descriptor_entry *) malloc(8 * sizeof(struct file_descriptor_entry));
    	table->total_descriptors = 8;
    	table->used_descriptors = 3;

    	table->entries[0].fd = 0;
    	table->entries[1].fd = 1;
    	table->entries[2].fd = 2;

    	for (i = 3; i < table->total_descriptors; i++) {
    		table->entries[i].fd = FD_NOT_USED;
    	}
    	
    	HASH_ADD_INT(file_descriptor_tables, pid, table);
    	return table;
    }
    return NULL;
}

struct file_descriptor_table * get_file_descriptor_table(int pid) {
	struct file_descriptor_table * table;

	HASH_FIND_INT(file_descriptor_tables, &pid, table);
	return table;
}

void delete_file_descriptor_table(int pid) {
	struct file_descriptor_table * table;

	table = get_file_descriptor_table(pid);

	if (table) {
		HASH_DEL(file_descriptor_tables, table);

		free(table->entries);
		free(table);
	}
}

int find_available_fd(int pid) {
	struct file_descriptor_table * table;
	int i;

	table = get_file_descriptor_table(pid);

	if (table == NULL) {
		return -1;
	}

	if (table->total_descriptors == table->used_descriptors) {
		return -1;
	}
	for (i = 3; i < table->total_descriptors; i++) {
		if (table->entries[i].fd == FD_NOT_USED) {
			return i;
		}
	}
	return -1;
}

struct file_descriptor_entry * allocate_file_descriptor_entry(int pid) {
	struct file_descriptor_table * table;
	struct file_descriptor_entry * entry;
	int available_fd;

	// Allocate file descriptor table for the process if it doesn't exist
	table = get_file_descriptor_table(pid);
	if (table == NULL) {
		table = allocate_file_descriptor_table(pid);
	}

	available_fd = find_available_fd(pid);

	if (available_fd == -1) {
		// Need to resize array of entries to twice the size
		available_fd = table->total_descriptors;
		table->entries = realloc(table->entries, ((table->total_descriptors) * 2 * sizeof(struct file_descriptor_entry)));
		table->total_descriptors = table->total_descriptors * 2;

		int i;
		for (i = available_fd; i < table->total_descriptors; i++) {
    		table->entries[i].fd = FD_NOT_USED;
    	}
	}
	entry = &table->entries[available_fd];
	entry->fd = available_fd;
	table->used_descriptors++;

	return entry;
}

struct file_descriptor_entry * get_file_descriptor_entry(int pid, int fd) {
	struct file_descriptor_table * table;
	struct file_descriptor_entry * entry;

	table = get_file_descriptor_table(pid);
	if (table == NULL || fd >= table->total_descriptors) {
		return NULL;
	}

	entry = &table->entries[fd];

	// If file descriptor not yet assigned (properly allocated for an open file), we return NULL
	if (entry->fd == FD_NOT_USED) {
		return NULL;
	}

	return entry;
}

void delete_file_descriptor_entry(int pid, int fd) {
	struct file_descriptor_table * table;
	struct file_descriptor_entry * entry;

	table = get_file_descriptor_table(pid);
	if (table == NULL || fd >= table->total_descriptors || fd <= 2) {
		return;
	}

	entry = &table->entries[fd];

	// We just set the fd to be unused (-1). It would be cool to realloc the entries if we notice that 
	// all the allocated fd entries are together and below the max allocated entry to save memory.
	entry->fd = FD_NOT_USED;
	table->used_descriptors--;
}

int syscall2__get_pid(void) {
	#ifdef SYSCALL2__TEST
		return syscall2__pid;
	#else
		return (int)fuse_get_context()->pid;
	#endif
}

int get_size_of_file(big_int num_used_blocks, int num_used_bytes_in_last_block) {
	if (num_used_blocks == 0) {
		return 0;
	}
	return (num_used_blocks - 1) * BLOCK_SIZE + num_used_bytes_in_last_block;
}

int syscalls2__open(const char *path, int oflag, ...) {
	struct file_descriptor_entry * fde;
	struct inode inod;
	int inode_number, pid;
	mode_t mode;

	if (oflag & O_CREAT) {
		va_list ap;

		va_start(ap, oflag);
		mode = va_arg(ap, int);
		va_end(ap);

		// TODO We should call mknod here (not supported now cause not a priority as mknod exists and fuse doesn't use flag O_CREAT)
		if (syscalls1__mknod(path, mode, 0) == -1) {
			return -1;
		}
	}


	// Get inode number related to path (we suppose creation flag doesn't work in open for now as FUSE doesn't support it in open)
	inode_number = namei(path);
	if (inode_number == -1) {
		errno = ENOENT;
		return -1;
	}

	get_inode(inode_number, &inod);

	pid = syscall2__get_pid();
	fde = allocate_file_descriptor_entry(pid);

	// The different modes can be: O_RDONLY, O_RDWR, O_WRONLY, O_APPEND (not implemented for now)   O_CREAT, O_TRUNC (not implemented for now cause fuse doesn't use those flags)

	// TODO Here we should check for permissions when opening a file (if time permits). We need to add the field integer file_permission in inode which stores R,W...
	if (oflag & O_WRONLY) {
		// Check for write permissions here
		fde->mode = WRITE;
	}
	else if (oflag & O_RDWR) {
		// Check for read/write permissions here
		fde->mode = READ_WRITE;
	}
	else {
		fde->mode = READ;	
	}

	if (oflag & O_APPEND) {
		if (fde->mode == WRITE || fde->mode == READ_WRITE) {
			// TODO Put fde->byte_offset at right position... (not priority for now)
			// We need to look for last block number that is not a zero in the list of blocks of the inode. Then if it's indirect block, we need to look inside
		}
		else {
			errno = EACCES;
			return -1;
		}
	}
	else {
		fde->byte_offset = 0;
	}

	fde->inode_number = inode_number;

	inod.last_accessed_file = time(NULL);
	put_inode(&inod);
	return fde->fd;
}

int syscalls2__close(int fildes) {
	struct file_descriptor_entry * fde;
	struct file_descriptor_table * fdt;
	int pid;

	pid = syscall2__get_pid();
	fde = get_file_descriptor_entry(pid, fildes);

	if (fde == NULL) {
		errno = EBADF;
		return -1;
	}

	delete_file_descriptor_entry(pid, fildes);
	fdt = get_file_descriptor_table(pid);

	// Only trhe standard input, output and error are opened so we close the file descriptor table for that process
	if (fdt->used_descriptors == 3) {
		delete_file_descriptor_table(pid);
	}
	return 0;
}

// ith block starts from 1 (not from 0 !!)
int is_ith_block_in_range_of_direct_and_indirect_blocks(big_int ith_block) {
	big_int num_blocks_before_single_indirects, num_blocks_before_double_indirects, num_blocks_before_triple_indirects, num_blocks_before_quadruple_indirects;

	num_blocks_before_single_indirects = NUM_DIRECT_BLOCKS;
	num_blocks_before_double_indirects = num_blocks_before_single_indirects + BLOCK_ID_LIST_LENGTH;
	num_blocks_before_triple_indirects = num_blocks_before_double_indirects + BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH;
	num_blocks_before_quadruple_indirects = num_blocks_before_triple_indirects + BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH;

	if (ith_block < 1 || ith_block > num_blocks_before_quadruple_indirects) {
		return 0;
	}
	return 1;
}

// We count datablock from 1 to num_blocks
big_int get_ith_datablock_number(struct inode * inod, big_int ith_block) {
	return get_block_id(inod, ith_block - 1);
}

// We count datablock from 1 to num_blocks
// We set the block number of the ith block at the right position in list of direct/indirect block numbers and we update the num_blocks and 
// num_used_bytes_in_last_block = 0
// If the indirect blocks do no exist (block number = 0), we allocate them on the fly
// We save the inode and the new block number on disk
int set_ith_datablock_number(struct inode * inod, big_int ith_block, big_int block_number) {
	if (set_block_id(inod, ith_block - 1, block_number) == -1) {
		return -1;
	}

	// If we allocated a datablock that is beyond the end of the file, then we need to update the inode
	if (inod->num_blocks < ith_block) {
		inod->num_blocks = ith_block;
		inod->num_used_bytes_in_last_block = 0;
	}
	put_inode(inod);
	return 0;
}

// We count datablock from 1 to num_blocks
big_int convert_byte_offset_to_ith_datablock(off_t offset) {
	return (offset / BLOCK_SIZE) + 1;
}

ssize_t syscalls2__pread(int fildes, void *buf, size_t nbyte, off_t offset) {
	struct file_descriptor_entry * fde;
	struct inode inod;
	struct data_block db;
	int pid;
	size_t remaining_bytes, bytes_to_be_copied, read_bytes;
	off_t current_offset_in_block;
	big_int block_num_pos, current_block_number;

	pid = syscall2__get_pid();
	fde = get_file_descriptor_entry(pid, fildes);

	if (fde == NULL) {
		errno = EBADF;
		return -1; // file descriptor doesn't exist
	}

	if ((fde->mode != READ) && (fde->mode != READ_WRITE)) {
		errno = EBADF;
		return -1;
	}

	if (get_inode(fde->inode_number, &inod) == -1) {
		errno = EIO;
		return -1;
	}

	if (offset >= get_size_of_file(inod.num_blocks, inod.num_used_bytes_in_last_block)) {
		return 0; // request to read at position bigger than size of file
	}

	block_num_pos = convert_byte_offset_to_ith_datablock(offset);
	current_block_number = get_ith_datablock_number(&inod, block_num_pos);

	read_bytes = 0;
	remaining_bytes = nbyte;
	current_offset_in_block = offset % BLOCK_SIZE;

	while (remaining_bytes > 0) {
		if (remaining_bytes == nbyte) {
			bytes_to_be_copied = min(BLOCK_SIZE - current_offset_in_block, remaining_bytes);
		}
		else {
			bytes_to_be_copied = min(BLOCK_SIZE, remaining_bytes);
		}

		if (block_num_pos == inod.num_blocks) {
			if ((inod.num_used_bytes_in_last_block == current_offset_in_block) && remaining_bytes > 0) {
				// We have reached the end of the file and there are still some bytes to be read
				fde->byte_offset = get_size_of_file(inod.num_blocks, inod.num_used_bytes_in_last_block);
				return read_bytes;
			}
			bytes_to_be_copied = min(bytes_to_be_copied, inod.num_used_bytes_in_last_block - current_offset_in_block);
		}

		if (current_block_number == 0) {
			// Non allocated datablock so we fill with 0s only
			memset(&((char *)buf)[read_bytes], 0, bytes_to_be_copied);
		}
		else {
			if (bread(current_block_number, &db) == -1) {
				errno = EIO;
				return -1;
			}
			memcpy(&((char *)buf)[read_bytes], &db.block[current_offset_in_block], bytes_to_be_copied);
		}

		read_bytes += bytes_to_be_copied;
		current_offset_in_block = (current_offset_in_block + bytes_to_be_copied) % BLOCK_SIZE;
		remaining_bytes -= bytes_to_be_copied;

		if (current_offset_in_block == 0) {
			// We need to read the next datablock cause we have reached the end of a block in this read step
			block_num_pos++;
			current_block_number = get_ith_datablock_number(&inod, block_num_pos);
		}
	}

	fde->byte_offset = offset + read_bytes;

	inod.last_accessed_file = time(NULL);
	if (put_inode(&inod) == -1) {
		errno = EIO;
		return -1;
	}
	return read_bytes;
}

ssize_t syscalls2__pwrite(int fildes, const void *buf, size_t nbyte, off_t offset) {
	struct file_descriptor_entry * fde;
	struct inode inod;
	struct data_block db;
	int pid;
	size_t remaining_bytes, bytes_to_be_copied, written_bytes;
	off_t current_offset_in_block;
	big_int block_num_pos, current_block_number;

	pid = syscall2__get_pid();
	fde = get_file_descriptor_entry(pid, fildes);

	if (fde == NULL) {
		errno = EBADF;
		return -1; // file descriptor doesn't exist
	}

	if ((fde->mode != WRITE) && (fde->mode != READ_WRITE)) {
		errno = EBADF;
		return -1;
	}

	get_inode(fde->inode_number, &inod);
	block_num_pos = convert_byte_offset_to_ith_datablock(offset);

	if (offset >= get_size_of_file(inod.num_blocks, inod.num_used_bytes_in_last_block) && !is_ith_block_in_range_of_direct_and_indirect_blocks(block_num_pos)) {
		// request to write at position bigger than size of file
		// AND We cannot allocate and write in a block that is not in the range of direct/indirect blocks
		errno = EFBIG;
		return -1;
	}

	current_block_number = get_ith_datablock_number(&inod, block_num_pos);
	remaining_bytes = nbyte;
	current_offset_in_block = offset % BLOCK_SIZE;
	written_bytes = 0;


	while (remaining_bytes > 0) {
		bytes_to_be_copied = min(BLOCK_SIZE - current_offset_in_block, remaining_bytes);

		// if datablock is not allocated
		if (current_block_number == 0) {
			if (data_block_alloc(&db) == -1) {
				errno = EDQUOT;
				return -1;
			}
			if (set_ith_datablock_number(&inod, block_num_pos, db.data_block_id) == -1) {
				errno = EIO;
				return -1;
			}

			// We increment the number of datablocks allocated for the file
			inod.num_allocated_blocks++;
		}
		else {
			if (bread(current_block_number, &db) == -1) {
				errno = EIO;
				return -1;
			}
		}

		memcpy(&db.block[current_offset_in_block], &((char *)buf)[written_bytes], bytes_to_be_copied);
		if (bwrite(&db) == -1) {
			errno = EIO;
			return -1;
		}
		written_bytes += bytes_to_be_copied;
		current_offset_in_block = (current_offset_in_block + bytes_to_be_copied) % BLOCK_SIZE;
		remaining_bytes -= bytes_to_be_copied;

		if (current_offset_in_block == 0) {
			block_num_pos++;
			current_block_number = get_ith_datablock_number(&inod, block_num_pos);
		}
	}

	// Update the inode for the number of blocks used and position of last byte in file
	if ((block_num_pos > inod.num_blocks) || (block_num_pos == inod.num_blocks && current_offset_in_block > inod.num_used_bytes_in_last_block)) {
		if (current_offset_in_block == 0) {
			inod.num_blocks = block_num_pos - 1;
			inod.num_used_bytes_in_last_block = BLOCK_SIZE;
		}
		else {
			inod.num_blocks = block_num_pos;
			inod.num_used_bytes_in_last_block = current_offset_in_block;
		}
		inod.last_modified_inode = time(NULL);
	}

	inod.last_modified_file = time(NULL);
	
	if (put_inode(&inod) == -1) {
		errno = EIO;
		return -1;
	}

	fde->byte_offset = offset + written_bytes;
	return written_bytes;
}
