#ifndef _SYSCALLS_1_
#define _SYSCALLS_1_

#include "common.h"

// The methods below do not use user file descriptor tables

int syscalls1__mkdir(const char *path, mode_t mode);
int syscalls1__mknod(const char *path, mode_t mode, dev_t dev);

int syscalls1__readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset);
int syscalls1__unlink(const char *path);
int syscalls1__rmdir(const char *path);
int syscalls1__lstat(const char *path, struct stat *buf);

#endif
