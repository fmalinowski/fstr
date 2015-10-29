#include "blocks_handler.h"

int superblock_to_disk_format(struct superblock * sp_block, void * block);
int superblock_from_disk_format(struct superblock * sp_block, void * block);


int inode_to_disk_format(struct inode * inod, void * inode_block_format);
int inode_from_disk_format(struct inode * inod, void * inode_block_format);


int datablock_to_disk_format(struct data_block * data_b, void * block);
int datablock_from_disk_format(struct data_block * data_b, void * block);
// CODE TO BE ADDED


static int inodes_currently_in_memory;
static int* in_memory_inodes_list;


int iget(struc inode * target, int inode_number){

	/** TODO IN PHASE 3: Implement Inode fetch from buffer cache. Following code assumes there is no buffer cache.
	 * A list of inodes currently in memory is maintained.
	*/

	int block_number_of_inode = ILIST_BEGIN + (inode_number/(DATA_BLOCK_SIZE/INODE_SIZE) - 1);
	
	int inode_offset_in_block = inode_number%(DATA_BLOCK_SIZE/INODE_SIZE);
	
	// If inode is already in memory and is not locked, increment reference count and return success
	if (inode_in_memory(inode_number)) {
 		 while(is_locked(inode_number)) {
    		//TODO Implement sleep and assign target (target should be a pointer to a pointer?)
  		}
  		increment_reference_count(inode_number);
  		return 0;
	}

	//Else, read from disk
	struct data_block * inode_block = bread(block_number_of_inode);

	//block read error
	if(!inode_block){
		return -1;
	}

	/** Not sure if this cast would work (INODE_SIZE may not be equal to sizeof(struct inode), does that make a difference?) 
	 * 	Also, once a block is read in memory, we can directly point into it right? Or should we allocate new memory for
	 *  the inode? Assuming below that we don't.
	*/
	target = (struct inode *) inode_block->block[inode_offset_in_block * INODE_SIZE];

	// Initialise inode
	lock_inode(inode_number); //TODO
	increment_reference_count(inode_number); //TODO


	//update the in-memory list of inodes in memory
	if(!in_memory_inodes_list){
		in_memory_inodes_list = (int*)malloc(sizeof(int));
		inodes_currently_in_memory = 1;
		in_memory_inodes_list[0] = inode_number;
	}else{
		in_memory_inodes_list[inodes_currently_in_memory] = (int*)malloc(sizeof(int));
		in_memory_inodes_list[inodes_currently_in_memory++] = inode_number;
	}
	unlock_inode(inode_number); // TODO
	return 0; // Success
}

int iput(struct inode * inod){
	for(int i = 0; i < inodes_currently_in_memory; i++){ //find inode in memory
		if(in_memory_inodes_list[i] == inod->inode_id){
			lock_inode(inod->inode_id);  //TODO
			decrement_reference_count(inod->inode_id); // TODO
			
			if(reference_count(inod->inode_id) == 0){ // TODO
				if(inod->links_nb == 0){
					for(int j = 0; j < 16; j++){
						data_block_free(inod->direct_blocks[i]);
					}
				}
			}
		}
	}
}