#include "common.h"
#include "mkfs.h"
#include "disk_emulator.h"
#include "syscalls1.h"
#include "syscalls2.h"

static int fstr_getattr(const char *path, struct stat *stbuf) {
    LOGD("fstr_getattr");
    if(lstat(path, stbuf) == -1) {
        return -errno;
    }
    return 0;
}

static int fstr_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){
	LOGD("fstr_readdir");
    // Not doing anything
    // if(readdir(path, buf, filler, offset) == -1) {
    //     return -errno;
    // }
	return 0;
}

static int fstr_open(const char *path, struct fuse_file_info *fi) {
    LOGD("fstr_open(path: \"%s\")", path);
    int ret_val = open(path, fi->flags);
    if (ret_val == -1) {
    	return -errno;
    }
    fi->fh = ret_val;
    return 0;
}

static int fstr_mkdir(const char *path, mode_t mode) {
    LOGD("fstr_mkdir(path: \"%s\")", path);
    if (mkdir(path, mode) == -1) {
    	return -errno;
    }
    return 0;
}

static int fstr_rmdir(const char *path) {
    LOGD("fstr_rmdir(path: \"%s\")", path);
    if (rmdir(path) == -1){
        return -errno;
    }
    return 0;
}

static int fstr_mknod(const char *path, mode_t mode, dev_t dev) {
	LOGD("fstr_mknod(path: \"%s\")", path);
    if(mknod(path, mode, dev) == -1) {
    	return -errno;
    }
    return 0;
}

static int fstr_unlink(const char *path) {
    LOGD("fstr_unlink(path: \"%s\")", path);
    if(unlink(path) == -1) {
        return -errno;
    }
    return 0;
}

static int fstr_release(const char *path, struct fuse_file_info *fi) {
    LOGD("fstr_release(path: \"%s\")", path);
    if(close(fi->fh) == -1) {
        return -errno;
    }
    return 0;
}

static int fstr_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    LOGD("fstr_read(path: \"%s\")", path);
    int bytes = pread(fi->fh, buf, size, offset);
    if (bytes == -1) {
        return -errno;
    }
    return bytes;
}

static int fstr_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    LOGD("fstr_write(path: \"%s\")", path);
    int bytes = pwrite(fi->fh, buf, size, offset);
    if (bytes == -1) {
		return -errno;
    }
    return bytes;
}


static int fstr_utimens(const char *path, const struct timespec tv[2]) {
    LOGD("fstr_utimens");
    // Not doing anything
    // if(utimens(path, tv) == -1) {
    //     return -errno;
    // }
    return 0;
}

static int fstr_chmod(const char *path, mode_t mode) {
    LOGD("fstr_chmod");
    if(chmod(path, mode) == -1) {
        return -errno;
    }
    return 0;
}

static int fstr_chown(const char *path, uid_t uid, gid_t gid) {
    LOGD("fstr_chown");
    if(chown(path, uid, gid) == -1) {
        return -errno;
    }
    return 0;
}

static int fstr_rename(const char *oldpath, const char *newpath) {
    LOGD("fstr_rename");
    if(rename(oldpath, newpath) == -1) {
        return -errno;
    }
    return 0;
}

static struct fuse_operations fstr_fuse_oper = {
	.getattr	= fstr_getattr,
	.mkdir		= fstr_mkdir,
    .rmdir      = fstr_rmdir,
	.mknod 		= fstr_mknod,
	.unlink		= fstr_unlink,
	.readdir	= fstr_readdir,
	.open		= fstr_open,
	.release	= fstr_release,
	.read		= fstr_read,
	.write		= fstr_write,
    .utimens    = fstr_utimens,
    .chmod      = fstr_chmod,
    .chown      = fstr_chown,
    .rename     = fstr_rename
};

int main(int argc, char *argv[]) {

    // if(init_disk_emulator() == -1) {
    //     fprintf(stderr, "Failed to init disk emulator\n");
    //     return -1;
    // }

    // if (create_fs() == -1) {
    //     fprintf(stderr, "Failed to create fs\n");
    //     return -1;
    // }
    
    // if(init_superblock() == -1) {
    //     fprintf(stderr, "Failed to init superblock\n");
    //     return -1;
    // }
    
	return fuse_main(argc, argv, &fstr_fuse_oper, NULL);
}
