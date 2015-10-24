#include <string.h>

#include "unity.h"
#include "unity_fixture.h"

#include "disk_emulator.h"
#include "constants.h"


TEST_GROUP_RUNNER(TestDiskEmulator) {
  RUN_TEST_CASE(TestDiskEmulator, allocate_disk_and_unallocation_work_well);
  RUN_TEST_CASE(TestDiskEmulator, write_block__succeed_if_write_is_inside_limits_of_disk);
  RUN_TEST_CASE(TestDiskEmulator, read_block__cannot_read_if_outside_disk_limits);
  RUN_TEST_CASE(TestDiskEmulator, read_block__write_and_read_correctly_what_was_written);
}





TEST_GROUP(TestDiskEmulator);


// To be executed before each test case
TEST_SETUP(TestDiskEmulator) {
}

// To be xecuted after each test
TEST_TEAR_DOWN(TestDiskEmulator) {
}


TEST(TestDiskEmulator, read_block__cannot_read_if_outside_disk_limits) {
	int blck_size, inode_blck_nb, data_blck_nb;
	char read_buffer[10] = "55555";

	blck_size = 3;
	inode_blck_nb = 2;
	data_blck_nb = 1;
	
	TEST_ASSERT_EQUAL(-1, read_block(-1, read_buffer)); // invalid read before 1st block
	TEST_ASSERT_EQUAL_STRING("55555", read_buffer);
	TEST_ASSERT_EQUAL(-1, read_block(inode_blck_nb + data_blck_nb + 1, read_buffer)); // invalid read after last block
	TEST_ASSERT_EQUAL_STRING("55555", read_buffer);
}

TEST(TestDiskEmulator, read_block__write_and_read_correctly_what_was_written) {
	int blck_size, inode_blck_nb, data_blck_nb;
	char read_buffer[6];

	blck_size = 3;
	inode_blck_nb = 2;
	data_blck_nb = 1;

	allocate_disk(blck_size, inode_blck_nb, data_blck_nb);
	
	char block_1[5] = "aaaaa";
	char block_2[5] = "bbbbb";
	char block_3[5] = "ccccc";

	write_block(0, block_1);
	TEST_ASSERT_EQUAL(0, read_block(0, read_buffer));
	TEST_ASSERT_EQUAL_STRING("aaa", read_buffer); // make sure it contains the right data and size of copied data is block size (not more)

	strcpy(read_buffer, "zzzzz"); // write something in read buffer to make sure the read is done correctly
	read_block(1, read_buffer);
	TEST_ASSERT_FALSE('a' == read_buffer[0]); // We make sure that the write didn't spill over the next block (it wrote just a block size even if input is bigger)
	TEST_ASSERT_FALSE('a' == read_buffer[1]);

	strcpy(read_buffer, "zzzzz"); // write something in read buffer to make sure the read is done correctly
	write_block(1, block_2);
	TEST_ASSERT_EQUAL(0, read_block(1, read_buffer));
	TEST_ASSERT_EQUAL_STRING("bbbzz", read_buffer);

	strcpy(read_buffer, "zzzzz"); // write something in read buffer to make sure the read is done correctly
	write_block(3, block_3);
	TEST_ASSERT_EQUAL(0, read_block(3, read_buffer));
	TEST_ASSERT_EQUAL_STRING("ccczz", read_buffer);

	strcpy(read_buffer, "zzzzz"); // write something in read buffer to make sure the read is done correctly
	TEST_ASSERT_EQUAL(0, read_block(0, read_buffer));
	TEST_ASSERT_EQUAL_STRING("aaazz", read_buffer);

	unallocate_disk();
}
