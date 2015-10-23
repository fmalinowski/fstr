#define FUSE_USE_VERSION 26

#include <fuse.h>

static struct fuse_operations fstr_oper = {
};

int main(int argc, char *argv[]) {
	return fuse_main(argc, argv, &fstr_oper, NULL);
}