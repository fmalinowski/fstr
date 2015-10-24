#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdlib.h>
#include <stdio.h>
#include "disk_emulator.h"

static struct fuse_operations fstr_fuse_oper = {
};

int main(int argc, char *argv[]) {
	return fuse_main(argc, argv, &fstr_fuse_oper, NULL);
}