#include "syscalls1.h"
#include "common.h"
#include "inodes_handler.h"
#include "inode_table.h"
#include "data_blocks_handler.h"
#include "disk_emulator.h"
#include "block_utils.h"
#include "namei.h"

int syscalls1__mkdir(const char *path, mode_t mode) {

	// Check for existing file
	if(namei(path) != -1) {
		fprintf(stderr, "file already exists\n");
		errno = EEXIST;
		return -1;
	}

	// Check for name length
	char *name = basename(strdup(path));
	size_t max_name_length = NAMEI_ENTRY_SIZE - sizeof(int) - 1;
	if(strlen(name) > max_name_length) {
		fprintf(stderr, "file name too long\n");
		errno = ENAMETOOLONG;
		return -1;
	}

	// Fetch parent inode
	struct inode *parent_inode = get_inode(get_parent_inode_id(path));
	if(parent_inode == NULL) {
		fprintf(stderr, "failed to get parent inode id\n");
		errno = ENOENT;
		return -1;
	}

	// Prepare a new data block for creating empty dir block
	struct data_block *block = data_block_alloc();
	if(block == NULL) {
		fprintf(stderr, "could not find a free data block\n");
		errno = EDQUOT;
		return -1;
	}

	// Prepare a new inode
	struct inode *inode = ialloc();
	if(inode == NULL) {
		fprintf(stderr, "could not find a free inode\n");
		errno = EDQUOT;
		return -1;
	}
	inode->type = TYPE_DIRECTORY;
	inode->mode = mode | S_IFDIR;
	inode->last_modified_file = time(NULL);
	inode->last_accessed_file = time(NULL);
	inode->last_modified_inode = time(NULL);

	// Init the new dir block
	big_int block_id = block->data_block_id;
	struct dir_block dir_block;
	if(init_dir_block(&dir_block, inode->inode_id, parent_inode->inode_id) == -1) {
		fprintf(stderr, "failed to format a dir block\n");
		return -1;
	}

	// Write the new dir block
	write_block(block_id, &dir_block, sizeof(struct dir_block));

	// Update the new inode with new dir block mapping
	if(set_block_id(inode, 0, block_id) == -1) {
		fprintf(stderr, "failed to set data block in inode\n");
		return -1;
	}

	// Write the new inode
	inode->num_blocks++;
	put_inode(inode);

	// Add new entry in parent inode
	if(add_entry_to_parent(parent_inode, inode->inode_id, name) == -1) {
		fprintf(stderr, "Failed to add entry to parent dir block\n");
		return -1;
	}

	return 0;
}

int syscalls1__mknod(const char *path, mode_t mode, dev_t dev) {

	(void) dev;
	
	// Check for existing file
	if(namei(path) != -1) {
		fprintf(stderr, "file already exists\n");
		errno = EEXIST;
		return -1;
	}

	// Check for name length
	char *name = basename(strdup(path));
	size_t max_name_length = NAMEI_ENTRY_SIZE - sizeof(int) - 1;
	if(strlen(name) > max_name_length) {
		fprintf(stderr, "file name too long\n");
		errno = ENAMETOOLONG;
		return -1;
	}

	// Fetch the parent inode
	struct inode *parent_inode = get_inode(get_parent_inode_id(path));
	if(parent_inode == NULL) {
		fprintf(stderr, "failed to get parent inode\n");
		errno = ENOENT;
		return -1;
	}

	// Prepare a new inode
	struct inode *inode = ialloc();
	if(inode == NULL) {
		fprintf(stderr, "could not find a free inode\n");
		errno = EDQUOT;
		return -1;
	}
	inode->type = TYPE_ORDINARY;
	inode->mode = mode | S_IFREG;
	inode->last_modified_file = time(NULL);
	inode->last_accessed_file = time(NULL);
	inode->last_modified_inode = time(NULL);

	// Persist the new inode
	put_inode(inode);

	// Add new entry in parent inode
	if(add_entry_to_parent(parent_inode, inode->inode_id, name) == -1) {
		fprintf(stderr, "failed to add entry to parent inode\n");
		return -1;
	}
	// Persist parent inode
	put_inode(parent_inode);

	return 0;
}

int syscalls1__readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset) {

	(void) offset;

	struct inode *inode = get_inode(namei(path));
	if(inode == NULL) {
		fprintf(stderr, "failed to get inode\n");
		errno = ENOENT;
		return -1;
	}

	big_int total_dir_blocks = inode->num_blocks;
	struct dir_block dir_block;
	int len = BLOCK_SIZE / NAMEI_ENTRY_SIZE;
	unsigned int i;
	int j;
	for(i = 0; i < total_dir_blocks; ++i) {
		big_int block_id = get_block_id(inode, i);
		if(block_id > 0 && read_block(block_id, &dir_block) == 0) {
			for(j = 0; j < len; ++j) {
				if(dir_block.inode_ids[j] != 0) {
					struct stat stat;
					stat.st_ino = dir_block.inode_ids[j];
					filler(buffer, dir_block.names[j], &stat, 0);
				}
			}
		}
	}

	return 0;
}

int syscalls1__unlink(const char *path) {

	struct inode *inode = get_inode(namei(path));
	if(inode == NULL) {
		fprintf(stderr, "failed to get inode\n");
		errno = ENOENT;
		return -1;
	}

	if(inode->type != TYPE_ORDINARY) {
		fprintf(stderr, "cannot unlink non ordinary files\n");
		return -1;
	}

	// Free inode and associated data blocks
	inode->links_nb = 0;
	if(put_inode(inode) == -1) {
		fprintf(stderr, "failed to put inode\n");
		return -1;
	}

	struct inode *parent_inode = get_inode(get_parent_inode_id(path));
	if(parent_inode == NULL) {
		fprintf(stderr, "failed to get parent inode\n");
		errno = ENOENT;
		return -1;
	}

	struct dir_block dir_block;
	unsigned int len = BLOCK_SIZE / NAMEI_ENTRY_SIZE;
	big_int total_dir_blocks = parent_inode->num_blocks;
	unsigned int i;
	for(i = 0; i < total_dir_blocks; ++i) {
		big_int block_id = get_block_id(parent_inode, i);
		if(block_id > 0 && read_block(block_id, &dir_block) == 0) {
			if(remove_entry_from_dir_block(&dir_block, inode->inode_id) == 0) {
				write_block(block_id, &dir_block, sizeof(struct dir_block));
				break;
			}
		}
	}

	if(i == len) {
		fprintf(stderr, "failed to remove entry from parent inode\n");
		return -1;
	}
	return put_inode(parent_inode);
}

int syscalls1__rmdir(const char *path) {

	struct inode *inode = get_inode(namei(path));
	if(inode == NULL) {
		fprintf(stderr, "failed to get inode\n");
		errno = ENOENT;
		return -1;
	}

	if(inode->type != TYPE_DIRECTORY) {
		fprintf(stderr, "cannot rmdir non directory files\n");
		errno = ENOTDIR;
		return -1;
	}

	// Check if it's empty
	int empty = 1;
	struct dir_block dir_block;
	unsigned int len = BLOCK_SIZE / NAMEI_ENTRY_SIZE;
	unsigned int i, j;
	big_int total_dir_blocks = inode->num_blocks;
	for(i = 0; i < total_dir_blocks; ++i) {
		big_int block_id = get_block_id(inode, i);
		if(block_id > 0 && read_block(block_id, &dir_block) == 0) {
			for(j = 0; j < len; ++j) {
				if(i == 0 && j < 2) {
					continue;
				}
				if(dir_block.inode_ids[j] != 0 || strcmp("", dir_block.names[j]) != 0) {
					empty = 0;
					break;
				}
			}
		}
	}

	if(!empty) {
		fprintf(stderr, "directory must be empty\n");
		errno = ENOTEMPTY;
		return -1;
	}

	// Free inode and associated data blocks
	inode->links_nb = 0;
	if(put_inode(inode) == -1) {
		fprintf(stderr, "failed to put inode\n");
		return -1;
	}

	struct inode *parent_inode = get_inode(get_parent_inode_id(path));
	if(parent_inode == NULL) {
		fprintf(stderr, "failed to get parent inode\n");
		errno = ENOENT;
		return -1;
	}

	total_dir_blocks = parent_inode->num_blocks;
	for(i = 0; i < total_dir_blocks; ++i) {
		big_int block_id = get_block_id(parent_inode, i);
		if(block_id > 0 && read_block(block_id, &dir_block) == 0) {
			if(remove_entry_from_dir_block(&dir_block, inode->inode_id) == 0) {
				write_block(block_id, &dir_block, sizeof(struct dir_block));
				break;
			}
		}
	}

	if(i == len) {
		fprintf(stderr, "failed to remove entry from parent inode\n");
		return -1;
	}
	return put_inode(parent_inode);
}


int syscalls1__lstat(const char *path, struct stat *buf) {

	struct inode *inode = get_inode(namei(path));
	if(inode == NULL) {
		fprintf(stderr, "failed to get inode\n");
		errno = ENOENT;
		return -1;
	}

	buf->st_ino = inode->inode_id;
	buf->st_mode = inode->mode;
	buf->st_nlink = inode->links_nb;
	buf->st_uid = inode->uid;
	buf->st_gid = inode->gid;

	if (inode->type == TYPE_ORDINARY) {
		buf->st_size = inode->num_blocks == 0 ? 0 : (inode->num_blocks - 1) * BLOCK_SIZE + inode->num_used_bytes_in_last_block;
	}
	else {
		buf->st_size = inode->num_blocks * BLOCK_SIZE;
	}

	buf->st_blksize = BLOCK_SIZE;
	buf->st_atime = inode->last_accessed_file;
	buf->st_mtime = inode->last_modified_inode;
	return 0;
}

int syscalls1__utimens(const char *path, const struct timespec tv[2]) {

	struct inode *inode = get_inode(namei(path));
	if(inode == NULL) {
		fprintf(stderr, "failed to get inode\n");
		errno = ENOENT;
		return -1;
	}

	if(tv == NULL) {
		inode->last_accessed_file = time(NULL);
		inode->last_modified_file = time(NULL);
	} else {
		inode->last_accessed_file += tv[0].tv_sec;
		inode->last_modified_file += tv[1].tv_sec;
	}
	return 0;
}

int syscalls1__chmod(const char *path, mode_t mode) {

	struct inode *inode = get_inode(namei(path));
	if(inode == NULL) {
		fprintf(stderr, "failed to get inode\n");
		errno = ENOENT;
		return -1;
	}
	
	inode->mode = mode;
	return 0;
}

int syscalls1__chown(const char *path, uid_t uid, gid_t gid) {

	struct inode *inode = get_inode(namei(path));
	if(inode == NULL) {
		fprintf(stderr, "failed to get inode\n");
		errno = ENOENT;
		return -1;
	}

	inode->uid = uid;
	inode->gid = gid;
	return 0;
}
