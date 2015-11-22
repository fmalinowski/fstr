#include "unity.h"
#include "unity_fixture.h"

#include "common.h"
#include "disk_emulator.h"
#include "mkfs.h"
#include "inodes_handler.h"

TEST_GROUP_RUNNER(TestMkfs) {
	RUN_TEST_CASE(TestMkfs, superblock_is_written_correctly);
	RUN_TEST_CASE(TestMkfs, inodes_are_written_contiguously_after_superblock);
	RUN_TEST_CASE(TestMkfs, free_lists_are_correctly_initialized);
	RUN_TEST_CASE(TestMkfs, create_fs);
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
	unsigned int i;
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

TEST(TestMkfs, free_lists_are_correctly_initialized) {
	create_superblock(); // This is required to init superblock
	create_free_blocks();

	struct block_id_list block_id_list;
	big_int free_blocks_count = 0;
	big_int free_list_pointer = superblock.next_free_block_list;

	while(free_list_pointer) {
		if(read_block(free_list_pointer, &block_id_list))	break;

		free_list_pointer = block_id_list.list[0];
		int i;
		for(i = BLOCK_ID_LIST_LENGTH - 1; i > 0; i--) {
			if(!block_id_list.list[i])	break;
			free_blocks_count++;
		}
	}

	TEST_ASSERT_EQUAL(superblock.num_free_blocks, free_blocks_count);
}

TEST(TestMkfs, create_fs) {
	create_fs();

	struct inode root_inode;
	TEST_ASSERT_TRUE(-1 != iget(ROOT_INODE_NUMBER, &root_inode));

	TEST_ASSERT_EQUAL(ROOT_INODE_NUMBER, root_inode.inode_id);
	TEST_ASSERT_EQUAL(1, root_inode.links_nb);
	TEST_ASSERT_EQUAL(1, root_inode.num_blocks);
	TEST_ASSERT_EQUAL(TYPE_DIRECTORY, root_inode.type);
	TEST_ASSERT_TRUE(root_inode.direct_blocks[0] != 0);

	struct dir_block dir_block;
	read_block(root_inode.direct_blocks[0], &dir_block);

	TEST_ASSERT_EQUAL(ROOT_INODE_NUMBER, dir_block.inode_ids[0]);
	TEST_ASSERT_EQUAL(0, strcmp(".", dir_block.names[0]));

	TEST_ASSERT_EQUAL(ROOT_INODE_NUMBER, dir_block.inode_ids[1]);
	TEST_ASSERT_EQUAL(0, strcmp("..", dir_block.names[1]));

	TEST_ASSERT_EQUAL(0, dir_block.inode_ids[2]);
	TEST_ASSERT_EQUAL(0, strcmp("", dir_block.names[2]));
}
