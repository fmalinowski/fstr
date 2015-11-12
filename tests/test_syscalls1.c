#include "unity.h"
#include "unity_fixture.h"

#include "common.h"
#include "data_blocks_handler.h"
#include "disk_emulator.h"
#include "mkfs.h"
#include "syscalls1.h"

int dummy_filler(void *buf, const char *name, const struct stat *stbuf, off_t off);

int dummy_filler(void *buf, const char *name, const struct stat *stbuf, off_t off) {
	// Do nothing
	(void) buf;
	(void) name;
	(void) stbuf;
	(void) off;
	return 0;
}

TEST_GROUP_RUNNER(TestSyscalls1) {
	RUN_TEST_CASE(TestSyscalls1, mkdir);
	RUN_TEST_CASE(TestSyscalls1, mknod);
	RUN_TEST_CASE(TestSyscalls1, readdir);
	RUN_TEST_CASE(TestSyscalls1, unlink);
}

TEST_GROUP(TestSyscalls1);

// To be executed before each test case
TEST_SETUP(TestSyscalls1) {
	init_disk_emulator();
	create_fs();
}

// To be executed after each test
TEST_TEAR_DOWN(TestSyscalls1) {
	free_disk_emulator();
}

TEST(TestSyscalls1, mkdir) {
	TEST_ASSERT_EQUAL(-1, mkdir("/folder1/folder2", 0));
	TEST_ASSERT_EQUAL(0, mkdir("/folder1", 0));
	TEST_ASSERT_EQUAL(-1, mkdir("/folder1/folder2/folder3", 0));
	TEST_ASSERT_EQUAL(0, mkdir("/folder1/folder2", 0));
}

TEST(TestSyscalls1, mknod) {
	TEST_ASSERT_EQUAL(-1, mknod("/folder1/newfile", 0, 0));
	TEST_ASSERT_EQUAL(0, mknod("/newfile", 0, 0));

	TEST_ASSERT_EQUAL(0, mkdir("/folder1", 0));
	TEST_ASSERT_EQUAL(0, mknod("/folder1/newfile", 0, 0));
}

TEST(TestSyscalls1, readdir) {
	TEST_ASSERT_EQUAL(0, readdir("/", NULL, dummy_filler, 0));
	TEST_ASSERT_EQUAL(-1, readdir("/folder1", NULL, dummy_filler, 0));
}

TEST(TestSyscalls1, unlink) {
	TEST_ASSERT_EQUAL(-1, unlink("/newfile"));

	mknod("/newfile", 0, 0);

	TEST_ASSERT_EQUAL(0, unlink("/newfile"));
	TEST_ASSERT_EQUAL(-1, unlink("/newfile"));

	TEST_ASSERT_EQUAL(-1, unlink("/"));
}
