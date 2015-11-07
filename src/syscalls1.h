#ifndef _SYSCALLS_1_
#define _SYSCALLS_1_

// The methods below do not use user file descriptor tables

int mkdir(const char *path, mode_t mode);
int mknod(const char *path, mode_t mode, dev_t dev);

int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result);
int unlink(const char *path);

#endif