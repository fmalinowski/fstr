#include <string.h>
#include <strings.h>

#include "unity.h"
#include "unity_fixture.h"

#include "common.h"
#include "disk_emulator.h"
#include "data_blocks_handler.h"


TEST_GROUP_RUNNER(TestDataBlocksHandler) {
	RUN_TEST_CASE(TestDataBlocksHandler, get_block_number_of_first_datablock);
	RUN_TEST_CASE(TestDataBlocksHandler, get_ith_block_number_in_datablock__returns_ID_of_datablock_placed_in_ith_position_in_datablock);
	RUN_TEST_CASE(TestDataBlocksHandler, set_ith_block_number_in_datablock__sets_datablock_ID_at_ith_position_in_datablock);
	RUN_TEST_CASE(TestDataBlocksHandler, has_at_least_one_datablock_number_left_without_pointer__returns_0_if_there_is_only_pointer_to_next_datablock_or_no_pointer_at_all);
	RUN_TEST_CASE(TestDataBlocksHandler, shift_datablock_numbers_in_buffer_to_left_except_pointer_to_next_block);
	RUN_TEST_CASE(TestDataBlocksHandler, data_block_alloc__allocates_correctly_a_datablock);
	RUN_TEST_CASE(TestDataBlocksHandler, is_datablock_full_of_free_datablock_numbers);
	RUN_TEST_CASE(TestDataBlocksHandler, get_ith_position_of_free_spot_in_free_datablock_number_list_for_new_free_datablock);
	RUN_TEST_CASE(TestDataBlocksHandler, data_block_free);
	RUN_TEST_CASE(TestDataBlocksHandler, bread);
	RUN_TEST_CASE(TestDataBlocksHandler, bwrite);
}




TEST_GROUP(TestDataBlocksHandler);


// To be executed before each test case
TEST_SETUP(TestDataBlocksHandler) {
}

// To be executed after each test
TEST_TEAR_DOWN(TestDataBlocksHandler) {
}


TEST(TestDataBlocksHandler, get_block_number_of_first_datablock) {
	// BLOCK_SIZE = 4096
	// NUM_INODES = 820
	// INODE_SIZE = 256

	TEST_ASSERT_EQUAL(821, get_block_number_of_first_datablock());
}

TEST(TestDataBlocksHandler, get_ith_block_number_in_datablock__returns_ID_of_datablock_placed_in_ith_position_in_datablock) {
	char datablock[BLOCK_SIZE];

	bzero(datablock, BLOCK_SIZE); // Write zeros everywhere in block as it should be when block is empty

	big_int block_number_1_to_be_copied = 1039;
	big_int block_number_2_to_be_copied = 23567;
	big_int block_number_3_to_be_copied = 2;

	memcpy(&datablock[0], &block_number_1_to_be_copied, sizeof(big_int));
	memcpy(&datablock[8], &block_number_2_to_be_copied, sizeof(big_int));
	memcpy(&datablock[16], &block_number_3_to_be_copied, sizeof(big_int));

	TEST_ASSERT_EQUAL(block_number_1_to_be_copied, get_ith_block_number_in_datablock(datablock, 1));
	TEST_ASSERT_EQUAL(block_number_2_to_be_copied, get_ith_block_number_in_datablock(datablock, 2));
	TEST_ASSERT_EQUAL(block_number_3_to_be_copied, get_ith_block_number_in_datablock(datablock, 3));
}

TEST(TestDataBlocksHandler, set_ith_block_number_in_datablock__sets_datablock_ID_at_ith_position_in_datablock) {
	char datablock[BLOCK_SIZE];

	bzero(datablock, BLOCK_SIZE); // Write zeros everywhere in block as it should be when block is empty

	big_int block_number_1_to_be_copied = 1039;
	big_int block_number_2_to_be_copied = 23567;
	big_int block_number_3_to_be_copied = 2;

	set_ith_block_number_in_datablock(datablock, 1, block_number_1_to_be_copied);
	set_ith_block_number_in_datablock(datablock, 2, block_number_2_to_be_copied);
	set_ith_block_number_in_datablock(datablock, 3, block_number_3_to_be_copied);

	TEST_ASSERT_EQUAL(block_number_1_to_be_copied, get_ith_block_number_in_datablock(datablock, 1));
	TEST_ASSERT_EQUAL(block_number_2_to_be_copied, get_ith_block_number_in_datablock(datablock, 2));
	TEST_ASSERT_EQUAL(block_number_3_to_be_copied, get_ith_block_number_in_datablock(datablock, 3));
}

TEST(TestDataBlocksHandler, has_at_least_one_datablock_number_left_without_pointer__returns_0_if_there_is_only_pointer_to_next_datablock_or_no_pointer_at_all) {
	char datablock[BLOCK_SIZE];

	bzero(datablock, BLOCK_SIZE); // Write zeros everywhere in block as it should be when block is empty

	big_int pointer_to_next_datablock_containing_IDs_of_free_datablocks = 1039;
	big_int block_number_1 = 23567;

	set_ith_block_number_in_datablock(datablock, 1, pointer_to_next_datablock_containing_IDs_of_free_datablocks);

	TEST_ASSERT_EQUAL(0, has_at_least_one_datablock_number_left_without_pointer(datablock));

	set_ith_block_number_in_datablock(datablock, 2, block_number_1);
	TEST_ASSERT_EQUAL(1, has_at_least_one_datablock_number_left_without_pointer(datablock));
}

TEST(TestDataBlocksHandler, shift_datablock_numbers_in_buffer_to_left_except_pointer_to_next_block) {
	char datablock[BLOCK_SIZE];
	int number_of_block_numbers_in_datablock;

	bzero(datablock, BLOCK_SIZE); // Write zeros everywhere in block as it should be when block is empty
	number_of_block_numbers_in_datablock = BLOCK_SIZE / sizeof(big_int);

	// We set the position of the block number as the block number in the buffer for the purpose of the test
	int block_number;
	for (block_number = 1; block_number <= number_of_block_numbers_in_datablock; block_number++) {
		set_ith_block_number_in_datablock(datablock, block_number, (big_int)block_number);
	}

	shift_datablock_numbers_in_buffer_to_left_except_pointer_to_next_block(datablock);

	// Assert 1st block didn't change
	TEST_ASSERT_EQUAL(1, get_ith_block_number_in_datablock(datablock, 1));

	// Assert all the other blocks left shifted (block number 2 disappeared)
	int i;
	for (i = 3; i <= number_of_block_numbers_in_datablock; i++) {
		TEST_ASSERT_EQUAL(i, get_ith_block_number_in_datablock(datablock, i-1));
	}

	// Make sure that last spot for a block number is empty (only 0s)
	TEST_ASSERT_EQUAL(0, get_ith_block_number_in_datablock(datablock, number_of_block_numbers_in_datablock));
}

TEST(TestDataBlocksHandler, data_block_alloc__allocates_correctly_a_datablock) {
	int first_datablock_position;
	big_int first_free_datablock_number, second_free_datablock_number, pointer_next_free_datablock_list;
	big_int first_free_datablock_number_in_second_list;
	char datablock_buffer[BLOCK_SIZE];
	char datablock_buffer2[BLOCK_SIZE];
	char free_block_buffer[BLOCK_SIZE];

	struct data_block * result_datablock;

	// BEGINNING OF SETUP for the test
	init_disk_emulator();

	superblock.num_free_blocks = 560;

	first_datablock_position = get_block_number_of_first_datablock();

	pointer_next_free_datablock_list = first_datablock_position + 7; // These are random but they have to be after 1st datablock
	first_free_datablock_number = first_datablock_position + 4; 
	second_free_datablock_number = first_datablock_position + 2;

	memset(datablock_buffer, 0, BLOCK_SIZE); // Set 0s to the datablock before setting datablock numbers

	// We set here the IDs of the free datablocks in the superblock free datablock list
	// Set pointer to next block
	set_ith_block_number_in_datablock(datablock_buffer, 1, pointer_next_free_datablock_list);

	// Set 1st free datablock number
	set_ith_block_number_in_datablock(datablock_buffer, 2, first_free_datablock_number);
	set_ith_block_number_in_datablock(datablock_buffer, 3, second_free_datablock_number);

	// We write to disk the datablock containing the list of free datablocks
	write_block(first_datablock_position, datablock_buffer, BLOCK_SIZE);

	// We write the IDs of the free datablocks in the datablock that is pointed by the 1st (superblock free datablock list) datablock
	memset(datablock_buffer2, 0, BLOCK_SIZE); // Set 0s to the datablock before setting datablock numbers
	first_free_datablock_number_in_second_list = first_datablock_position + 11;
	set_ith_block_number_in_datablock(datablock_buffer2, 1, 0); // We say that there's no pointer to a 3rd datablock.
	set_ith_block_number_in_datablock(datablock_buffer2, 2, first_free_datablock_number_in_second_list);

	// We write to disk the datablock containing the 2nd list of free datablocks
	write_block(pointer_next_free_datablock_list, datablock_buffer2, BLOCK_SIZE);

	// We write a bunch of random stuff in the blocks that are supposed to be free to see if the data will be cleared
	strcpy(free_block_buffer, "Hey bro! How is it going?");
	write_block(first_free_datablock_number, free_block_buffer, BLOCK_SIZE);
	write_block(second_free_datablock_number, free_block_buffer, BLOCK_SIZE);
	write_block(first_free_datablock_number_in_second_list, free_block_buffer, BLOCK_SIZE);

	// END OF SETUP

	TEST_ASSERT_EQUAL(560, superblock.num_free_blocks);

	// We assert that we get the right free datablock numbers during the allocation
	result_datablock = data_block_alloc();
	TEST_ASSERT_EQUAL(first_free_datablock_number, result_datablock->data_block_id);
	read_block(first_free_datablock_number, free_block_buffer); // We want to check whether the block got cleared on disk
	TEST_ASSERT_EQUAL(0, free_block_buffer[0]); // Assert that block got cleared
	TEST_ASSERT_EQUAL(0, free_block_buffer[1]);
	TEST_ASSERT_EQUAL(559, superblock.num_free_blocks);

	result_datablock = data_block_alloc();
	TEST_ASSERT_EQUAL(second_free_datablock_number, result_datablock->data_block_id);
	read_block(second_free_datablock_number, free_block_buffer); // We want to check whether the block got cleared on disk
	TEST_ASSERT_EQUAL(0, free_block_buffer[0]); // Assert that block got cleared
	TEST_ASSERT_EQUAL(0, free_block_buffer[1]);
	TEST_ASSERT_EQUAL(558, superblock.num_free_blocks);

	// Now that should be the block that was containing the 2nd part of the list of free datablock IDS that should be given back
	result_datablock = data_block_alloc();
	TEST_ASSERT_EQUAL_MESSAGE(pointer_next_free_datablock_list, result_datablock->data_block_id, "WASFD");
	read_block(pointer_next_free_datablock_list, free_block_buffer); // We want to check whether the block got cleared on disk
	TEST_ASSERT_EQUAL(0, free_block_buffer[0]); // Assert that block got cleared
	TEST_ASSERT_EQUAL(0, free_block_buffer[1]);
	TEST_ASSERT_EQUAL(557, superblock.num_free_blocks);

	result_datablock = data_block_alloc();
	TEST_ASSERT_EQUAL(first_free_datablock_number_in_second_list, result_datablock->data_block_id);
	read_block(first_free_datablock_number_in_second_list, free_block_buffer); // We want to check whether the block got cleared on disk
	TEST_ASSERT_EQUAL(0, free_block_buffer[0]); // Assert that block got cleared
	TEST_ASSERT_EQUAL(0, free_block_buffer[1]);
	TEST_ASSERT_EQUAL(556, superblock.num_free_blocks);

	// At this point we have no more free block numbers stored so we should get NULL
	result_datablock = data_block_alloc();
	TEST_ASSERT_NULL(result_datablock);
	TEST_ASSERT_EQUAL(556, superblock.num_free_blocks);

	free_disk_emulator();
}

TEST(TestDataBlocksHandler, is_datablock_full_of_free_datablock_numbers) {
	char full_datablock[BLOCK_SIZE], almost_full_datablock[BLOCK_SIZE], empty_datablock[BLOCK_SIZE];
	int i;

	// We set all the datablocks with 0s initially
	memset(full_datablock, 0, BLOCK_SIZE);
	memset(almost_full_datablock, 0, BLOCK_SIZE);
	memset(empty_datablock, 0, BLOCK_SIZE);
	
	// We make one datablock completely full
	for (i = 1; (i * sizeof(big_int)) <= BLOCK_SIZE; i++) {
		set_ith_block_number_in_datablock(full_datablock, i, 122); // 122 is a block number. We don't care what it is
	}

	// We make one datablock almost completely full (last spot is empty)
	for (i = 1; (i * sizeof(big_int)) <= (BLOCK_SIZE - sizeof(big_int)); i++) {
		set_ith_block_number_in_datablock(almost_full_datablock, i, 122); // 122 is a block number. We don't care what it is
	}

	TEST_ASSERT_EQUAL(1, is_datablock_full_of_free_datablock_numbers(full_datablock));
	TEST_ASSERT_EQUAL(0, is_datablock_full_of_free_datablock_numbers(almost_full_datablock));
	TEST_ASSERT_EQUAL(0, is_datablock_full_of_free_datablock_numbers(empty_datablock));
}

TEST(TestDataBlocksHandler, get_ith_position_of_free_spot_in_free_datablock_number_list_for_new_free_datablock) {
	char full_datablock[BLOCK_SIZE], almost_full_datablock[BLOCK_SIZE], 
		free_spots_datablock[BLOCK_SIZE], empty_datablock[BLOCK_SIZE];
	int i, last_ith_filled, last_block_rank_position;

	// We set all the datablocks with 0s initially
	memset(full_datablock, 0, BLOCK_SIZE);
	memset(almost_full_datablock, 0, BLOCK_SIZE);
	memset(free_spots_datablock, 0, BLOCK_SIZE);
	memset(empty_datablock, 0, BLOCK_SIZE);

	// We make one datablock completely full
	for (i = 1; (i * sizeof(big_int)) <= BLOCK_SIZE; i++) {
		set_ith_block_number_in_datablock(full_datablock, i, 122); // 122 is a block number. We don't care what it is
	}

	// We make one datablock almost completely full (last spot is empty)
	last_block_rank_position = 512; // 4096/8 = BLOCK_SIZE / sizeof(big_int)
	for (i = 1; (i * sizeof(big_int)) <= (BLOCK_SIZE - sizeof(big_int)); i++) {
		set_ith_block_number_in_datablock(almost_full_datablock, i, 122); // 122 is a block number. We don't care what it is
	}

	// We make one datablock half filled up to a certain position
	last_ith_filled = 120;

	for (i = 1; (i * sizeof(big_int)) <= (last_ith_filled * sizeof(big_int)); i++) {
		set_ith_block_number_in_datablock(free_spots_datablock, i, 122); // 122 is a block number. We don't care what it is
	}

	TEST_ASSERT_EQUAL(-1, get_ith_position_of_free_spot_in_free_datablock_number_list_for_new_free_datablock(full_datablock));
	TEST_ASSERT_EQUAL(last_block_rank_position, get_ith_position_of_free_spot_in_free_datablock_number_list_for_new_free_datablock(almost_full_datablock));
	TEST_ASSERT_EQUAL((last_ith_filled + 1), get_ith_position_of_free_spot_in_free_datablock_number_list_for_new_free_datablock(free_spots_datablock));
	TEST_ASSERT_EQUAL(1, get_ith_position_of_free_spot_in_free_datablock_number_list_for_new_free_datablock(empty_datablock));
}

TEST(TestDataBlocksHandler, data_block_free) {
	int i;
	int first_datablock_position, pointer_to_second_datablock, pointer_to_third_datablock;
	char first_datablock[BLOCK_SIZE], second_datablock[BLOCK_SIZE];
	char read_buffer[BLOCK_SIZE];

	// BEGINNING OF SETUP for the test
	init_disk_emulator();

	superblock.num_free_blocks = 1057;


	first_datablock_position = get_block_number_of_first_datablock();

	pointer_to_second_datablock = 9001; // These are random but they have to be after 1st datablock

	// We make the 1st datablock almost full in terms of datablock number (there's room just for a last datablock number)
	memset(first_datablock, 0, BLOCK_SIZE); // Set 0s to the datablock before setting datablock numbers
	for (i = 1; (i * sizeof(big_int)) <= (BLOCK_SIZE - sizeof(big_int)); i++) {
		set_ith_block_number_in_datablock(first_datablock, i, i);
	}
	set_ith_block_number_in_datablock(first_datablock, 1, 9001); // Set pointer to second datablock

	// We write to disk the datablock containing the list of free datablocks
	write_block(first_datablock_position, first_datablock, BLOCK_SIZE);

	pointer_to_third_datablock = 10053;
	memset(second_datablock, 0, BLOCK_SIZE); // Set 0s to the datablock before setting datablock numbers
	set_ith_block_number_in_datablock(second_datablock, 1, pointer_to_third_datablock); // Set pointer to third datablock (we won't create it)
	set_ith_block_number_in_datablock(second_datablock, 2, 8567); // Set a random free datablock number in second datablock list

	// We write to disk the datablock containing the list of free datablocks
	write_block(pointer_to_second_datablock, second_datablock, BLOCK_SIZE);

	// END OF SETUP


	// We free a datablock
	struct data_block datablock_to_free_1;
	datablock_to_free_1.data_block_id = 8046;
	memset(datablock_to_free_1.block, 1, BLOCK_SIZE); // Put random content in datablock to be freed

	TEST_ASSERT_EQUAL(0, data_block_free(&datablock_to_free_1));
	TEST_ASSERT_EQUAL(1058, superblock.num_free_blocks);

	// Make sure 1st datablock contain right datablock numbers
	read_block(first_datablock_position, read_buffer);
	TEST_ASSERT_EQUAL(pointer_to_second_datablock, get_ith_block_number_in_datablock(read_buffer, 1));
	TEST_ASSERT_EQUAL(2, get_ith_block_number_in_datablock(read_buffer, 2));
	TEST_ASSERT_EQUAL(3, get_ith_block_number_in_datablock(read_buffer, 3));
	TEST_ASSERT_EQUAL(8046, get_ith_block_number_in_datablock(read_buffer, BLOCK_SIZE/sizeof(big_int))); // 4096/8 = BLOCK_SIZE/sizeof(big_int) = last blocknumber

	// Make sure 2nd datablock contain the 2 right datablock numbers (pointer to 3rd datablock and another datablock number)
	read_block(pointer_to_second_datablock, read_buffer);
	TEST_ASSERT_EQUAL(pointer_to_third_datablock, get_ith_block_number_in_datablock(read_buffer, 1));
	TEST_ASSERT_EQUAL(8567, get_ith_block_number_in_datablock(read_buffer, 2));

	// We make sure the datablock that was freed has only 0s in its content
	read_block(8046, read_buffer);
	for (i = 0; i < BLOCK_SIZE; i++) {
		TEST_ASSERT_EQUAL(0, (int)read_buffer[i]);
	}



	// We free another datablock
	struct data_block datablock_to_free_2;
	datablock_to_free_2.data_block_id = 8053;
	memset(datablock_to_free_1.block, 1, BLOCK_SIZE); // Put random content in datablock to be freed

	// We free a datablock
	TEST_ASSERT_EQUAL(0, data_block_free(&datablock_to_free_2));
	TEST_ASSERT_EQUAL(1059, superblock.num_free_blocks);

	// Make sure 1st datablock contain only a pointer to the newly freed datablock and the pointed datablock contain what was in the 1st datablock
	read_block(first_datablock_position, read_buffer);
	TEST_ASSERT_EQUAL(8053, get_ith_block_number_in_datablock(read_buffer, 1));
	TEST_ASSERT_EQUAL(0, get_ith_block_number_in_datablock(read_buffer, 2));
	TEST_ASSERT_EQUAL(0, get_ith_block_number_in_datablock(read_buffer, 3));
	TEST_ASSERT_EQUAL(0, get_ith_block_number_in_datablock(read_buffer, BLOCK_SIZE/sizeof(big_int)));

	// Make sure the newly freed datablock contains datablock list that was contained in the 1st datablock before we freed the new datablock
	read_block(8053, read_buffer);
	TEST_ASSERT_EQUAL(pointer_to_second_datablock, get_ith_block_number_in_datablock(read_buffer, 1));
	TEST_ASSERT_EQUAL(2, get_ith_block_number_in_datablock(read_buffer, 2));
	TEST_ASSERT_EQUAL(3, get_ith_block_number_in_datablock(read_buffer, 3));
	TEST_ASSERT_EQUAL(8046, get_ith_block_number_in_datablock(read_buffer, BLOCK_SIZE/sizeof(big_int)));

	// Make sure 3rd datablock contain teh remaining datablock numbers
	read_block(pointer_to_second_datablock, read_buffer);
	TEST_ASSERT_EQUAL(pointer_to_third_datablock, get_ith_block_number_in_datablock(read_buffer, 1));
	TEST_ASSERT_EQUAL(8567, get_ith_block_number_in_datablock(read_buffer, 2));



	// We free another datablock
	struct data_block datablock_to_free_3;
	datablock_to_free_3.data_block_id = 8078;
	memset(datablock_to_free_3.block, 1, BLOCK_SIZE); // Put random content in datablock to be freed

	// We free a datablock
	TEST_ASSERT_EQUAL(0, data_block_free(&datablock_to_free_3));
	TEST_ASSERT_EQUAL(1060, superblock.num_free_blocks);

	// Make sure 1st datablock contain only a pointer to next datablock and the newly added free datablock
	read_block(first_datablock_position, read_buffer);
	TEST_ASSERT_EQUAL(8053, get_ith_block_number_in_datablock(read_buffer, 1));
	TEST_ASSERT_EQUAL(8078, get_ith_block_number_in_datablock(read_buffer, 2));
	TEST_ASSERT_EQUAL(0, get_ith_block_number_in_datablock(read_buffer, 3));
	TEST_ASSERT_EQUAL(0, get_ith_block_number_in_datablock(read_buffer, BLOCK_SIZE/sizeof(big_int)));

	// Make sure the newly freed datablock contains datablock list that was contained in the 1st datablock before we freed the new datablock
	read_block(8053, read_buffer);
	TEST_ASSERT_EQUAL(pointer_to_second_datablock, get_ith_block_number_in_datablock(read_buffer, 1));
	TEST_ASSERT_EQUAL(2, get_ith_block_number_in_datablock(read_buffer, 2));
	TEST_ASSERT_EQUAL(3, get_ith_block_number_in_datablock(read_buffer, 3));
	TEST_ASSERT_EQUAL(8046, get_ith_block_number_in_datablock(read_buffer, BLOCK_SIZE/sizeof(big_int)));

	// Make sure 3rd datablock contain teh remaining datablock numbers
	read_block(pointer_to_second_datablock, read_buffer);
	TEST_ASSERT_EQUAL(pointer_to_third_datablock, get_ith_block_number_in_datablock(read_buffer, 1));
	TEST_ASSERT_EQUAL(8567, get_ith_block_number_in_datablock(read_buffer, 2));

	free_disk_emulator();
}

TEST(TestDataBlocksHandler, bread) {
	int i;
	struct data_block * datablock;
	char read_buffer[BLOCK_SIZE], buffer[BLOCK_SIZE];

	init_disk_emulator();

	read_block(8201, read_buffer);
	for (i = 0; i < BLOCK_SIZE; i++) {
		buffer[i] = 'd';
		TEST_ASSERT_FALSE(read_buffer[i] == 'd');
	}

	write_block(8201, buffer, BLOCK_SIZE);

	
	datablock = bread(8201);
	TEST_ASSERT_EQUAL(8201, datablock->data_block_id);
	
	for (i = 0; i < BLOCK_SIZE; i++) {
		TEST_ASSERT_EQUAL('d', datablock->block[i]);
	}
}

TEST(TestDataBlocksHandler, bwrite) {
	int i;
	struct data_block datablock;
	char read_buffer[BLOCK_SIZE];

	init_disk_emulator();

	datablock.data_block_id = 8201;

	for (i = 0; i < BLOCK_SIZE; i++) {
		datablock.block[i] = 'e';
	}

	TEST_ASSERT_EQUAL(0, bwrite(&datablock));

	read_block(8201, read_buffer);
	
	for (i = 0; i < BLOCK_SIZE; i++) {
		TEST_ASSERT_EQUAL('e', read_buffer[i]);
	}
}
