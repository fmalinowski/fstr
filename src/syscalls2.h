#ifndef _SYSCALLS_2_
#define _SYSCALLS_2_

// The methods below use user file descriptor tables

ssize_t pread(int fildes, void *buf, size_t nbyte, off_t offset); 
ssize_t pwrite(int fildes, const void *buf, size_t nbyte, off_t offset);

int open(const char *path, int oflag, ... );
int close(int fildes);

#endif