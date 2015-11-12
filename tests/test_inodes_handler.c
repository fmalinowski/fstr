#include "unity.h"
#include "unity_fixture.h"

#include "common.h"
#include "data_blocks_handler.h"
#include "disk_emulator.h"
#include "inodes_handler.h"
#include "mkfs.h"

TEST_GROUP_RUNNER(TestInodesHandler) {
	RUN_TEST_CASE(TestInodesHandler, test_iget_works_correctly);
	RUN_TEST_CASE(TestInodesHandler, test_ialloc_works_correctly);
	RUN_TEST_CASE(TestInodesHandler, test_ifree_works_correctly);
	RUN_TEST_CASE(TestInodesHandler, test_iput_works_correctly);
}

TEST_GROUP(TestInodesHandler);

// To be executed before each test case
TEST_SETUP(TestInodesHandler) {
	init_disk_emulator();
	create_fs();
}

// To be executed after each test
TEST_TEAR_DOWN(TestInodesHandler) {
	free_disk_emulator();
}

TEST(TestInodesHandler, test_iget_works_correctly){
	struct inode * inod;
	struct inode * inod2;
	inod = iget(100);
	TEST_ASSERT_EQUAL(100, inod->inode_id);
	inod2 = iget(101);
	TEST_ASSERT_EQUAL(101, inod2->inode_id);
	//free(inod);
	inod = iget(0);
	TEST_ASSERT_EQUAL(NULL, inod);
	//free(inod);
}


TEST(TestInodesHandler, test_ialloc_works_correctly){
	struct inode * inod = ialloc();
	struct inode * inod2 = ialloc();
	TEST_ASSERT_TRUE(inod != NULL);
	TEST_ASSERT_TRUE(inod2 != NULL);
	TEST_ASSERT_TRUE(inod->links_nb > 0);
	TEST_ASSERT_TRUE(inod->inode_id > 0);
	TEST_ASSERT_TRUE(inod2->links_nb > 0);
	TEST_ASSERT_TRUE(inod2->inode_id > 0);
	TEST_ASSERT_TRUE(inod->inode_id <= NUM_INODES);
	TEST_ASSERT_TRUE(inod2->inode_id <= NUM_INODES);
	TEST_ASSERT_TRUE(inod->inode_id != inod2->inode_id);
}


TEST(TestInodesHandler, test_ifree_works_correctly){
	struct inode * inod = ialloc();
	TEST_ASSERT_TRUE(inod != NULL);
	TEST_ASSERT_EQUAL(0, ifree(inod));
}

TEST(TestInodesHandler, test_iput_works_correctly){ // Wait for Francois's reply. 
	int x;
	struct inode* inod2;
	struct inode* inod = ialloc();
	
	//printf("inode allocated\n");
	TEST_ASSERT_TRUE(inod != NULL);
	//printf("inode allocated, not null\n");
	struct data_block *blok;
	int i, j;
	big_int k;
	x = inod->inode_id;
	inod->links_nb = 0;
	for(i = 0; i < 16; i++){
		inod->direct_blocks[i] = i + 1000;
	}
	inod->single_indirect_block = 16 + 1000;
	inod->double_indirect_block = 17 + 1000;
	inod->triple_indirect_block = 18 + 1000;
	//printf("calling iput now");
	TEST_ASSERT_EQUAL(0, iput(inod)); // test that the superblock is not freed ever

	inod->links_nb = 1;
	for(i = 0; i < 16; i++){
		inod->direct_blocks[i] = i + 1000;
	}
	inod->single_indirect_block = 16 + 1000;
	blok = bread(inod->single_indirect_block);
	j = 20 + 1000;
	for(k = 0; k < BLOCK_SIZE/sizeof(big_int); k++, j++){
		memcpy(&(blok->block[k]), &j, sizeof(big_int));
	}

	inod->double_indirect_block = 17 + 1000;
	inod->triple_indirect_block = 18 + 1000;
	TEST_ASSERT_EQUAL(0, iput(inod));	
	inod2 = iget(x);
	TEST_ASSERT_EQUAL(x, inod2->inode_id);	
}



