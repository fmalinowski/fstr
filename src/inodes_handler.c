#include "inodes_handler.h"
#include "data_blocks_handler.h"
// ASSUMING INODE NUMBERS START FROM 1

struct inode* iget(big_int inode_number){
	struct inode * target;
	int block_number_of_inode;
	int inode_offset_in_block;
	struct data_block * inode_block;

	if(inode_number < 1 || inode_number > NUM_INODES){
		LOGD("IGET: invalid inode number");
		return NULL;
	}

	block_number_of_inode = ILIST_BEGIN + ((inode_number - 1) / (BLOCK_SIZE / INODE_SIZE));
	
	inode_offset_in_block = ((inode_number - 1) % (BLOCK_SIZE / INODE_SIZE)) * INODE_SIZE;
	//printf("block number of inode %d, inode offset in block %d", block_number_of_inode, inode_offset_in_block);
	//Read from disk
	inode_block = bread(block_number_of_inode);
	
	//block read error
	if(!inode_block){
		return NULL;
	}
 	target = (struct inode*) malloc(sizeof(struct inode));
	if(target != NULL){
		memcpy(target, &(inode_block->block[inode_offset_in_block]), sizeof(struct inode));
		return target; // Success
	}
	LOGD("IGET: target is null");
	return NULL;
}

int iput(struct inode * inod){
		struct data_block blok;
		struct data_block blok2;
		struct data_block *blok3;
		struct data_block *blok4;
		struct data_block *blok5;
		
		big_int block_number;
		big_int inode_offset_in_block;
		unsigned int i, j, k, p;

		if(inod->links_nb == 0){
			for(j = 0; j < 16; j++){
				blok.data_block_id = inod->direct_blocks[j];
				if(blok.data_block_id == 0){
					LOGD("IPUT: not freeing block 0");
					continue;
				}
				data_block_free(&blok);  // free direct data blocks
			}
			
			if(inod->single_indirect_block != 0){
				blok4 = bread(inod->single_indirect_block);
				if(blok4 != NULL){
					for(i = 0; i < BLOCK_SIZE/sizeof(big_int); i++){
						memcpy(&block_number, &(blok4->block[i*sizeof(big_int)]), sizeof(big_int));
						if(block_number != 0){
							blok2.data_block_id = block_number;
							data_block_free(&blok2);
						}
					}
					data_block_free(blok4);
				}
			}

			if(inod->double_indirect_block != 0){
				blok5 = bread(inod->double_indirect_block);
				
				if(blok5 != NULL){
					for(i = 0; i < BLOCK_SIZE/sizeof(big_int); i++){
						memcpy(&block_number, &(blok5->block[i*sizeof(big_int)]), sizeof(big_int));
						if(block_number != 0){
							blok3 = bread(block_number);
							if(blok3 != NULL){
								for(k = 0; k < BLOCK_SIZE/sizeof(big_int); k++){
									memcpy(&block_number, &(blok3->block[k*sizeof(big_int)]), sizeof(big_int));
									if(block_number != 0){
										blok2.data_block_id = block_number;
										data_block_free(&blok2);
									}
								}
								data_block_free(blok3);
							}
						}
					}
					data_block_free(blok5);
				}
			}

			if(inod->triple_indirect_block != 0){
				blok5 = bread(inod->triple_indirect_block);
				if(blok5 != NULL){
					for(i = 0; i < BLOCK_SIZE/sizeof(big_int); i++){
						memcpy(&block_number, &(blok5->block[i*sizeof(big_int)]), sizeof(big_int));
						if(block_number != 0){
							blok3 = bread(block_number);
							if(blok3 != NULL){
								for(k = 0; k < BLOCK_SIZE/sizeof(big_int); k++){
									memcpy(&block_number, &(blok3->block[k*sizeof(big_int)]), sizeof(big_int));
									if(block_number != 0){
										blok4 = bread(block_number);
								
										if(blok4 != NULL){
											for(p = 0; p < BLOCK_SIZE/sizeof(big_int); p++){
												memcpy(&block_number, &(blok4->block[p*sizeof(big_int)]), sizeof(big_int));
												if(block_number != 0){
													blok2.data_block_id = block_number;
													data_block_free(&blok2);
												}
											}
											data_block_free(blok4);
										}
							
										else{
											//printf("blo4 is null");
										}
									}
								}
								data_block_free(blok3);
							}
						}
					}
					data_block_free(blok5);
				}
			}
			ifree(inod); //free inode
			return 0; // Success
		}
		else{
			// Else, write the inode block to disk to save changes
			blok3 = bread(ILIST_BEGIN + ((inod->inode_id - 1)/(BLOCK_SIZE/INODE_SIZE)));
			if(blok3 == NULL){
				return -1;
			}
			inode_offset_in_block = (inod->inode_id - 1) % (BLOCK_SIZE/INODE_SIZE);	
			memcpy(&(blok3->block[inode_offset_in_block]), inod, sizeof(struct inode));
			if(bwrite(blok3) == 0){
				return 0; // Success	
			}
			else{
				LOGD("IPUT: bwrite was unsuccessful");
				return -1;
			}
		}
		LOGD("IPUT: something went wrong");
		return -1;
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
	superblock.commit();
	inod->links_nb = 1;

	// Update the inode on disk
	blok3 = bread(ILIST_BEGIN + ((inod->inode_id - 1) / (BLOCK_SIZE / INODE_SIZE)));
	
	inode_offset_in_block = ((inod->inode_id - 1) % (BLOCK_SIZE / INODE_SIZE)) * INODE_SIZE;	
	inod->type = TYPE_ORDINARY;
	memcpy(&(blok3->block[inode_offset_in_block]), inod, sizeof(struct inode));
	
	if(bwrite(blok3) == 0){
		return inod;
	}
	LOGD("IPUT: bwrite was unsuccessful");
	return NULL;
}

int ifree(struct inode * inod){
	
	struct data_block *blok3;
	int inode_offset_in_block;

	superblock.num_free_inodes++;
	superblock.commit();

	inod->type = TYPE_FREE;

	blok3 = bread(ILIST_BEGIN + ((inod->inode_id - 1) / (BLOCK_SIZE / INODE_SIZE)));
	inode_offset_in_block = ((inod->inode_id - 1) % (BLOCK_SIZE / INODE_SIZE)) * INODE_SIZE;
	memcpy(&(blok3->block[inode_offset_in_block]), inod, sizeof(struct inode));
	
	if(bwrite(blok3) == 0){
		return 0;
	}
	LOGD("IFREE: bwrite was unsuccessful");
	return -1;
}
