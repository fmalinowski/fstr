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

	// printf("dummy_filler: %" PRIuMAX " %s\n", stbuf->st_ino, name);

	return 0;
}

TEST_GROUP_RUNNER(TestSyscalls1) {
	RUN_TEST_CASE(TestSyscalls1, syscalls1__mkdir);
	RUN_TEST_CASE(TestSyscalls1, syscalls1__mknod);
	RUN_TEST_CASE(TestSyscalls1, syscalls1__readdir);
	RUN_TEST_CASE(TestSyscalls1, syscalls1__unlink);
	RUN_TEST_CASE(TestSyscalls1, random_create_remove_files_dir);
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

TEST(TestSyscalls1, syscalls1__mkdir) {


	TEST_ASSERT_EQUAL(0, syscalls1__mkdir("/folder1", 0));
}

TEST(TestSyscalls1, syscalls1__mknod) {
	TEST_ASSERT_EQUAL(0, syscalls1__mknod("/newfile", 0, 0));
}

TEST(TestSyscalls1, syscalls1__readdir) {
	syscalls1__mkdir("/folder1", 0);
	syscalls1__mknod("/folder1/newfile1", 0, 0);
	syscalls1__mknod("/folder1/newfile2", 0, 0);
	syscalls1__mkdir("/folder1/folder2", 0);

	TEST_ASSERT_EQUAL(0, syscalls1__readdir("/folder1", NULL, dummy_filler, 0));
}

TEST(TestSyscalls1, syscalls1__unlink) {
	syscalls1__mknod("/newfile", 0, 0);
	TEST_ASSERT_EQUAL(0, syscalls1__unlink("/newfile"));
}

TEST(TestSyscalls1, syscalls1__rmdir) {
	syscalls1__mkdir("/newfile", 0);
	TEST_ASSERT_EQUAL(0, syscalls1__rmdir("/newfile"));
}

TEST(TestSyscalls1, random_create_remove_files_dir) {
	// create 2 folders
	TEST_ASSERT_EQUAL(0, syscalls1__mkdir("/folder1", 0));
	TEST_ASSERT_EQUAL(0, syscalls1__mkdir("/folder2", 0));

	// fail at creating 2 folders again
	TEST_ASSERT_EQUAL(-1, syscalls1__mkdir("/folder1", 0));
	TEST_ASSERT_EQUAL(-1, syscalls1__mkdir("/folder2", 0));

	// create 2 files
	TEST_ASSERT_EQUAL(0, syscalls1__mknod("/newfile1", 0, 0));
	TEST_ASSERT_EQUAL(0, syscalls1__mknod("/newfile2", 0, 0));

	// // fail at creating 2 files again
	TEST_ASSERT_EQUAL(-1, syscalls1__mknod("/newfile1", 0, 0));
	TEST_ASSERT_EQUAL(-1, syscalls1__mknod("/newfile2", 0, 0));

	// remove 1st folder and file
	TEST_ASSERT_EQUAL(0, syscalls1__rmdir("/folder1"));
	TEST_ASSERT_EQUAL(0, syscalls1__unlink("/newfile1"));

	// fail at removing 1st folder and file
	TEST_ASSERT_EQUAL(-1, syscalls1__rmdir("/folder1"));
	TEST_ASSERT_EQUAL(-1, syscalls1__unlink("/newfile1"));

	// fail at removing 2nd folder and file
	TEST_ASSERT_EQUAL(-1, syscalls1__unlink("/folder2"));
	TEST_ASSERT_EQUAL(-1, syscalls1__rmdir("/newfile2"));

	// fail at creating 2nd folder and file
	TEST_ASSERT_EQUAL(-1, syscalls1__mkdir("/newfile2", 0));
	TEST_ASSERT_EQUAL(-1, syscalls1__mknod("/folder2", 0, 0));

	// create file in folder2
	TEST_ASSERT_EQUAL(0, syscalls1__mknod("/folder2/newfile3", 0, 0));

	// fail at removing folder2
	TEST_ASSERT_EQUAL(-1, syscalls1__rmdir("/folder2"));

	// remove file from folder2
	TEST_ASSERT_EQUAL(0, syscalls1__unlink("/folder2/newfile3"));

	// remove folder2
	TEST_ASSERT_EQUAL(0, syscalls1__rmdir("/folder2"));

	// remove newfile2
	TEST_ASSERT_EQUAL(0, syscalls1__unlink("/newfile2"));
}
