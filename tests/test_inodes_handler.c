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
	RUN_TEST_CASE(TestInodesHandler, test_2_allocs_and_2_saves);
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
	struct inode inod;
	struct inode inod2;
	iget(100, &inod);
	TEST_ASSERT_EQUAL(100, inod.inode_id);
	iget(101, &inod2);
	TEST_ASSERT_EQUAL(101, inod2.inode_id);
	TEST_ASSERT_EQUAL(-1, iget(0, &inod));
}


TEST(TestInodesHandler, test_ialloc_works_correctly){
	int x;
	struct inode inod;
	struct inode inod2;
	TEST_ASSERT_TRUE(ialloc(&inod) != -1);
	TEST_ASSERT_TRUE(ialloc(&inod2) != -1);
	TEST_ASSERT_TRUE(inod.links_nb > 0);
	TEST_ASSERT_TRUE(inod.inode_id > 0);
	TEST_ASSERT_TRUE(inod2.links_nb > 0);
	TEST_ASSERT_TRUE(inod2.inode_id > 0);
	TEST_ASSERT_TRUE(inod.inode_id <= NUM_INODES);
	TEST_ASSERT_TRUE(inod2.inode_id <= NUM_INODES);
	TEST_ASSERT_TRUE(inod.inode_id != inod2.inode_id);
	inod.links_nb = 2;		
	x = inod.inode_id;		
	// printf("x is %d", inod->inode_id);		
	TEST_ASSERT_TRUE(0 == iput(&inod));		
	struct inode inod3;
	iget(x, &inod3);		
	TEST_ASSERT_TRUE(2 == inod3.links_nb);
}


TEST(TestInodesHandler, test_ifree_works_correctly){
	struct inode inod;
	TEST_ASSERT_TRUE(ialloc(&inod) != -1);
	TEST_ASSERT_EQUAL(0, ifree(&inod));
	struct inode inod2;
	ialloc(&inod2);
	inod2.links_nb = 0;
	TEST_ASSERT_EQUAL(0, iput(&inod2));
}

TEST(TestInodesHandler, test_iput_works_correctly){ // Wait for Francois's reply. 
	int x;
	struct inode inod2;
	struct inode inod;
	
	//printf("inode allocated\n");
	TEST_ASSERT_TRUE(ialloc(&inod) != -1);
	//printf("inode allocated, not null\n");
	struct data_block *blok;
	int i, j;
	big_int k;
	x = inod.inode_id;
	inod.links_nb = 0;
	for(i = 0; i < 16; i++){
		inod.direct_blocks[i] = i + 1000;
	}
	inod.single_indirect_block = 16 + 1000;
	inod.double_indirect_block = 17 + 1000;
	inod.triple_indirect_block = 18 + 1000;
	//printf("calling iput now");
	TEST_ASSERT_EQUAL(0, iput(&inod)); // test that the superblock is not freed ever

	inod.links_nb = 1;
	for(i = 0; i < 16; i++){
		inod.direct_blocks[i] = i + 1000;
	}
	inod.single_indirect_block = 16 + 1000;
	blok = bread(inod.single_indirect_block);
	j = 20 + 1000;
	for(k = 0; k < BLOCK_SIZE/sizeof(big_int); k++, j++){
		memcpy(&(blok->block[k]), &j, sizeof(big_int));
	}

	inod.double_indirect_block = 17 + 1000;
	inod.triple_indirect_block = 18 + 1000;
	TEST_ASSERT_EQUAL(0, iput(&inod));	
	iget(x, &inod2);
	TEST_ASSERT_EQUAL(x, inod2.inode_id);	
}

TEST(TestInodesHandler, test_2_allocs_and_2_saves){
	struct inode inod1, inod2;
	
	ialloc(&inod1);
	TEST_ASSERT_EQUAL(2, inod1.inode_id);

	inod1.last_modified_file = 56;
	inod1.links_nb = 14;
	iput(&inod1);

	iget(2, &inod1);
	TEST_ASSERT_EQUAL(2, inod1.inode_id);
	TEST_ASSERT_EQUAL(14, inod1.links_nb);
	TEST_ASSERT_EQUAL(56, inod1.last_modified_file);

	ialloc(&inod2);
	TEST_ASSERT_EQUAL(3, inod2.inode_id);

	inod2.last_modified_file = 57;
	inod2.links_nb = 15;
	iput(&inod2);

	iget(3, &inod2);
	TEST_ASSERT_EQUAL(3, inod2.inode_id);
	TEST_ASSERT_EQUAL(15, inod2.links_nb);
	TEST_ASSERT_EQUAL(57, inod2.last_modified_file);
}
