#include <string.h>

#include "unity.h"
#include "unity_fixture.h"

#include "disk_emulator.h"
#include "constants.h"


TEST_GROUP_RUNNER(TestDiskEmulator) {
  RUN_TEST_CASE(TestDiskEmulator, rw_block_test);
}



TEST_GROUP(TestDiskEmulator);


// To be executed before each test case
TEST_SETUP(TestDiskEmulator) {
	disk_init();
}

// To be xecuted after each test
TEST_TEAR_DOWN(TestDiskEmulator) {
	disk_destroy();
}


TEST(TestDiskEmulator, rw_block_test) {
	int block_id = 1;
	char *data = "Test Data";
	int bytes = sizeof(char) * strlen(data) + 1;

	// Write data
	TEST_ASSERT_EQUAL(0, write_block(block_id, data, bytes));

	// Read back
	void *p = NULL;
	read_block(block_id, &p);
	TEST_ASSERT_EQUAL_STRING(data, (char*)p);
}
