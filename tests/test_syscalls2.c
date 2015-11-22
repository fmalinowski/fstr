#include "unity.h"
#include "unity_fixture.h"

#include "syscalls2.h"
#include "disk_emulator.h"
#include "mkfs.h"
#include "inodes_handler.h"
#include "data_blocks_handler.h"
#include "inode_table.h"


TEST_GROUP_RUNNER(TestSyscalls2) {
	RUN_TEST_CASE(TestSyscalls2, allocate_file_descriptor_table);
	RUN_TEST_CASE(TestSyscalls2, get_file_descriptor_table);
	RUN_TEST_CASE(TestSyscalls2, delete_file_descriptor_table);
	RUN_TEST_CASE(TestSyscalls2, find_available_fd);
	RUN_TEST_CASE(TestSyscalls2, allocate_file_descriptor_entry);
	RUN_TEST_CASE(TestSyscalls2, allocate_file_descriptor_entry__when_table_is_full);
	RUN_TEST_CASE(TestSyscalls2, get_file_descriptor_entry);
	RUN_TEST_CASE(TestSyscalls2, delete_file_descriptor_entry);
	RUN_TEST_CASE(TestSyscalls2, syscall2__get_pid);
	RUN_TEST_CASE(TestSyscalls2, syscalls2__open);
	RUN_TEST_CASE(TestSyscalls2, syscalls2__open__when_path_does_not_exist);
	RUN_TEST_CASE(TestSyscalls2, syscalls2__open__with_ocreate_mode);
	RUN_TEST_CASE(TestSyscalls2, syscalls2__close);
	RUN_TEST_CASE(TestSyscalls2, syscalls2__open_syscalls2__close);
	RUN_TEST_CASE(TestSyscalls2, get_size_of_file);
	RUN_TEST_CASE(TestSyscalls2, convert_byte_offset_to_ith_datablock);
	RUN_TEST_CASE(TestSyscalls2, is_ith_block_in_range_of_direct_and_indirect_blocks);
	RUN_TEST_CASE(TestSyscalls2, get_ith_datablock_number);
	RUN_TEST_CASE(TestSyscalls2, set_ith_datablock_number);
	RUN_TEST_CASE(TestSyscalls2, read__error_cases__fd_does_not_exist__or_fde_set_to_write_only__or_offset_too_big);
	RUN_TEST_CASE(TestSyscalls2, read__read_bytes_in_last_block_of_file);
	RUN_TEST_CASE(TestSyscalls2, read__less_bytes_to_read_than_a_block);
	RUN_TEST_CASE(TestSyscalls2, read__more_bytes_to_read_than_a_block);
	RUN_TEST_CASE(TestSyscalls2, read__more_bytes_to_read_than_available_in_file);
	RUN_TEST_CASE(TestSyscalls2, pwrite__write_after_end_of_file);
	RUN_TEST_CASE(TestSyscalls2, pwrite__write_in_a_block_that_was_already_written);
}




TEST_GROUP(TestSyscalls2);


// To be executed before each test case
TEST_SETUP(TestSyscalls2) {
}

// To be executed after each test
TEST_TEAR_DOWN(TestSyscalls2) {
}



TEST(TestSyscalls2, allocate_file_descriptor_table) {
	struct file_descriptor_table *table;
	table = allocate_file_descriptor_table(2112);

	TEST_ASSERT_EQUAL(2112, table->pid);
	TEST_ASSERT_EQUAL(8, table->total_descriptors);

	int i;

	// Make sure the 1st file descriptors are set properly
	TEST_ASSERT_EQUAL(0, table->entries[0].fd);
	TEST_ASSERT_EQUAL(1, table->entries[1].fd);
	TEST_ASSERT_EQUAL(2, table->entries[2].fd);

	for (i = 3; i < 8; i++) {
		TEST_ASSERT_EQUAL(FD_NOT_USED, table->entries[i].fd); // Make sure file descriptor is set to unused
	}

	table = allocate_file_descriptor_table(2112);
	TEST_ASSERT_NULL(table);

	delete_file_descriptor_table(2112);
}

TEST(TestSyscalls2, get_file_descriptor_table) {
	struct file_descriptor_table *table, *result_table;

	TEST_ASSERT_NULL(get_file_descriptor_table(2112));
	table = allocate_file_descriptor_table(2112);

	table->total_descriptors = 3;

	result_table = get_file_descriptor_table(2112);
	TEST_ASSERT_EQUAL(2112, result_table->pid);
	TEST_ASSERT_EQUAL(3, result_table->total_descriptors);

	delete_file_descriptor_table(2112);
}

TEST(TestSyscalls2, delete_file_descriptor_table) {
	struct file_descriptor_table *table, *result_table;

	table = allocate_file_descriptor_table(2112);
	TEST_ASSERT_FALSE(table == NULL);

	delete_file_descriptor_table(2112);
	result_table = get_file_descriptor_table(2112);
	TEST_ASSERT_NULL(result_table);
}

TEST(TestSyscalls2, find_available_fd) {
	struct file_descriptor_table *table;

	table = allocate_file_descriptor_table(2112);

	TEST_ASSERT_EQUAL(-1, find_available_fd(4));
	TEST_ASSERT_EQUAL(3, find_available_fd(2112));

	table->entries[3].fd = 3;
	TEST_ASSERT_EQUAL(4, find_available_fd(2112));

	table->used_descriptors = table->total_descriptors;
	TEST_ASSERT_EQUAL(-1, find_available_fd(2112));

	delete_file_descriptor_table(2112);
}

TEST(TestSyscalls2, allocate_file_descriptor_entry) {
	struct file_descriptor_table *table;
	struct file_descriptor_entry *entry;
	
	entry = allocate_file_descriptor_entry(1632);

	table = get_file_descriptor_table(1632);
	TEST_ASSERT_FALSE(table == NULL);
	TEST_ASSERT_EQUAL(1632, table->pid);
	TEST_ASSERT_EQUAL(&table->entries[3], entry);

	TEST_ASSERT_EQUAL(4, table->used_descriptors);
	TEST_ASSERT_EQUAL(3, entry->fd);

	delete_file_descriptor_table(1632);
}

TEST(TestSyscalls2, allocate_file_descriptor_entry__when_table_is_full) {
	struct file_descriptor_table *table;
	struct file_descriptor_entry *entry, *entry2;
	
	entry = allocate_file_descriptor_entry(1632);

	table = get_file_descriptor_table(1632);
	TEST_ASSERT_EQUAL(8, table->total_descriptors);
	TEST_ASSERT_EQUAL(3, entry->fd);

	allocate_file_descriptor_entry(1632);
	allocate_file_descriptor_entry(1632);
	allocate_file_descriptor_entry(1632);
	allocate_file_descriptor_entry(1632);

	entry2 = allocate_file_descriptor_entry(1632);
	TEST_ASSERT_EQUAL(8, entry2->fd);
	TEST_ASSERT_EQUAL(16, table->total_descriptors);

	// Make sure previous file descriptor entries are intact
	TEST_ASSERT_EQUAL(3, entry->fd);

	delete_file_descriptor_table(1632);
}

TEST(TestSyscalls2, get_file_descriptor_entry) {
	struct file_descriptor_entry *entry, *result_entry;

	result_entry = get_file_descriptor_entry(2134, 0);
	TEST_ASSERT_NULL(result_entry);

	entry = allocate_file_descriptor_entry(2134);
	
	result_entry = get_file_descriptor_entry(2134, 8);
	TEST_ASSERT_NULL(result_entry);

	result_entry = get_file_descriptor_entry(2134, 4);
	TEST_ASSERT_NULL(result_entry);

	result_entry = get_file_descriptor_entry(2134, 3);
	TEST_ASSERT_FALSE(result_entry == NULL);
	TEST_ASSERT_EQUAL(entry, result_entry);
	TEST_ASSERT_EQUAL(3, result_entry->fd);

	delete_file_descriptor_table(2134);
}

TEST(TestSyscalls2, delete_file_descriptor_entry) {
	struct file_descriptor_entry *entry, *entry2, *result_entry;
	struct file_descriptor_table *table;

	// Make sure it doesn't blow up if we delete something that didn't exist before
	delete_file_descriptor_entry(2134, 3);

	entry = allocate_file_descriptor_entry(2134);
	table = get_file_descriptor_table(2134);

	entry2 = allocate_file_descriptor_entry(2134);

	TEST_ASSERT_EQUAL(5, table->used_descriptors);

	// Make sure we cannot delete the file descriptors 0, 1, 2
	delete_file_descriptor_entry(2134, 0);
	delete_file_descriptor_entry(2134, 1);
	delete_file_descriptor_entry(2134, 2);

	result_entry = get_file_descriptor_entry(2134, 0);
	TEST_ASSERT_EQUAL(0, result_entry->fd);
	result_entry = get_file_descriptor_entry(2134, 1);
	TEST_ASSERT_EQUAL(1, result_entry->fd);
	result_entry = get_file_descriptor_entry(2134, 2);
	TEST_ASSERT_EQUAL(2, result_entry->fd);
	
	TEST_ASSERT_EQUAL(3, entry->fd);
	delete_file_descriptor_entry(2134, 3);
	TEST_ASSERT_EQUAL(FD_NOT_USED, entry->fd);
	TEST_ASSERT_EQUAL(4, table->used_descriptors);

	TEST_ASSERT_EQUAL(4, entry2->fd);
	delete_file_descriptor_entry(2134, 4);
	TEST_ASSERT_EQUAL(FD_NOT_USED, entry2->fd);
	TEST_ASSERT_EQUAL(3, table->used_descriptors);

	delete_file_descriptor_table(2134);
}

TEST(TestSyscalls2, syscall2__get_pid) {
	// We just make sure here that we can stub properly the function syscall2__get_pid in tests
	// The function will call the fuse function in production mode though
	syscall2__pid = 3;
	TEST_ASSERT_EQUAL(3, syscall2__get_pid());
	syscall2__pid = 4;
	TEST_ASSERT_EQUAL(4, syscall2__get_pid());
}

TEST(TestSyscalls2, syscalls2__open) {
	struct inode inod1, inod2;
	struct file_descriptor_entry * fde;
	big_int inode_id_1, inode_id_2;

	init_disk_emulator();
	create_fs();


	ialloc(&inod1); // We allocate an inode just to have a valid inode number and use it for the purpose of the test
	// 1st syscalls2__open of that file
	syscall2__pid = 2122; // Simulate a Process calling syscalls2__open
	inode_id_1 = inod1.inode_id;
	syscall2__namei = inode_id_1; // Inode number returned by namei (which is the inode number being created a few steps before)

	TEST_ASSERT_EQUAL(3, syscalls2__open("whateverfile", O_RDONLY));
	
	get_inode(inode_id_1, &inod1);
	TEST_ASSERT((time(NULL) - inod1.last_accessed_file) <= 120); // We make sure last accessed time is set (2mn max the current time)

	fde = get_file_descriptor_entry(2122, 3);
	TEST_ASSERT_EQUAL(inode_id_1, fde->inode_number);
	TEST_ASSERT_EQUAL(READ, fde->mode);
	TEST_ASSERT_EQUAL(0, fde->byte_offset);


	ialloc(&inod2); // We allocate another inode for another file
	inode_id_2 = inod2.inode_id;
	syscall2__namei = inode_id_2;

	TEST_ASSERT_EQUAL(4, syscalls2__open("otherfile", O_WRONLY));
	
	get_inode(inode_id_2, &inod2);
	TEST_ASSERT((time(NULL) - inod2.last_accessed_file) <= 120); // We make sure last accessed time is set (2mn max the current time)

	fde = get_file_descriptor_entry(2122, 4);
	TEST_ASSERT_EQUAL(inode_id_2, fde->inode_number);
	TEST_ASSERT_EQUAL(WRITE, fde->mode);
	TEST_ASSERT_EQUAL(0, fde->byte_offset);


	syscall2__namei = inode_id_1;
	TEST_ASSERT_EQUAL(5, syscalls2__open("whateverfile", O_RDWR));
	
	get_inode(inode_id_1, &inod1);
	TEST_ASSERT((time(NULL) - inod1.last_accessed_file) <= 120); // We make sure last accessed time is set (2mn max the current time)

	fde = get_file_descriptor_entry(2122, 5);
	TEST_ASSERT_EQUAL(inode_id_1, fde->inode_number);
	TEST_ASSERT_EQUAL(READ_WRITE, fde->mode);
	TEST_ASSERT_EQUAL(0, fde->byte_offset);


	syscall2__pid = 1783; // We change the process calling syscalls2__open
	syscall2__namei = inode_id_1;
	TEST_ASSERT_EQUAL(3, syscalls2__open("whateverfile", O_RDWR));
	
	get_inode(inode_id_1, &inod1);
	TEST_ASSERT((time(NULL) - inod1.last_accessed_file) <= 120); // We make sure last accessed time is set (2mn max the current time)

	fde = get_file_descriptor_entry(1783, 3);
	TEST_ASSERT_EQUAL(inode_id_1, fde->inode_number);
	TEST_ASSERT_EQUAL(READ_WRITE, fde->mode);
	TEST_ASSERT_EQUAL(0, fde->byte_offset);

	delete_file_descriptor_table(2122);
	delete_file_descriptor_table(1783);
	free_disk_emulator();
}

TEST(TestSyscalls2, syscalls2__open__when_path_does_not_exist) {
	syscall2__namei = -1; // Simulate the fact that namei returns -1 for a path. It corresponds to a non valid path
	TEST_ASSERT_EQUAL(-1, syscalls2__open("invalid_path", O_RDWR));
}

TEST(TestSyscalls2, syscalls2__open__with_ocreate_mode) {
	struct inode inod;
	struct file_descriptor_entry * fde;
	big_int inode_id_1;

	init_disk_emulator();
	create_fs();

	ialloc(&inod); // We allocate an inode just to have a valid inode number and use it for the purpose of the test

	syscall2__pid = 2122; // Simulate a Process calling syscalls2__open
	inode_id_1 = inod.inode_id;
	syscall2__namei = inode_id_1;

	TEST_ASSERT_EQUAL(3, syscalls2__open("whateverfile", O_CREAT|O_WRONLY, 700));
	TEST_ASSERT_EQUAL(700, syscall2__mknod); // Make sure we called mknod from syscalls2__open and the right mode was provided to mknod


	get_inode(inode_id_1, &inod);
	TEST_ASSERT((time(NULL) - inod.last_accessed_file) <= 120); // We make sure last accessed time is set (2mn max the current time)

	fde = get_file_descriptor_entry(2122, 3);
	TEST_ASSERT_EQUAL(inode_id_1, fde->inode_number);
	TEST_ASSERT_EQUAL(WRITE, fde->mode);
	TEST_ASSERT_EQUAL(0, fde->byte_offset);


	TEST_ASSERT_EQUAL(4, syscalls2__open("whateverfile", O_CREAT|O_RDWR, 751));
	TEST_ASSERT_EQUAL(751, syscall2__mknod); // Make sure we called mknod from syscalls2__open and the right mode was provided to mknod

	// COMMENTED OUT UNTIL IGET AFTER IPUT IS FIXED IN INODES_HANDLER
	get_inode(inode_id_1, &inod);
	TEST_ASSERT((time(NULL) - inod.last_accessed_file) <= 120); // We make sure last accessed time is set (2mn max the current time)

	fde = get_file_descriptor_entry(2122, 4);
	TEST_ASSERT_EQUAL(inode_id_1, fde->inode_number);
	TEST_ASSERT_EQUAL(READ_WRITE, fde->mode);
	TEST_ASSERT_EQUAL(0, fde->byte_offset);

	delete_file_descriptor_table(2122);
	free_disk_emulator();
}

TEST(TestSyscalls2, syscalls2__open__with_oappend_flag) {
	// IMPLEMENT THE O_APPEND FLAG AND ADD TEST HERE
}

TEST(TestSyscalls2, syscalls2__close) {
	struct file_descriptor_entry *entry;
	struct file_descriptor_table *table;

	entry = allocate_file_descriptor_entry(2122); // file descriptor will be 3
	TEST_ASSERT_EQUAL(3, entry->fd);
	entry = allocate_file_descriptor_entry(2122); // file descriptor will be 4
	TEST_ASSERT_EQUAL(4, entry->fd);
	table = get_file_descriptor_table(2122);
	TEST_ASSERT_EQUAL(5, table->used_descriptors);

	entry = allocate_file_descriptor_entry(1897); // file descriptor will be 3
	TEST_ASSERT_EQUAL(3, entry->fd);
	table = get_file_descriptor_table(1897);
	TEST_ASSERT_EQUAL(4, table->used_descriptors);

	// We syscalls2__close a file in process 2122
	syscall2__pid = 2122; // Simulate a Process calling syscalls2__close
	TEST_ASSERT_EQUAL(0, syscalls2__close(3));
	table = get_file_descriptor_table(2122);
	TEST_ASSERT_EQUAL(4, table->used_descriptors);	
	entry = get_file_descriptor_entry(2122, 3);
	TEST_ASSERT_NULL(entry);

	// We try to syscalls2__close a descriptor that was already syscalls2__closed
	syscall2__pid = 2122; // Simulate a Process calling syscalls2__close
	TEST_ASSERT_EQUAL(-1, syscalls2__close(3));

	// We syscalls2__close a file in process 1897
	syscall2__pid = 1897; // Simulate a Process calling syscalls2__close
	TEST_ASSERT_EQUAL(0, syscalls2__close(3));
	table = get_file_descriptor_table(1897);
	TEST_ASSERT_NULL(table);

	// We syscalls2__close a file in process 2122
	syscall2__pid = 2122; // Simulate a Process calling syscalls2__close
	TEST_ASSERT_EQUAL(0, syscalls2__close(4));
	table = get_file_descriptor_table(2122);
	TEST_ASSERT_NULL(table);
}

TEST(TestSyscalls2, syscalls2__open_syscalls2__close) {
	struct inode inod1, inod2, inod3, inod4;
	big_int inode_id_1, inode_id_2, inode_id_3, inode_id_4;

	init_disk_emulator();
	create_fs();

	syscall2__pid = 2122; // Simulate a Process calling syscalls2__open

	ialloc(&inod1); // We allocate an inode just to have a valid inode number and use it for the purpose of the test	
	inode_id_1 = inod1.inode_id;
	syscall2__namei = inode_id_1;

	TEST_ASSERT_EQUAL(3, syscalls2__open("filepath1", O_RDONLY));


	ialloc(&inod2); // We allocate an inode just to have a valid inode number and use it for the purpose of the test
	inode_id_2 = inod2.inode_id;
	syscall2__namei = inode_id_2;

	TEST_ASSERT_EQUAL(4, syscalls2__open("filepath2", O_RDONLY));

	//We try to syscalls2__close 3rd fd now
	TEST_ASSERT_EQUAL(0, syscalls2__close(3));

	// We try to syscalls2__open a file again and expect 3rd file descriptor to be reused for other file
	ialloc(&inod3); // We allocate an inode just to have a valid inode number and use it for the purpose of the test
	inode_id_3 = inod3.inode_id;
	syscall2__namei = inode_id_3;

	TEST_ASSERT_EQUAL(3, syscalls2__open("filepath3", O_RDONLY)); //  we assert that that file descriptor is reused but not for the same inode
	TEST_ASSERT_EQUAL(inode_id_3, get_file_descriptor_entry(2122, 3)->inode_number);

	ialloc(&inod4); // We allocate an inode just to have a valid inode number and use it for the purpose of the test
	inode_id_4 = inod4.inode_id;
	syscall2__namei = inode_id_4;

	TEST_ASSERT_EQUAL(5, syscalls2__open("filepath4", O_RDONLY)); // we assert that the file descriptor uses 5 and not 4

	delete_file_descriptor_table(2122);
	free_disk_emulator();
}

TEST(TestSyscalls2, get_size_of_file) {
	TEST_ASSERT_EQUAL(0, get_size_of_file(0, 0));
	TEST_ASSERT_EQUAL(156, get_size_of_file(1, 156));
	TEST_ASSERT_EQUAL(4096, get_size_of_file(1, 4096));
	TEST_ASSERT_EQUAL(4097, get_size_of_file(2, 1));
	TEST_ASSERT_EQUAL(40960, get_size_of_file(10, 4096));
	TEST_ASSERT_EQUAL(40961, get_size_of_file(11, 1));
}

TEST(TestSyscalls2, convert_byte_offset_to_ith_datablock) {
	TEST_ASSERT_EQUAL(1, convert_byte_offset_to_ith_datablock(0));
	TEST_ASSERT_EQUAL(1, convert_byte_offset_to_ith_datablock(1));
	TEST_ASSERT_EQUAL(1, convert_byte_offset_to_ith_datablock(4095));
	TEST_ASSERT_EQUAL(2, convert_byte_offset_to_ith_datablock(4096));
	TEST_ASSERT_EQUAL(2, convert_byte_offset_to_ith_datablock(4097));
}

TEST(TestSyscalls2, is_ith_block_in_range_of_direct_and_indirect_blocks) {
	TEST_ASSERT_EQUAL(0, is_ith_block_in_range_of_direct_and_indirect_blocks(0));
	TEST_ASSERT_EQUAL(1, is_ith_block_in_range_of_direct_and_indirect_blocks(1));
	TEST_ASSERT_EQUAL(1, is_ith_block_in_range_of_direct_and_indirect_blocks(NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH + BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH + BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH));
	TEST_ASSERT_EQUAL(0, is_ith_block_in_range_of_direct_and_indirect_blocks(NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH + BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH + BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH + 1));
}

TEST(TestSyscalls2, get_ith_datablock_number) {
	struct data_block *db, *single_indirect_db; 
	struct data_block *double_indirect_level1_db; 
	struct data_block *double_indirect_level2_first_first_db, *double_indirect_level2_second_first_db, *double_indirect_level2_last_last_db;
	struct data_block *triple_indirect_level1_db;
	struct data_block *triple_indirect_level2_first_first_db, *triple_indirect_level3_first_db;
	struct data_block *triple_indirect_level2_second_first_db;
	struct data_block *triple_indirect_level2_last_last_db, *triple_indirect_level3_last_db;
	big_int i, single_indirect_block_id;

	init_disk_emulator();
	create_fs();

	struct inode inod;

	/* *******
		TEST OF DIRECT BLOCKS
	******* */

	for (i = 1; i <= NUM_DIRECT_BLOCKS; i++) {
		db = data_block_alloc();
		inod.direct_blocks[i-1] = db->data_block_id;
		// printf("i: %llu, block id: %llu, stored in inode: %llu\n", i, db->data_block_id, inod.direct_blocks[i-1]);
		TEST_ASSERT(inod.direct_blocks[i-1] != 0);

		TEST_ASSERT_EQUAL(inod.direct_blocks[i-1], get_ith_datablock_number(&inod, i));
		free_data_block_pointer(db);
	}

	/* *******
		TEST OF SINGLE INDIRECT BLOCKS
	******* */

	single_indirect_db = data_block_alloc();
	single_indirect_block_id= single_indirect_db->data_block_id;
	inod.single_indirect_block = single_indirect_block_id;
	
	// Test first block number in indirect block is obtained correctly
	db = data_block_alloc();
	memcpy(single_indirect_db->block, &db->data_block_id, sizeof(big_int));
	bwrite(single_indirect_db); // Let's save the block containing block numbers on disk
	TEST_ASSERT_EQUAL(db->data_block_id, get_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + 1));
	free_data_block_pointer(db);

	// Test last block number in indirect block is obtained correctly
	db = data_block_alloc();
	memcpy(&single_indirect_db->block[(BLOCK_ID_LIST_LENGTH-1) * sizeof(big_int)], &db->data_block_id, sizeof(big_int));
	bwrite(single_indirect_db); // Let's save the block containing block numbers on disk
	TEST_ASSERT_EQUAL(db->data_block_id, get_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH));
	free_data_block_pointer(db);

	free_data_block_pointer(single_indirect_db);

	

	/* *******
		TEST OF DOUBLE INDIRECT BLOCKS
	******* */

	double_indirect_level1_db = data_block_alloc();
	inod.double_indirect_block = double_indirect_level1_db->data_block_id;


	// Set first datablock in level 2 of double indirect list
	double_indirect_level2_first_first_db = data_block_alloc();
	memcpy(double_indirect_level1_db->block, &double_indirect_level2_first_first_db->data_block_id, sizeof(big_int));
	bwrite(double_indirect_level1_db);

	db = data_block_alloc();
	memcpy(double_indirect_level2_first_first_db->block, &db->data_block_id, sizeof(big_int));
	bwrite(double_indirect_level2_first_first_db);
	TEST_ASSERT_EQUAL(db->data_block_id, get_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH + 1));
	free_data_block_pointer(db);

	free_data_block_pointer(double_indirect_level2_first_first_db);



	// Set first datablock in level 2 of the second datablock in level1 of double indirect list
	double_indirect_level2_second_first_db = data_block_alloc();
	memcpy(&double_indirect_level1_db->block[sizeof(big_int)], &double_indirect_level2_second_first_db->data_block_id, sizeof(big_int));
	bwrite(double_indirect_level1_db);

	db = data_block_alloc();
	memcpy(double_indirect_level2_second_first_db->block, &db->data_block_id, sizeof(big_int));
	bwrite(double_indirect_level2_second_first_db);
	TEST_ASSERT_EQUAL(db->data_block_id, get_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH + BLOCK_ID_LIST_LENGTH + 1));
	free_data_block_pointer(db);

	free_data_block_pointer(double_indirect_level2_second_first_db);



	// Set last datablock in level 2 of the last datablock in level1 of double indirect list
	double_indirect_level2_last_last_db = data_block_alloc();
	memcpy(&double_indirect_level1_db->block[(BLOCK_ID_LIST_LENGTH-1) * sizeof(big_int)], &double_indirect_level2_last_last_db->data_block_id, sizeof(big_int));
	bwrite(double_indirect_level1_db);

	db = data_block_alloc();
	memcpy(&double_indirect_level2_last_last_db->block[(BLOCK_ID_LIST_LENGTH-1) * sizeof(big_int)], &db->data_block_id, sizeof(big_int));
	bwrite(double_indirect_level2_last_last_db);
	TEST_ASSERT_EQUAL(db->data_block_id, get_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH + BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH));
	free_data_block_pointer(db);

	free_data_block_pointer(double_indirect_level2_last_last_db);



	free_data_block_pointer(double_indirect_level1_db);







	/* *******
		TEST OF TRIPLE INDIRECT BLOCKS
	******* */

	triple_indirect_level1_db = data_block_alloc();
	inod.triple_indirect_block = triple_indirect_level1_db->data_block_id;



	// Set first datablock in level 3 of the first datablock in level 2 of the first datablock in level 1 of triple indirect list
	triple_indirect_level2_first_first_db = data_block_alloc();
	memcpy(triple_indirect_level1_db->block, &triple_indirect_level2_first_first_db->data_block_id, sizeof(big_int));
	bwrite(triple_indirect_level1_db);

	triple_indirect_level3_first_db = data_block_alloc();
	memcpy(triple_indirect_level2_first_first_db->block, &triple_indirect_level3_first_db->data_block_id, sizeof(big_int));
	bwrite(triple_indirect_level2_first_first_db);


	db = data_block_alloc();
	memcpy(triple_indirect_level3_first_db->block, &db->data_block_id, sizeof(big_int));
	bwrite(triple_indirect_level3_first_db);
	TEST_ASSERT_EQUAL(db->data_block_id, get_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH + BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH + 1));
	free_data_block_pointer(db);


	free_data_block_pointer(triple_indirect_level3_first_db);
	free_data_block_pointer(triple_indirect_level2_first_first_db);




	// Set first datablock in level 3 of the first datablock in level 2 of the second datablock in level 1 of triple indirect list
	triple_indirect_level2_second_first_db = data_block_alloc();
	memcpy(&triple_indirect_level1_db->block[sizeof(big_int)], &triple_indirect_level2_second_first_db->data_block_id, sizeof(big_int));
	bwrite(triple_indirect_level1_db);

	triple_indirect_level3_first_db = data_block_alloc();
	memcpy(triple_indirect_level2_second_first_db->block, &triple_indirect_level3_first_db->data_block_id, sizeof(big_int));
	bwrite(triple_indirect_level2_second_first_db);


	db = data_block_alloc();
	memcpy(triple_indirect_level3_first_db->block, &db->data_block_id, sizeof(big_int));
	bwrite(triple_indirect_level3_first_db);
	TEST_ASSERT_EQUAL(db->data_block_id, get_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH + 2 * BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH + 1));
	free_data_block_pointer(db);


	free_data_block_pointer(triple_indirect_level3_first_db);
	free_data_block_pointer(triple_indirect_level2_second_first_db);




	// Set last datablock in level 3 of the last datablock in level 2 of the last datablock in level 1 of triple indirect list
	triple_indirect_level2_last_last_db = data_block_alloc();
	memcpy(&triple_indirect_level1_db->block[(BLOCK_ID_LIST_LENGTH-1) * sizeof(big_int)], &triple_indirect_level2_last_last_db->data_block_id, sizeof(big_int));
	bwrite(triple_indirect_level1_db);

	triple_indirect_level3_last_db = data_block_alloc();
	memcpy(&triple_indirect_level2_last_last_db->block[(BLOCK_ID_LIST_LENGTH-1) * sizeof(big_int)], &triple_indirect_level3_last_db->data_block_id, sizeof(big_int));
	bwrite(triple_indirect_level2_last_last_db);


	db = data_block_alloc();
	memcpy(&triple_indirect_level3_last_db->block[(BLOCK_ID_LIST_LENGTH-1) * sizeof(big_int)], &db->data_block_id, sizeof(big_int));
	bwrite(triple_indirect_level3_last_db);
	TEST_ASSERT_EQUAL(db->data_block_id, get_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH + BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH + BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH));
	free_data_block_pointer(db);


	free_data_block_pointer(triple_indirect_level3_last_db);
	free_data_block_pointer(triple_indirect_level2_last_last_db);


	free_data_block_pointer(triple_indirect_level1_db);

	free_disk_emulator();
}

TEST(TestSyscalls2, set_ith_datablock_number) {
	big_int i;
	int inode_number;

	init_disk_emulator();
	create_fs();

	struct inode inod;
	ialloc(&inod);
	inode_number = inod.inode_id;

	/* *******
		TEST OF DIRECT BLOCKS
	******* */

	for (i = 1; i <= NUM_DIRECT_BLOCKS; i++) {
		TEST_ASSERT_EQUAL(0, get_ith_datablock_number(&inod, i));
		set_ith_datablock_number(&inod, i, i);

		get_inode(inode_number, &inod);
		TEST_ASSERT_EQUAL(i, inod.direct_blocks[i-1]);
		TEST_ASSERT_EQUAL(i, get_ith_datablock_number(&inod, i));
	}


	/* *******
		TEST OF SINGLE INDIRECT BLOCKS
	******* */

	TEST_ASSERT_EQUAL(0, inod.single_indirect_block);
	
	// Test first block number in indirect block is obtained correctly
	TEST_ASSERT_EQUAL(0, get_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + 1));
	set_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + 1, 433);
	
	get_inode(inode_number, &inod);
	TEST_ASSERT_EQUAL(433, get_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + 1));
	TEST_ASSERT(inod.single_indirect_block != 0);

	// Test last block number in indirect block is obtained correctly
	TEST_ASSERT_EQUAL(0, get_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH));
	set_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH, 1734);
	
	get_inode(inode_number, &inod);
	TEST_ASSERT_EQUAL(1734, get_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH));
	

	/* *******
		TEST OF DOUBLE INDIRECT BLOCKS
	******* */

	TEST_ASSERT_EQUAL(0, inod.double_indirect_block);

	// Set first datablock in level 2 of double indirect list
	TEST_ASSERT_EQUAL(0, get_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH + 1));
	set_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH + 1, 67);

	get_inode(inode_number, &inod);
	TEST_ASSERT_EQUAL(67, get_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH + 1));
	TEST_ASSERT(inod.double_indirect_block != 0);

	// Set first datablock in level 2 of the second datablock in level1 of double indirect list
	TEST_ASSERT_EQUAL(0, get_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH + BLOCK_ID_LIST_LENGTH + 1));
	set_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH + BLOCK_ID_LIST_LENGTH + 1, 10675);

	get_inode(inode_number, &inod);
	TEST_ASSERT_EQUAL(10675, get_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH + BLOCK_ID_LIST_LENGTH + 1));

	// Set last datablock in level 2 of the last datablock in level1 of double indirect list
	TEST_ASSERT_EQUAL(0, get_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH + BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH));
	set_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH + BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH, 876);

	get_inode(inode_number, &inod);
	TEST_ASSERT_EQUAL(876, get_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH + BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH));



	/* *******
		TEST OF TRIPLE INDIRECT BLOCKS
	******* */

	TEST_ASSERT_EQUAL(0, inod.triple_indirect_block);

	// Set first datablock in level 3 of the first datablock in level 2 of the first datablock in level 1 of triple indirect list
	TEST_ASSERT_EQUAL(0, get_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH + BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH + 1));
	set_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH + BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH + 1, 123);

	get_inode(inode_number, &inod);
	TEST_ASSERT_EQUAL(123, get_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH + BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH + 1));
	TEST_ASSERT(inod.triple_indirect_block != 0);

	// Set first datablock in level 3 of the first datablock in level 2 of the second datablock in level 1 of triple indirect list
	TEST_ASSERT_EQUAL(0, get_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH + 2 * BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH + 1));
	set_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH + 2 * BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH + 1, 989);

	get_inode(inode_number, &inod);
	TEST_ASSERT_EQUAL(989, get_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH + 2 * BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH + 1));

	// Set last datablock in level 3 of the last datablock in level 2 of the last datablock in level 1 of triple indirect list
	TEST_ASSERT_EQUAL(0, get_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH + BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH + BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH));
	set_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH + BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH + BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH, 48973);

	get_inode(inode_number, &inod);
	TEST_ASSERT_EQUAL(48973, get_ith_datablock_number(&inod, NUM_DIRECT_BLOCKS + BLOCK_ID_LIST_LENGTH + BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH + BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH * BLOCK_ID_LIST_LENGTH));

	free_disk_emulator();
}

TEST(TestSyscalls2, read__error_cases__fd_does_not_exist__or_fde_set_to_write_only__or_offset_too_big) {
	struct inode inod;
	char buffer[256];
	int fd1, fd2;

	init_disk_emulator();
	create_fs();

	ialloc(&inod);

	syscall2__pid = 2122; // Simulate a Process calling syscalls2__open
	syscall2__namei = inod.inode_id; // "stub" namei by returning this inode id

	fd1 = syscalls2__open("filepath", O_WRONLY);
	TEST_ASSERT_EQUAL(3, fd1);
	
	TEST_ASSERT_EQUAL(-1, syscalls2__pread(4, buffer, 21, 0)); // test that -1 returned if we provide a file descriptor that doesn't exist
	TEST_ASSERT_EQUAL(-1, syscalls2__pread(fd1, buffer, 21, 0)); // -1 returned if we try to read a file that is write only

	syscalls2__close(fd1);

	inod.num_blocks = 1;
	inod.num_used_bytes_in_last_block = 1;
	put_inode(&inod);

	fd2 = syscalls2__open("filepath", O_RDONLY);
	TEST_ASSERT_EQUAL(3, fd2);

	TEST_ASSERT_EQUAL(0, syscalls2__pread(fd2, buffer, 1, 1)); // We try to read after limit of file, it cannot work
	
	syscalls2__close(fd2);

	free_disk_emulator();	
}

TEST(TestSyscalls2, read__read_bytes_in_last_block_of_file) {
	struct data_block *block1, *block2;
	struct inode inod;
	char buffer[256];
	int fd;
	int i;

	init_disk_emulator();
	create_fs();

	ialloc(&inod);

	syscall2__pid = 2122; // Simulate a Process calling syscalls2__open
	syscall2__namei = inod.inode_id; // "stub" namei by returning this inode id

	block1 = data_block_alloc();
	block2 = data_block_alloc();

	memset(block1->block, 'a', BLOCK_SIZE); // Write a full block of a
	memset(block2->block, 'b', 100); // Write 100 b in the block.
	bwrite(block1);
	bwrite(block2);

	inod.direct_blocks[4] = block1->data_block_id;
	inod.direct_blocks[5] = block2->data_block_id;
	inod.num_blocks = 6; // 6 blocks are used in this file
	inod.num_used_bytes_in_last_block = 100; // end of file is at byte offset 100 included in 6th block
	put_inode(&inod);

	fd = syscalls2__open("filepath", O_RDONLY);

	memset(buffer, 0, 256);
	TEST_ASSERT_EQUAL(15, syscalls2__pread(fd, buffer, 15, BLOCK_SIZE * 4 + BLOCK_SIZE - 3)); // We read 15 characters

	TEST_ASSERT_EQUAL('a', buffer[0]);
	TEST_ASSERT_EQUAL('a', buffer[1]);
	TEST_ASSERT_EQUAL('a', buffer[2]);
	for (i = 3; i < 15; i++) {
		TEST_ASSERT_EQUAL('b', buffer[i]);
	}
	TEST_ASSERT_EQUAL(0, buffer[15]);

	syscalls2__close(fd);

	free_data_block_pointer(block1);
	free_data_block_pointer(block2);
	free_disk_emulator();
}

TEST(TestSyscalls2, read__less_bytes_to_read_than_a_block) {
	struct data_block *block1, *block2;
	struct inode inod;
	char buffer[256];
	int fd;

	init_disk_emulator();
	create_fs();

	ialloc(&inod);

	syscall2__pid = 2122; // Simulate a Process calling syscalls2__open
	syscall2__namei = inod.inode_id; // "stub" namei by returning this inode id

	block1 = data_block_alloc();
	block2 = data_block_alloc();

	block1->block[0] = 'a';
	block1->block[1] = 'b';
	block1->block[2] = 'c';
	block1->block[3] = 'd';
	block1->block[4] = 'e';
	memset(&block1->block[5], 'a', BLOCK_SIZE-5); // Write a full block of a
	memset(block2->block, 'b', 100); // Write 100 b in the block.
	bwrite(block1);
	bwrite(block2);

	inod.direct_blocks[4] = block1->data_block_id;
	inod.direct_blocks[5] = block2->data_block_id;
	inod.num_blocks = 6; // 6 blocks are used in this file
	inod.num_used_bytes_in_last_block = 100; // end of file is at byte offset 100 included in 6th block
	put_inode(&inod);

	fd = syscalls2__open("filepath", O_RDONLY);

	memset(buffer, 0, 256);
	TEST_ASSERT_EQUAL(3, syscalls2__pread(fd, buffer, 3, BLOCK_SIZE * 4 + 1)); // We read 3 characters

	TEST_ASSERT_EQUAL('b', buffer[0]);
	TEST_ASSERT_EQUAL('c', buffer[1]);
	TEST_ASSERT_EQUAL('d', buffer[2]);
	TEST_ASSERT_EQUAL(0, buffer[3]);

	syscalls2__close(fd);

	free_data_block_pointer(block1);
	free_data_block_pointer(block2);
	free_disk_emulator();
}

TEST(TestSyscalls2, read__more_bytes_to_read_than_a_block) {
	struct data_block *block1, *block2, *block3;
	struct inode inod;
	char buffer[8200];
	int fd;
	int i;

	init_disk_emulator();
	create_fs();

	ialloc(&inod);

	syscall2__pid = 2122; // Simulate a Process calling syscalls2__open
	syscall2__namei = inod.inode_id; // "stub" namei by returning this inode id

	block1 = data_block_alloc();
	block2 = data_block_alloc();
	block3 = data_block_alloc();

	memset(block1->block, 'a', BLOCK_SIZE); // Write a full block of a
	memset(block2->block, 'b', BLOCK_SIZE); // Write 100 b in the block.
	memset(block3->block, 'c', BLOCK_SIZE); // Write 100 b in the block.
	bwrite(block1);
	bwrite(block2);
	bwrite(block3);

	inod.direct_blocks[4] = block1->data_block_id;
	inod.direct_blocks[5] = block2->data_block_id;
	inod.direct_blocks[6] = block3->data_block_id;
	inod.num_blocks = 10; // 10 blocks are used in this file
	inod.num_used_bytes_in_last_block = 67; // end of file is at byte offset 100 included in 6th block
	put_inode(&inod);

	fd = syscalls2__open("filepath", O_RDONLY);

	memset(buffer, 0, 8200);
	TEST_ASSERT_EQUAL(8195, syscalls2__pread(fd, buffer, 8195, BLOCK_SIZE * 4)); // We read 8195 characters

	for (i = 0; i < BLOCK_SIZE; i++) {
		TEST_ASSERT_EQUAL('a', buffer[i]);
	}
	for (i = BLOCK_SIZE; i < 2 * BLOCK_SIZE; i++) {
		TEST_ASSERT_EQUAL('b', buffer[i]);	
	}
	TEST_ASSERT_EQUAL('c', buffer[2 * BLOCK_SIZE]);	
	TEST_ASSERT_EQUAL('c', buffer[2 * BLOCK_SIZE + 1]);	
	TEST_ASSERT_EQUAL('c', buffer[2 * BLOCK_SIZE + 2]);	
	TEST_ASSERT_EQUAL(0, buffer[2 * BLOCK_SIZE + 3]);	

	syscalls2__close(fd);

	free_data_block_pointer(block1);
	free_data_block_pointer(block2);
	free_disk_emulator();
}

TEST(TestSyscalls2, read__more_bytes_to_read_than_available_in_file) {
	struct data_block *block1;
	struct inode inod;
	char buffer[256];
	int fd;
	int i;

	init_disk_emulator();
	create_fs();

	ialloc(&inod);

	syscall2__pid = 2122; // Simulate a Process calling syscalls2__open
	syscall2__namei = inod.inode_id; // "stub" namei by returning this inode id

	block1 = data_block_alloc();

	memset(block1->block, 'a', 15);
	bwrite(block1);

	inod.direct_blocks[4] = block1->data_block_id;
	inod.num_blocks = 5; // 5 blocks are used in this file
	inod.num_used_bytes_in_last_block = 15; // end of file is at byte offset 15 included in 6th block
	put_inode(&inod);

	fd = syscalls2__open("filepath", O_RDONLY);

	memset(buffer, 0, 256);
	TEST_ASSERT_EQUAL(13, syscalls2__pread(fd, buffer, 14, BLOCK_SIZE * 4 + 2)); // We want to read 14 characters but EOF reached after 13 characters

	for (i = 0; i < 13; i++) {
		TEST_ASSERT_EQUAL('a', buffer[i]);
	}
	TEST_ASSERT_EQUAL(0, buffer[14]);	

	TEST_ASSERT_EQUAL(13, syscalls2__pread(fd, buffer, 13, BLOCK_SIZE * 4 + 2)); // We read 13 characters, the 13th character is the last one

	syscalls2__close(fd);

	free_data_block_pointer(block1);
	free_disk_emulator();
}

TEST(TestSyscalls2, pwrite__write_after_end_of_file) {
	struct data_block *block1;
	struct inode inod;
	char buffer[256];
	char buffer2[7 * BLOCK_SIZE];
	int fd1, fd2, i;

	init_disk_emulator();
	create_fs();

	ialloc(&inod);

	syscall2__pid = 2122; // Simulate a Process calling syscalls2__open
	syscall2__namei = inod.inode_id; // "stub" namei by returning this inode id

	block1 = data_block_alloc();

	memset(block1->block, 'a', 15);
	bwrite(block1);

	inod.direct_blocks[4] = block1->data_block_id;
	inod.num_blocks = 5; // 5 blocks are used in this file
	inod.num_used_bytes_in_last_block = 15; // end of file is at byte offset 16 included in 5th block
	put_inode(&inod);


	fd1 = syscalls2__open("filepath", O_WRONLY);

	memset(buffer, 'b', 15);
	syscalls2__pwrite(fd1, buffer, 7, 6 * BLOCK_SIZE + 20);

	syscalls2__close(fd1);

	get_inode(syscall2__namei, &inod);
	TEST_ASSERT_EQUAL(7, inod.num_blocks);
	TEST_ASSERT_EQUAL(27, inod.num_used_bytes_in_last_block);

	fd2 = syscalls2__open("filepath", O_RDONLY);
	TEST_ASSERT_EQUAL(6 * BLOCK_SIZE + 27, syscalls2__pread(fd2, buffer2, 7 * BLOCK_SIZE, 0));

	for (i = 0; i < 4 * BLOCK_SIZE; i++) {
		TEST_ASSERT_EQUAL(0, buffer2[i]);
	}

	for (i = 4 * BLOCK_SIZE; i < 4 * BLOCK_SIZE + 15; i++) {
		TEST_ASSERT_EQUAL('a', buffer2[i]);
	}

	for (i = 4 * BLOCK_SIZE + 15; i < 6 * BLOCK_SIZE + 20; i++) {
		TEST_ASSERT_EQUAL(0, buffer2[i]);
	}

	for (i = 6 * BLOCK_SIZE + 20; i < 6 * BLOCK_SIZE + 27; i++) {
		TEST_ASSERT_EQUAL('b', buffer2[i]);
	}

	TEST_ASSERT_EQUAL(0, buffer2[6 * BLOCK_SIZE + 27]);

	syscalls2__close(fd2);

	free_disk_emulator();
}

TEST(TestSyscalls2, pwrite__write_in_a_block_that_was_already_written) {
	struct data_block *block1;
	struct inode inod;
	char buffer[2 * BLOCK_SIZE];
	char buffer2[2 * BLOCK_SIZE];
	int fd1, fd2, i;

	init_disk_emulator();
	create_fs();

	ialloc(&inod);

	syscall2__pid = 2122; // Simulate a Process calling syscalls2__open
	syscall2__namei = inod.inode_id; // "stub" namei by returning this inode id

	block1 = data_block_alloc();

	memset(&block1->block[10], 'a', 5);
	bwrite(block1);

	inod.direct_blocks[1] = block1->data_block_id;
	inod.num_blocks = 2;
	inod.num_used_bytes_in_last_block = 15;
	put_inode(&inod);


	fd1 = syscalls2__open("filepath", O_WRONLY);

	memset(buffer, 'b', BLOCK_SIZE + 8);
	syscalls2__pwrite(fd1, buffer, BLOCK_SIZE + 8, 0);

	syscalls2__close(fd1);

	get_inode(syscall2__namei, &inod);
	TEST_ASSERT_EQUAL(2, inod.num_blocks);
	TEST_ASSERT_EQUAL(15, inod.num_used_bytes_in_last_block);

	fd2 = syscalls2__open("filepath", O_RDONLY);
	TEST_ASSERT_EQUAL(BLOCK_SIZE + 15, syscalls2__pread(fd2, buffer2, 2 * BLOCK_SIZE, 0));

	for (i = 0; i < BLOCK_SIZE + 8; i++) {
		TEST_ASSERT_EQUAL('b', buffer2[i]);
	}

	for (i = BLOCK_SIZE + 8; i < BLOCK_SIZE + 10; i++) {
		TEST_ASSERT_EQUAL(0, buffer2[i]);
	}

	for (i = BLOCK_SIZE + 10; i < BLOCK_SIZE + 15; i++) {
		TEST_ASSERT_EQUAL('a', buffer2[i]); // Make sure we have not overriden the existing characters in the block
	}

	TEST_ASSERT_EQUAL(0, buffer2[BLOCK_SIZE + 15]);

	syscalls2__close(fd2);

	free_disk_emulator();
}
