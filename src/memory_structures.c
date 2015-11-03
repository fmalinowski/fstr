#include <stdio.h>
#include <stdlib.h>

#include "memory_structures.h"
#include "disk_emulator.h"
#include "blocks_handler.h"

// THIS MODULE HAS TO BE IMPLEMENTED CORRECTLY AND TESTED
// BASIC CODE FOR NOW to make the module blocks_handler compile


static struct superblock * _superblock;

// THIS HAS TO BE IMPLEMENTED CORRECTLY AND TESTED
// BASIC CODE FOR NOW to make the module blocks_handler compile
struct superblock * get_superblock(void) {
	if (!_superblock) {
		_superblock = (struct superblock *) malloc(sizeof(struct superblock));
	}
	return _superblock;
}

void mark_superblock_modified(void) {

}
