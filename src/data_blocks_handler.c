#include "common.h"
#include "disk_emulator.h"
#include "data_blocks_handler.h"


big_int get_block_number_of_first_datablock(void) {
	return 1 + NUM_INODE_BLOCKS;
}

// first block number is when i = 1
big_int get_ith_block_number_in_datablock(char * datablock, int i) {
	// a block number is encoded on 4 bytes (size of an integer)
	big_int block_number;
	memcpy(&block_number, &datablock[(i-1) * sizeof(big_int)], sizeof(big_int));
	return block_number;
}

// first block number is when i = 1
void set_ith_block_number_in_datablock(char * datablock, int i, big_int block_number) {
	// a block number is encoded on 4 bytes (size of an integer)
	memcpy(&datablock[(i-1) * sizeof(big_int)], &block_number, sizeof(big_int));
}

// Returns 0 if there is only the pointer to the next datablock containing IDs of free datablocks) or if there is no pointer.
int has_at_least_one_datablock_number_left_without_pointer(char * datablock) {
	big_int block_number;
	int i;

	for (i = BLOCK_ID_LIST_LENGTH; i >= 2; i--) {
		memcpy(&block_number, &datablock[(i-1) * sizeof(big_int)], sizeof(big_int));

		if (block_number != 0) {
			return 1;
		}
	}
	return 0;
}

int is_datablock_full_of_free_datablock_numbers(char * datablock) {
	big_int block_number;
	int i;

	for (i = 2; i <= (int)BLOCK_ID_LIST_LENGTH; i++) {
		memcpy(&block_number, &datablock[(i-1) * sizeof(big_int)], sizeof(big_int));

		if (block_number == 0) {
			return 0;
		}
	}
	return 1;
}

int get_ith_position_of_free_spot_in_free_datablock_number_list_for_new_free_datablock(char * datablock) {
	// a block number is encoded on 4 bytes (size of an integer)
	big_int block_number;
	int i;

	for (i = 2; (big_int)i <= BLOCK_ID_LIST_LENGTH; i++) {
		memcpy(&block_number, &datablock[(i-1) * sizeof(big_int)], sizeof(big_int));

		if (block_number == 0) {
			return i;
		}
	}
	return -1;
}

// Browse the block numbers in datablock from right to left and stop when it find a non zero block number. 
// (It avoid the 1st block number which is a pointer to next datablock containing free datablock numbers)
// It returns it to the user and set 0 at the block number position
big_int get_first_free_datablock_starting_from_end_of_block_and_set_0(char * datablock) {
	int number_of_block_numbers, i;
	big_int block_number;

	number_of_block_numbers = BLOCK_ID_LIST_LENGTH;
	for (i = number_of_block_numbers; i > 1; i--) {
		block_number = get_ith_block_number_in_datablock(datablock, i);
		
		if (block_number != 0) {
			set_ith_block_number_in_datablock(datablock, i, 0);
			return block_number;
		}
	}

	return 0;
}

struct data_block * data_block_alloc(void) {
	struct data_block * datablock;

	int position_of_first_datablock;
	big_int free_block_number_to_be_used;
	char read_buffer[BLOCK_SIZE];
	
	// We don't implement lock here for superblock !

	
	// 	We get 1st datablock. That's also the block that contains the id of the free data blocks and 
	// 	the pointer to the next data block containing other part of free data block numbers!
	
	position_of_first_datablock = get_block_number_of_first_datablock();
	free_block_number_to_be_used = -1;
	
	// Read the datablock containing normally some of the free data block numbers.
	if (read_block(position_of_first_datablock, read_buffer) == -1) {
		return NULL;
	}

	if (has_at_least_one_datablock_number_left_without_pointer(read_buffer) == 0) {
		// No block numbers left, we copy content of datablock pointed by current block into current block

		// We check if there's a block pointed by current block. If not, we return an error (NULL)
		big_int next_block_number = get_ith_block_number_in_datablock(read_buffer, 1);
		
		if (next_block_number == 0) {
			return NULL;
		}

		if (read_block(next_block_number, read_buffer) == -1) {
			return NULL;
		}

		if (write_block(position_of_first_datablock, read_buffer, BLOCK_SIZE) == -1) {
			return NULL;
		}

		// The block that was pointed by 1st data block will be used as the free data block
		free_block_number_to_be_used = next_block_number;
	}
	else {
		free_block_number_to_be_used = get_first_free_datablock_starting_from_end_of_block_and_set_0(read_buffer);
		write_block(position_of_first_datablock, read_buffer, BLOCK_SIZE);
	}

	datablock = (struct data_block *) malloc(sizeof(struct data_block));
	datablock->data_block_id = free_block_number_to_be_used;
	memset(datablock->block, 0, BLOCK_SIZE); // set 0s to the buffer
	write_block(free_block_number_to_be_used, datablock->block, BLOCK_SIZE); // Set 0s the block on disk too

	superblock.num_free_blocks--; // Decrement the number of free data blocks
	superblock.commit();

	return datablock;
}

struct data_block * bread(big_int data_block_nb) {
	struct data_block * datablock;

	datablock = (struct data_block *) malloc(sizeof(struct data_block));
	if (read_block(data_block_nb, datablock->block) == -1) {
		free(datablock);
		return NULL;
	}

	datablock->data_block_id = data_block_nb;
	return datablock;
}

int bwrite(struct data_block * datablock) {
	return write_block(datablock->data_block_id, datablock->block, BLOCK_SIZE);
}

int data_block_free(struct data_block * datablock) {
	int position_of_first_datablock, ith_position;
	char read_buffer[BLOCK_SIZE], buffer[BLOCK_SIZE];

	// We don't implement lock here for superblock!

	// Look at first datablock and see if it's full (full of free datablock numbers).
	position_of_first_datablock = get_block_number_of_first_datablock();

	// Read the datablock containing normally some of the free data block numbers.
	if (read_block(position_of_first_datablock, read_buffer) == -1) {
		return -1;
	}

	if (is_datablock_full_of_free_datablock_numbers(read_buffer) == 1) {
		// We copy the content of the 1st datablock (containing free datablock numbers) into the new free datablock
		// Then we clear the 1st datablock number and set the 1st block number (pointer) to be the number of the new free datablock

		if (write_block(datablock->data_block_id, read_buffer, BLOCK_SIZE) == -1) {
			return -1;
		}

		memset(buffer, 0, BLOCK_SIZE); // We sets 0s in the whole block
		set_ith_block_number_in_datablock(buffer, 1, datablock->data_block_id);
		if (write_block(position_of_first_datablock, buffer, BLOCK_SIZE) == -1) {
			return -1;
		}
	}
	else {
		// We add the new free datablock number at the 1st empty spot in the list of datablock numbers in the 1st datablock

		memset(datablock->block, 0, BLOCK_SIZE); // We sets 0s in the whole new datablock
		if (write_block(datablock->data_block_id, datablock->block, BLOCK_SIZE) == -1) {
			return -1;
		}

		ith_position = get_ith_position_of_free_spot_in_free_datablock_number_list_for_new_free_datablock(read_buffer);
		set_ith_block_number_in_datablock(read_buffer, ith_position, datablock->data_block_id);
		if (write_block(position_of_first_datablock, read_buffer, BLOCK_SIZE) == -1) {
			return -1;
		}
	}

	superblock.num_free_blocks++;
	superblock.commit();

	return 0;
}

void free_data_block_pointer(struct data_block * db) {
	free(db);
}
