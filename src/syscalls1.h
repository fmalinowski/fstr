#ifndef _SYSCALLS_1_
#define _SYSCALLS_1_

#include "common.h"

// The methods below do not use user file descriptor tables

int mkdir(const char *path, mode_t mode);
int mknod(const char *path, mode_t mode, dev_t dev);

int readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset);
int unlink(const char *path);

#endif
