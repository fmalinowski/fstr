#include "unity.h"
#include "unity_fixture.h"

#include "common.h"
#include "disk_emulator.h"
#include "mkfs.h"

TEST_GROUP_RUNNER(TestMkfs) {
	RUN_TEST_CASE(TestMkfs, superblock_is_written_correctly);
	RUN_TEST_CASE(TestMkfs, inodes_are_written_contiguously_after_superblock);
}

TEST_GROUP(TestMkfs);

// To be executed before each test case
TEST_SETUP(TestMkfs) {
	init_disk_emulator();
}

// To be executed after each test
TEST_TEAR_DOWN(TestMkfs) {
	free_disk_emulator();
}

TEST(TestMkfs, superblock_is_written_correctly) {
	// Init superblock and write to disk
	create_superblock();

	struct superblock target;
	char buffer[BLOCK_SIZE];

	// Read block 0
	read_block(0, buffer);
	memcpy(&target, buffer, sizeof(struct superblock));
	
	TEST_ASSERT_EQUAL(0, memcmp(&superblock, &target, sizeof(struct superblock)));
}

TEST(TestMkfs, inodes_are_written_contiguously_after_superblock) {
	create_inodes();

	char buffer[BLOCK_SIZE];
	struct inode inode;

	int inode_count = 0;
	int i;
	for(i = 0; inode_count < NUM_INODES && i < NUM_INODE_BLOCKS; i++) {
		read_block(1 + i, buffer);

		int j;
		for(j = 0; inode_count < NUM_INODES && j < (BLOCK_SIZE / INODE_SIZE); j++) {
			memcpy(&inode, buffer + (j * INODE_SIZE), sizeof(struct inode));
			TEST_ASSERT_EQUAL(inode_count + 1, inode.inode_id);
			inode_count++;
		}
	}
}