#include <string.h>

#include "unity.h"
#include "unity_fixture.h"

#include "constants.h"
#include "disk_emulator.h"


TEST_GROUP_RUNNER(TestDiskEmulator) {
  RUN_TEST_CASE(TestDiskEmulator, init_disk_emulator_and_free_disk_emulator_well);
  RUN_TEST_CASE(TestDiskEmulator, write_block__succeed_if_buffer_size_is_less_than_DATA_BLOCK_SIZE);
  RUN_TEST_CASE(TestDiskEmulator, write_block__succeed_if_write_is_inside_limits_of_disk);
  RUN_TEST_CASE(TestDiskEmulator, read_block__cannot_read_if_outside_disk_limits);
  RUN_TEST_CASE(TestDiskEmulator, read_block__write_and_read_correctly_what_was_written);
  RUN_TEST_CASE(TestDiskEmulator, write_block__we_can_write_any_data_structure_to_disk_and_read_them_correctly);
  RUN_TEST_CASE(TestDiskEmulator, write_block__we_can_write_several_consecutive_data_structures_in_same_block_and_read_them_correctly);
}





TEST_GROUP(TestDiskEmulator);


// To be executed before each test case
TEST_SETUP(TestDiskEmulator) {
}

// To be xecuted after each test
TEST_TEAR_DOWN(TestDiskEmulator) {
}

static char * create_string(char c, int length) {
	char * string = malloc((length + 1) * sizeof(char));
	for (int i = 0; i < length; i++) {
		string[i] = c;
	}
	string[length] = '\0';
	return string;
}

static void free_string(char * string) {
	free(string);
}

static int are_character_sequences_equal(char * s1, char * s2, int length) {
	for (int i = 0; i < length; i++) {
		if (s1[i] != s2[i]) {
			return -1;
		}
	}
	return 0;
}



TEST(TestDiskEmulator, init_disk_emulator_and_free_disk_emulator_well) {
	free_disk_emulator();
	TEST_ASSERT_EQUAL(0, init_disk_emulator());
	TEST_ASSERT_EQUAL(-1, init_disk_emulator());
	free_disk_emulator();
	TEST_ASSERT_EQUAL(0, init_disk_emulator());
	free_disk_emulator();
}

// write test when buffer to be written to disk is smaller than block_size or bigger

TEST(TestDiskEmulator, write_block__succeed_if_write_is_inside_limits_of_disk) {
	char * buffer;

	buffer = create_string('a', DATA_BLOCK_SIZE - 1);

	init_disk_emulator();
	
	TEST_ASSERT_EQUAL(0, write_block(0, buffer, strlen(buffer) + 1)); // Write in first block
	TEST_ASSERT_EQUAL(0, write_block(TOTAL_BLOCKS - 1, buffer, strlen(buffer) + 1)); // Write in last block

	TEST_ASSERT_EQUAL(-1, write_block(-1, buffer, strlen(buffer) + 1)); // Write doesn't work before 1st block
	TEST_ASSERT_EQUAL(-1, write_block(TOTAL_BLOCKS, buffer, strlen(buffer) + 1)); // Write doesn't work after last block
	
	free_disk_emulator();
	free_string(buffer);
}

TEST(TestDiskEmulator, read_block__cannot_read_if_outside_disk_limits) {
	char *read_buffer, *expected_string;

	read_buffer = create_string('a', DATA_BLOCK_SIZE - 1);
	expected_string = create_string('a', DATA_BLOCK_SIZE - 1);

	init_disk_emulator();
	
	TEST_ASSERT_EQUAL(-1, read_block(-1, read_buffer)); // invalid read before 1st block
	TEST_ASSERT_EQUAL_STRING(expected_string, read_buffer);
	TEST_ASSERT_EQUAL(-1, read_block(TOTAL_BLOCKS, read_buffer)); // invalid read after last block
	TEST_ASSERT_EQUAL_STRING(expected_string, read_buffer);
	
	free_disk_emulator();
	free_string(read_buffer);
	free_string(expected_string);
}

TEST(TestDiskEmulator, read_block__write_and_read_correctly_what_was_written) {
	char read_buffer[DATA_BLOCK_SIZE];

	init_disk_emulator();
	
	char * expected_block_1 = create_string('a', DATA_BLOCK_SIZE);
	char * expected_block_2 = create_string('b', DATA_BLOCK_SIZE);
	char * expected_block_3 = create_string('c', DATA_BLOCK_SIZE);

	char * block_1 = create_string('a', DATA_BLOCK_SIZE + 5);
	char * block_2 = create_string('b', DATA_BLOCK_SIZE + 5);
	char * block_3 = create_string('c', DATA_BLOCK_SIZE + 5);

	write_block(0, block_1, strlen(block_1) + 1);
	TEST_ASSERT_EQUAL(0, read_block(0, read_buffer));

	// We cannot assert strings are equal as write_block won't copy the "end of string" character (\0) if the string provided for the copy is bigger
	// than the DATA_BLOCK_SIZE. Therefore we need to manually check that every character is correct for a given length.
	// We want to make sure it contains the right data and size of copied data is block size (not more)
	TEST_ASSERT_EQUAL(0, are_character_sequences_equal(expected_block_1, read_buffer, DATA_BLOCK_SIZE));

	read_block(1, read_buffer);
	TEST_ASSERT_FALSE('a' == read_buffer[0]); // We make sure that the write didn't spill over the next block (it wrote just a block size even if input is bigger)
	TEST_ASSERT_FALSE('a' == read_buffer[1]);

	write_block(1, block_2, strlen(block_2) + 1);
	TEST_ASSERT_EQUAL(0, read_block(1, read_buffer));
	TEST_ASSERT_EQUAL(0, are_character_sequences_equal(expected_block_2, read_buffer, DATA_BLOCK_SIZE));

	write_block(TOTAL_BLOCKS - 1, block_3, strlen(block_3) + 1);
	TEST_ASSERT_EQUAL(0, read_block(TOTAL_BLOCKS - 1, read_buffer));
	TEST_ASSERT_EQUAL(0, are_character_sequences_equal(expected_block_3, read_buffer, DATA_BLOCK_SIZE));

	TEST_ASSERT_EQUAL(0, read_block(0, read_buffer));
	TEST_ASSERT_EQUAL(0, are_character_sequences_equal(expected_block_1, read_buffer, DATA_BLOCK_SIZE));

	free_disk_emulator();
	free_string(block_1);
	free_string(block_2);
	free_string(block_3);
	free_string(expected_block_1);
	free_string(expected_block_2);
	free_string(expected_block_3);
}

TEST(TestDiskEmulator, write_block__succeed_if_buffer_size_is_less_than_DATA_BLOCK_SIZE) {
	char * buffer;
	char read_buffer[DATA_BLOCK_SIZE];

	buffer = create_string('a', 5);
	init_disk_emulator();
	
	TEST_ASSERT_EQUAL(0, write_block(0, buffer, strlen(buffer) + 1)); // Write in first block
	read_block(0, read_buffer);
	TEST_ASSERT_EQUAL_STRING("aaaaa", read_buffer);

	free_string(buffer);
	buffer = create_string('b', 3);
	TEST_ASSERT_EQUAL(0, write_block(0, buffer, strlen(buffer) + 1)); // Write in first block
	read_block(0, read_buffer);
	TEST_ASSERT_EQUAL_STRING("bbb", read_buffer);
	TEST_ASSERT_FALSE('a' == read_buffer[4]);
	
	free_disk_emulator();
	free_string(buffer);
}

TEST(TestDiskEmulator, write_block__we_can_write_any_data_structure_to_disk_and_read_them_correctly) {
	struct structA {
		int a;
		int b;
	};

	struct structA a;

	a.a = 25;
	a.b = 27;

	void * read_buffer = malloc(DATA_BLOCK_SIZE * sizeof(void *));
	init_disk_emulator();

	write_block(0, &a, sizeof(struct structA));
	read_block(0, read_buffer);

	struct structA * result = (struct structA *) read_buffer;
	TEST_ASSERT_EQUAL(25, result->a);
	TEST_ASSERT_EQUAL(27, result->b);



	free(read_buffer);
	free_disk_emulator();
}

TEST(TestDiskEmulator, write_block__we_can_write_several_consecutive_data_structures_in_same_block_and_read_them_correctly) {
	struct structA {
		int a;
		int b;
	};

	struct structB {
		char a;
		int b;
	};

	struct structA a;
	struct structA b;

	a.a = 25;
	a.b = 27;
	b.a = 'g';
	b.b = 35;

	void ** write_buffer = malloc(DATA_BLOCK_SIZE * sizeof(void *));
	void ** read_buffer = malloc(DATA_BLOCK_SIZE * sizeof(void *));
	init_disk_emulator();

	write_buffer[0] = &a;
	write_buffer[sizeof(a)] = &b;

	write_block(0, write_buffer, DATA_BLOCK_SIZE);
	read_block(0, read_buffer);

	struct structA * resultA = (struct structA *) read_buffer[0];
	struct structB * resultB = (struct structB *) read_buffer[sizeof(struct structA)];
	TEST_ASSERT_EQUAL(25, resultA->a);
	TEST_ASSERT_EQUAL(27, resultA->b);
	TEST_ASSERT_EQUAL('g', resultB->a);
	TEST_ASSERT_EQUAL(35, resultB->b);

	
	free(write_buffer);
	free(read_buffer);
	free_disk_emulator();
}
