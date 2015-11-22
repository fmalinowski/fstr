#include "inodes_handler.h"
#include "data_blocks_handler.h"
#include "block_utils.h"
// ASSUMING INODE NUMBERS START FROM 1

struct inode* iget(int inode_number){
	struct inode * target;
	int block_number_of_inode;
	int inode_offset_in_block;
	struct data_block * inode_block;

	if(inode_number < 1 || inode_number > NUM_INODES){
		LOGD("IGET: invalid inode number %d", inode_number);
		return NULL;
	}
	//printf("going to get inode number %d", (int)inode_number);
	block_number_of_inode = ILIST_BEGIN + ((inode_number - 1) / (BLOCK_SIZE / INODE_SIZE));
	
	inode_offset_in_block = ((inode_number - 1) % (BLOCK_SIZE / INODE_SIZE)) * INODE_SIZE;
	//printf("block number of inode %d, inode offset in block %d", block_number_of_inode, inode_offset_in_block);
	//Read from disk
	inode_block = bread(block_number_of_inode);
	//printf("block number read is %d", (int)inode_block->data_block_id);
	//block read error
	if(!inode_block){
		return NULL;
	}
 	target = (struct inode*) malloc(sizeof(struct inode));
	if(target != NULL){
		memcpy(target, &(inode_block->block[inode_offset_in_block]), sizeof(struct inode));
		LOGD("returning target inode id %d", target->inode_id);
		return target; // Success
	}
	LOGD("IGET: target is null");
	return NULL;
}

int iput(struct inode * inod) {
	LOGD("iput inode id: %d", inod->inode_id);
	if(inod->links_nb == 0) {
		// Free the inode and data blocks
		struct data_block data_block;
		big_int total_blocks = inod->num_blocks;
		big_int i;
		for(i = 0; i < total_blocks; ++i) {
			big_int block_id = get_block_id(inod, i);
			if(block_id > 0) {
				data_block.data_block_id = block_id;
				data_block_free(&data_block);
			}
		}
		
		return ifree(inod);
	}

	// Else, write the inode block to disk to save changes
	struct data_block *blok = bread(ILIST_BEGIN + ((inod->inode_id - 1)/(BLOCK_SIZE/INODE_SIZE)));
	if(blok == NULL){
		fprintf(stderr, "failed to read block\n");
		return -1;
	}
	big_int inode_offset_in_block = ((inod->inode_id - 1) % (BLOCK_SIZE/INODE_SIZE)) * INODE_SIZE;
	memcpy(&(blok->block[inode_offset_in_block]), inod, sizeof(struct inode));
	return bwrite(blok);
}

int next_free_inode_number(void){ // It's correctness depends on how mkfs organizes stuff
	
	struct data_block *blok;
	struct inode *inod;
	int i, j;
	int total_blocks_for_inodes = NUM_INODE_BLOCKS;
	inod = (struct inode*)malloc(sizeof(struct inode));
	for(i = 1; i <= total_blocks_for_inodes; i++){
		blok = bread(i);
		for(j = 0; j < BLOCK_SIZE/INODE_SIZE; j++){
			memcpy(inod, &(blok->block[j*INODE_SIZE]), sizeof(struct inode));
			if(inod->type == TYPE_FREE){
				return inod->inode_id;
			}
		}
	}
	return -1;
}

struct inode* ialloc(void){  // THIS DOES NOT SET THE FILETYPE OF INODE. MUST BE DONE AT LAYER 2
	
	struct inode *inod;
	struct data_block *blok3;
	int free_inode_number;
	int inode_offset_in_block;
	
	free_inode_number = next_free_inode_number(); 
	if(free_inode_number == -1){
		LOGD("IALLOC: free inode not found");
		return NULL;
	}
	
	inod = iget(free_inode_number);
	
	superblock.num_free_inodes--; // decrease count of number of free inodes in the file system
	commit_superblock();
	inod->links_nb = 1;

	// Update the inode on disk
	blok3 = bread(ILIST_BEGIN + ((inod->inode_id - 1) / (BLOCK_SIZE / INODE_SIZE)));
	
	inode_offset_in_block = ((inod->inode_id - 1) % (BLOCK_SIZE / INODE_SIZE)) * INODE_SIZE;	
	inod->type = TYPE_ORDINARY;
	memcpy(&(blok3->block[inode_offset_in_block]), inod, sizeof(struct inode));
	
	if(bwrite(blok3) == 0){
		if(commit_superblock() == 0){
			LOGD("ialloc returning inode id: %d", inod->inode_id);
			return inod;
		}
		else{
			LOGD("IPUT: superblock commit was unsuccessful");		
			return NULL;
		}
	}
	LOGD("IPUT: bwrite was unsuccessful");
	return NULL;
}

int ifree(struct inode * inod){
	
	struct data_block *blok3;
	int inode_offset_in_block;

	superblock.num_free_inodes++;
	commit_superblock();

	struct inode fresh_inode = {
        .inode_id = inod->inode_id,
        .type = TYPE_FREE
    };

	blok3 = bread(ILIST_BEGIN + ((fresh_inode.inode_id - 1) / (BLOCK_SIZE / INODE_SIZE)));
	inode_offset_in_block = ((fresh_inode.inode_id - 1) % (BLOCK_SIZE / INODE_SIZE)) * INODE_SIZE;
	memcpy(&(blok3->block[inode_offset_in_block]), &fresh_inode, sizeof(struct inode));
	
	if(bwrite(blok3) == 0){
		if(commit_superblock() == 0){
			return 0;
		}
		else{
			LOGD("IPUT: superblock commit was unsuccessful");		
			return -1;
		}	
	}
	LOGD("IFREE: bwrite was unsuccessful");
	return -1;
}

void free_inode(struct inode * inod) {		
	free(inod);		
}
