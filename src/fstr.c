#include "common.h"
#include "disk_emulator.h"
#include "mkfs.h"

static struct fuse_operations fstr_fuse_oper = {
};

int main(int argc, char *argv[]) {
	init_disk_emulator();
	create_fs();
	return fuse_main(argc, argv, &fstr_fuse_oper, NULL);
}