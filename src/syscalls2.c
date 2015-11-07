#include "syscalls2.h"

// UTILITIES

struct file_descriptor_table *file_descriptor_tables = NULL;

struct file_descriptor_table * allocate_file_descriptor_table(int pid) {
	struct file_descriptor_table * result_table;

	HASH_FIND_INT(file_descriptor_tables, &pid, result_table);
    
    if (result_table == NULL) {
    	int i;

    	struct file_descriptor_table * table = (struct file_descriptor_table *) malloc(sizeof(struct file_descriptor_table));
    	table->pid = pid;

    	table->entries = (struct file_descriptor_entry *) malloc(8 * sizeof(struct file_descriptor_entry));
    	table->total_descriptors = 8;
    	table->used_descriptors = 3;

    	table->entries[0].fd = 0;
    	table->entries[1].fd = 1;
    	table->entries[2].fd = 2;

    	for (i = 3; i < table->total_descriptors; i++) {
    		table->entries[i].fd = FD_NOT_USED;
    	}
    	
    	HASH_ADD_INT(file_descriptor_tables, pid, table);
    	return table;
    }
    return NULL;
}

struct file_descriptor_table * get_file_descriptor_table(int pid) {
	struct file_descriptor_table * table;

	HASH_FIND_INT(file_descriptor_tables, &pid, table);
	return table;
}

void delete_file_descriptor_table(int pid) {
	struct file_descriptor_table * table;

	table = get_file_descriptor_table(pid);

	if (table) {
		HASH_DEL(file_descriptor_tables, table);

		free(table->entries);
		free(table);
	}
}

int find_available_fd(int pid) {
	struct file_descriptor_table * table;
	int i;

	table = get_file_descriptor_table(pid);

	if (table == NULL) {
		return -1;
	}

	if (table->total_descriptors == table->used_descriptors) {
		return -1;
	}
	for (i = 3; i < table->total_descriptors; i++) {
		if (table->entries[i].fd == FD_NOT_USED) {
			return i;
		}
	}
	return -1;
}

struct file_descriptor_entry * allocate_file_descriptor_entry(int pid) {
	struct file_descriptor_table * table;
	struct file_descriptor_entry * entry;
	int available_fd;

	// Allocate file descriptor table for the process if it doesn't exist
	table = get_file_descriptor_table(pid);
	if (table == NULL) {
		table = allocate_file_descriptor_table(pid);
	}

	available_fd = find_available_fd(pid);

	if (available_fd == -1) {
		// Need to resize array of entries to twice the size
		available_fd = table->total_descriptors;
		table->entries = realloc(table->entries, ((table->total_descriptors) * 2 * sizeof(struct file_descriptor_entry)));
		table->total_descriptors = table->total_descriptors * 2;
	}
	entry = &table->entries[available_fd];
	entry->fd = available_fd;
	table->used_descriptors++;

	return entry;
}

struct file_descriptor_entry * get_file_descriptor_entry(int pid, int fd) {
	struct file_descriptor_table * table;
	struct file_descriptor_entry * entry;

	table = get_file_descriptor_table(pid);
	if (table == NULL || fd >= table->total_descriptors) {
		return NULL;
	}

	entry = &table->entries[fd];

	// If file descriptor not yet assigned (properly allocated for an open file), we return NULL
	if (entry->fd == FD_NOT_USED) {
		return NULL;
	}

	return entry;
}

void delete_file_descriptor_entry(int pid, int fd) {
	struct file_descriptor_table * table;
	struct file_descriptor_entry * entry;

	table = get_file_descriptor_table(pid);
	if (table == NULL || fd >= table->total_descriptors || fd <= 2) {
		return;
	}

	entry = &table->entries[fd];

	// We just set the fd to be unused (-1). It would be cool to realloc the entries if we notice that 
	// all the allocated fd entries are together and below the max allocated entry to save memory.
	entry->fd = FD_NOT_USED;
	table->used_descriptors--;
}
