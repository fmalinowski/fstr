#ifndef MEMORY_STRUCTURES
#define MEMORY_STRUCTURES

/*
	In this module we have some helper functions that enables other modules to get 
	the current superblock that is in memory, the inodes that are in memory, ...
	This will have to be implemented but the prorotype of the functions are here.

	There will be probably a pointer to the current superblock in memory in the .c file
	We might also have linkedlist or array of the inodes currently in memory etc.
	But this implementation will be in the .c file and we don't care about it in other modules
*/

struct superblock * get_superblock(void);

void mark_superblock_modified(void);  // This call mark the superblock as modified. Called for instance by data_block_alloc when allocation of datablock

#endif
