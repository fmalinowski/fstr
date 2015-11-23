#include "common.h"
#include "disk_emulator.h"
#include "inodes_handler.h"
#include "block_utils.h"
#include "namei.h"

static int check_data_block_for_next_entry(struct dir_block *dir_block, char *next_file) { // returns -1 if entry not found
	int len = BLOCK_SIZE / NAMEI_ENTRY_SIZE;
	int i;
	for(i = 0; i < len; i++){
		if(strcmp(next_file, dir_block->names[i]) == 0){
			int inumber = dir_block->inode_ids[i];
			LOGD("next inumber is %d", inumber);
			return inumber;
		}
	}
	return -1;
} 

int namei(const char *path) {
	LOGD("namei called. path: %s", path);
	
	if(path == NULL || path[0] != PATH_DELIMITER[0]){
		fprintf(stderr, "Invalid path name\n");
		LOGD("namei invalid path name");
		return -1;
	}

	char *next_file = strtok(strdup(path), PATH_DELIMITER);
	struct inode next_inode;
	int next_inode_success = iget(ROOT_INODE_NUMBER, &next_inode);
	struct dir_block dir_block;
	big_int block_id;

	while(next_file != NULL && next_inode_success == 0) {

		int next_inode_number = -1;
		big_int i;
		big_int len = next_inode.num_blocks;
		for(i = 0; i < len; ++i) {
			block_id = get_block_id(&next_inode, i);
			read_block(block_id, &dir_block);
			next_inode_number = check_data_block_for_next_entry(&dir_block, next_file);
			if(next_inode_number != -1) {
				break;
			}
		}

		if(next_inode_number == -1) {
			return -1;
		}

		next_file = strtok(NULL, PATH_DELIMITER);
		next_inode_success = iget(next_inode_number, &next_inode);
	}

	if(next_inode_success == -1){
		fprintf(stderr, "iget returned -1, exiting namei..\n");
		return -1;
	}
	LOGD("namei inode id: %d", next_inode.inode_id);

	return next_inode.inode_id;
}
