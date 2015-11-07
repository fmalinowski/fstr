#include "unity.h"
#include "unity_fixture.h"

#include "syscalls2.h"


TEST_GROUP_RUNNER(TestSyscalls2) {
	RUN_TEST_CASE(TestSyscalls2, set_and_get_file_descriptor_table);
	RUN_TEST_CASE(TestSyscalls2, delete_file_descriptor_table);
}




TEST_GROUP(TestSyscalls2);


// To be executed before each test case
TEST_SETUP(TestSyscalls2) {
}

// To be executed after each test
TEST_TEAR_DOWN(TestSyscalls2) {
}


TEST(TestSyscalls2, set_and_get_file_descriptor_table) {
	struct file_descriptor_table *table, *result_table;

	table = malloc(sizeof(struct file_descriptor_table)); // We need to use dynamic allocation with that hash table
	table->pid = 12;
	table->total_descriptors = 2;

	set_file_descriptor_table(12, table);

	result_table = get_file_descriptor_table(12);
	TEST_ASSERT_EQUAL(12, result_table->pid);
	TEST_ASSERT_EQUAL(2, result_table->total_descriptors);

	result_table->total_descriptors = 3;
	TEST_ASSERT_EQUAL(3, table->total_descriptors);

	TEST_ASSERT_NULL(get_file_descriptor_table(2));

	delete_file_descriptor_table(12);
	free(table);
}

TEST(TestSyscalls2, delete_file_descriptor_table) {
	struct file_descriptor_table * table;

	table = malloc(sizeof(struct file_descriptor_table)); // We need to use dynamic allocation with that hash table
	table->pid = 12;
	table->total_descriptors = 2;

	set_file_descriptor_table(12, table);

	TEST_ASSERT_FALSE(get_file_descriptor_table(12) == NULL);

	delete_file_descriptor_table(12);

	TEST_ASSERT_NULL(get_file_descriptor_table(12));
	TEST_ASSERT_FALSE(table == NULL);
	TEST_ASSERT_EQUAL(12, table->pid);

	// make sure it doesn't blow up if we delete something that doesn't exist
	delete_file_descriptor_table(12);
	free(table);
}
