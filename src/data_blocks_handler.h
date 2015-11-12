#ifndef DATA_BLOCKS_HANDLER
#define DATA_BLOCKS_HANDLER

#include "common.h"


// TODO Can we just return block id?
struct data_block * data_block_alloc(void); // Allocate a new data block
struct data_block * bread(big_int data_block_nb); // Read the data block from disk
int bwrite(struct data_block *); // Write the data block to disk
int data_block_free(struct data_block *); // WARNING: this doesn't free the struct data_block. It has to be done by developer

// UTILITIES
big_int get_block_number_of_first_datablock(void);
big_int get_ith_block_number_in_datablock(char * datablock, int i);
void set_ith_block_number_in_datablock(char * datablock, int i, big_int block_number);
int has_at_least_one_datablock_number_left_without_pointer(char * datablock);
int is_datablock_full_of_free_datablock_numbers(char * datablock);
int get_ith_position_of_free_spot_in_free_datablock_number_list_for_new_free_datablock(char * datablock);
big_int get_first_free_datablock_starting_from_end_of_block_and_set_0(char * datablock);

#endif
