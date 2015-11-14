#include "unity.h"
#include "unity_fixture.h"

#include "common.h"
#include "disk_emulator.h"
#include "mkfs.h"
#include "inodes_handler.h"
#include "block_utils.h"

TEST_GROUP_RUNNER(TestBlockUtils) {
	RUN_TEST_CASE(TestBlockUtils, traverse_complete_block_list);
}

TEST_GROUP(TestBlockUtils);

// To be executed before each test case
TEST_SETUP(TestBlockUtils) {
	init_disk_emulator();
	create_fs();
}

// To be executed after each test
TEST_TEAR_DOWN(TestBlockUtils) {
	free_disk_emulator();
}

TEST(TestBlockUtils, traverse_complete_block_list) {
	struct inode *inode = ialloc();

	big_int total_blocks = NUM_DIRECT_BLOCKS;
	total_blocks += BLOCK_ID_LIST_LENGTH;
	total_blocks += BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH;
	// Uncomment the line below to test triple indirection
	// Caution: It takes quite a while to test it
	// total_blocks += BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH;

	// Try setting all blocks
	big_int i;
	for(i = 0; i < total_blocks; ++i) {
		TEST_ASSERT_EQUAL(0, set_block_id(inode, i, i+1));
	}

	// Try reading all blocks
	for(i = 0; i < total_blocks; ++i) {
		TEST_ASSERT_EQUAL(i+1, get_block_id(inode, i));
	}

	// Try reading something we've not set
	TEST_ASSERT_EQUAL(0, get_block_id(inode, i));
}
