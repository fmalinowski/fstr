#include "syscalls1.h"
#include "common.h"
#include "inodes_handler.h"
#include "inode_table.h"
#include "data_blocks_handler.h"
#include "disk_emulator.h"
#include "block_utils.h"
#include "namei.h"

int mkdir(const char *path, mode_t mode) {
	// Verify if path is correct
	if(path == NULL || path[0] == '\0') {
		fprintf(stderr, "path is empty or null\n");
		errno = ENOENT;
		return -1;
	}

	// Prepare a new data block for creating empty dir block
	struct data_block *block = data_block_alloc();
	if(block == NULL) {
		fprintf(stderr, "could not find a free data block\n");
		errno = ENOSPC;
		return -1;
	}

	// Fetch parent inode
	struct inode *parent_inode = get_inode(get_parent_inode_id(path));
	if(parent_inode == NULL) {
		fprintf(stderr, "failed to get parent inode id\n");
		return -1;
	}

	// Prepare a new inode
	struct inode *inode = ialloc();
	if(inode == NULL) {
		fprintf(stderr, "could not find a free inode\n");
		errno = ENOSPC;
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
	if(write_block(block_id, &dir_block, sizeof(struct dir_block)) == -1) {
		fprintf(stderr, "failed to write inode to disk\n");
		return -1;
	}

	// Update the new inode with new dir block mapping
	if(set_block_id(inode, 0, block_id) == -1) {
		fprintf(stderr, "failed to set data block in inode\n");
		return -1;
	}

	// Write the new inode
	inode->num_blocks++;
	put_inode(inode);

	// Add new entry in parent inode
	if(add_entry_to_parent(parent_inode, inode->inode_id, basename(strdup(path))) == -1) {
		fprintf(stderr, "Failed to add entry to parent dir block\n");
		return -1;
	}

	return 0;
}

int mknod(const char *path, mode_t mode, dev_t dev) {
	// TODO Use dev
	(void) dev;
	
	char *name = basename(strdup(path));
	if(name == NULL || name[0] == '\0') {
		fprintf(stderr, "failed to extract basename\n");
		return -1;
	}

	// Fetch the parent inode
	struct inode *parent_inode = get_inode(get_parent_inode_id(path));
	if(parent_inode == NULL) {
		fprintf(stderr, "failed to get parent inode\n");
		return -1;
	}

	// Prepare a new inode
	struct inode *inode = ialloc();
	if(inode == NULL) {
		fprintf(stderr, "could not find a free inode\n");
		errno = ENOSPC;
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

int readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset) {
	// TODO Use offset?
	(void) offset;

	struct inode *inode = get_inode(namei(path));
	if(inode == NULL) {
		fprintf(stderr, "failed to get inode\n");
		return -1;
	}

	struct stat stat;
	stat.st_ino = inode->inode_id;
	stat.st_mode = inode->mode;
	stat.st_nlink = inode->links_nb;
	stat.st_uid = inode->uid;
	stat.st_gid = inode->gid;
	stat.st_atime = inode->last_accessed_file;
	stat.st_mtime = inode->last_modified_file;

	big_int total_dir_blocks = inode->num_blocks;
	struct dir_block dir_block;
	int len = BLOCK_SIZE / NAMEI_ENTRY_SIZE;
	unsigned int i;
	int j;
	for(i = 0; i < total_dir_blocks; ++i) {
		big_int block_id = get_block_id(inode, i);
		if(block_id > 0 && read_block(block_id, &dir_block) == 0) {
			for(j = 0; j < len; ++j) {
				if(dir_block.inode_ids[i] != 0) {
					filler(buffer, dir_block.names[i], &stat, 0);
				}
			}
		}
	}

	return 0;
}

int unlink(const char *path) {

	struct inode *inode = get_inode(namei(path));
	if(inode == NULL) {
		fprintf(stderr, "failed to get inode\n");
		return -1;
	}

	if(inode->type != TYPE_ORDINARY) {
		fprintf(stderr, "cannot unlink non ordinary files\n");
		return -1;
	}

	// Free inode and associated data blocks
	inode->links_nb = 0;
	if(iput(inode) == -1) {
		fprintf(stderr, "failed to iput inode\n");
		return -1;
	}

	struct inode *parent_inode = get_inode(get_parent_inode_id(path));
	if(parent_inode == NULL) {
		fprintf(stderr, "failed to get parent inode\n");
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
