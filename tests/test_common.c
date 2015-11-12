#include "unity.h"
#include "unity_fixture.h"

#include "common.h"
#include "disk_emulator.h"
#include "mkfs.h"

TEST_GROUP_RUNNER(TestCommon) {
	RUN_TEST_CASE(TestCommon, commit_and_read_superblock);
	RUN_TEST_CASE(TestCommon, write_block_offset);
	RUN_TEST_CASE(TestCommon, init_dir_block);
	RUN_TEST_CASE(TestCommon, add_and_remove_entry_from_dir_block);
	RUN_TEST_CASE(TestCommon, add_entry_to_parent);
}

TEST_GROUP(TestCommon);

// To be executed before each test case
TEST_SETUP(TestCommon) {
	init_disk_emulator();
	create_fs();
}

// To be executed after each test
TEST_TEAR_DOWN(TestCommon) {
	free_disk_emulator();
}

TEST(TestCommon, commit_and_read_superblock) {
	superblock.num_free_blocks++;
	superblock.num_free_inodes++;
	superblock.commit();

	struct data_block data_block;
	read_block(0, &data_block.block);

	struct superblock copy;
	memcpy(&copy, &data_block.block, sizeof(struct superblock));

	TEST_ASSERT_EQUAL(0, memcmp(&superblock, &copy, sizeof(struct superblock)));
}

TEST(TestCommon, write_block_offset) {
	big_int block_id = 5;

	struct data_block data_block;
	read_block(block_id, &data_block.block);

	const char *a = "test is";
	const char *b = " a success!";

	// Write strings individually to disk
	write_block_offset(block_id, (void*) a, strlen(a), 0);
	write_block_offset(block_id, (void*) b, strlen(b), strlen(a));

	// Commit to memory representation
	memcpy(data_block.block, a, strlen(a));
	memcpy(data_block.block + strlen(a), b, strlen(b));

	// Read a copy from disk
	struct data_block read_copy;
	read_block(block_id, &read_copy.block);

	// Verify
	TEST_ASSERT_EQUAL(0, memcmp(&data_block.block, &read_copy.block, BLOCK_SIZE));
}

TEST(TestCommon, init_dir_block) {
	int inode_id = 50;
	int parent_inode_id = 500;
	struct dir_block dir_block;

	init_dir_block(&dir_block, inode_id, parent_inode_id);

	TEST_ASSERT_EQUAL(inode_id, dir_block.inode_ids[0]);
	TEST_ASSERT_EQUAL(0, strcmp(".", dir_block.names[0]));

	TEST_ASSERT_EQUAL(parent_inode_id, dir_block.inode_ids[1]);
	TEST_ASSERT_EQUAL(0, strcmp("..", dir_block.names[1]));

	int len = BLOCK_SIZE / NAMEI_ENTRY_SIZE;
	int i;
	for(i = 2; i < len; ++i) {
		TEST_ASSERT_EQUAL(0, dir_block.inode_ids[i]);
		TEST_ASSERT_EQUAL(0, strcmp("", dir_block.names[i]));
	}
}

TEST(TestCommon, add_and_remove_entry_from_dir_block) {
	struct dir_block dir_block;
	init_dir_block(&dir_block, 50, 500);

	// Test adding entries
	TEST_ASSERT_EQUAL(0, add_entry_to_dir_block(&dir_block, 1, "name1"));
	TEST_ASSERT_EQUAL(0, add_entry_to_dir_block(&dir_block, 2, "name2"));
	TEST_ASSERT_EQUAL(0, add_entry_to_dir_block(&dir_block, 3, "name3"));

	TEST_ASSERT_EQUAL(1, dir_block.inode_ids[2]);
	TEST_ASSERT_EQUAL(0, strcmp("name1", dir_block.names[2]));

	TEST_ASSERT_EQUAL(2, dir_block.inode_ids[3]);
	TEST_ASSERT_EQUAL(0, strcmp("name2", dir_block.names[3]));

	TEST_ASSERT_EQUAL(3, dir_block.inode_ids[4]);
	TEST_ASSERT_EQUAL(0, strcmp("name3", dir_block.names[4]));

	int len = BLOCK_SIZE / NAMEI_ENTRY_SIZE;
	int i;
	for(i = 5; i < len; ++i) {
		TEST_ASSERT_EQUAL(0, dir_block.inode_ids[i]);
		TEST_ASSERT_EQUAL(0, strcmp("", dir_block.names[i]));
	}

	// Test removing entries
	TEST_ASSERT_EQUAL(0, remove_entry_from_dir_block(&dir_block, 1));
	TEST_ASSERT_EQUAL(0, remove_entry_from_dir_block(&dir_block, 2));
	TEST_ASSERT_EQUAL(0, remove_entry_from_dir_block(&dir_block, 3));

	for(i = 2; i < len; ++i) {
		TEST_ASSERT_EQUAL(0, dir_block.inode_ids[i]);
		TEST_ASSERT_EQUAL(0, strcmp("", dir_block.names[i]));
	}
}

TEST(TestCommon, add_entry_to_parent) {
	struct inode parent_inode = {
		.inode_id = 500
	};
	int inode_id = 50;

	struct dir_block dir_block;
	init_dir_block(&dir_block, inode_id, parent_inode.inode_id);

	TEST_ASSERT_EQUAL(0, parent_inode.num_blocks);

	int len = BLOCK_SIZE / NAMEI_ENTRY_SIZE;
	int i;
	for(i = 0; i < len + 1; ++i) {
		TEST_ASSERT_EQUAL(0, add_entry_to_parent(&parent_inode, inode_id, "name"));
	}

	TEST_ASSERT_EQUAL(2, parent_inode.num_blocks);
}
