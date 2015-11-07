#include "syscalls2.h"

// UTILITIES

struct file_descriptor_table *file_descriptor_tables = NULL;

struct file_descriptor_table * get_file_descriptor_table(int pid) {
	struct file_descriptor_table * table;

	HASH_FIND_INT(file_descriptor_tables, &pid, table);
	return table;
}

void set_file_descriptor_table(int pid, struct file_descriptor_table * table) {
	struct file_descriptor_table * result_table;

	HASH_FIND_INT(file_descriptor_tables, &pid, result_table);
    
    if (result_table == NULL) {
      HASH_ADD_INT(file_descriptor_tables, pid, table);
    }
}

void delete_file_descriptor_table(int pid) {
	struct file_descriptor_table * table;

	table = get_file_descriptor_table(pid);

	if (table) {
		HASH_DEL(file_descriptor_tables, table);
	}
}
