#include "common.h"
#include "disk_emulator.h"
#include "syscalls1.h"
#include "syscalls2.h"

static int fstr_getattr(const char *path, struct stat *stbuf)
{
    LOGD("called fstr_getattr");
    if(syscalls1__lstat(path, stbuf) == -1)
        return -errno;
    return 0;
}

static int fstr_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){
	LOGD("you called syscalls1__readdir\n");
    int ret_val = 0;
    ret_val = syscalls1__readdir(path, buf, filler, offset);
    
	return ret_val;
}

static int fstr_open(const char *path, struct fuse_file_info *fi){
    int ret_val = 0;
    int fd;
     
    LOGD("\nfstr_open(path\"%s\",..)\n", path);
    
    fd = syscalls2__open(path, fi->flags);
    if (fd < 0){
    	LOGD("OPEN error, fd: %d", fd);
    }
    fi->fh = fd;
    
    return ret_val;
}

static int fstr_mkdir(const char *path, mode_t mode){
    int ret_val = 0;
    
    LOGD("\nfstr_mkdir(path=\"%s\", ..)\n", path);
    
    ret_val = syscalls1__mkdir(path, mode);
    if (ret_val < 0){
    	LOGD("MKDIR error: %d", ret_val);
    }
    return ret_val;
}

static int fstr_rmdir(const char *path){
    int ret_val = 0;
    
    LOGD("\nfstr_rmdir(path=\"%s\", ..)\n", path);
    
    ret_val = syscalls1__rmdir(path);
    if (ret_val < 0){
        LOGD("RMDIR error: %d", ret_val);
    }
    return ret_val;
}


static int fstr_mknod(const char *path, mode_t mode, dev_t dev){ // dev_t is device type; I suppose we wouldn't need it
	int ret_val = 0;
    
    LOGD("\nfstr_mknod(path=\"%s\", .., ..)\n", path);
    
    ret_val = syscalls1__mknod(path, mode, dev);

    if(ret_val < 0){
    	
    	LOGD("MKNOD error: %d", ret_val);	
    }
    return ret_val;
}

static int fstr_unlink(const char *path)
{
    int ret_val = 0;

    LOGD("fstr_unlink(path=\"%s\")\n", path);
    
    ret_val = syscalls1__unlink(path);
    if (ret_val < 0)
		LOGD("UNLINK error: %d", ret_val);
    
    return ret_val;
}

static int fstr_release(const char *path, struct fuse_file_info *fi){
    int ret_val = 0;
    
    LOGD("\nfstr_release(path=\"%s\", ..)\n", path);

    ret_val = syscalls2__close(fi->fh); // In our implementation, this is syscalls1__unlink
    
    return ret_val;
}

static int fstr_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
    int ret_val = 0;
    
    LOGD("\nfstr_read(path=\"%s\", .., .., .., ..)\n", path);
    
    ret_val = syscalls2__pread(fi->fh, buf, size, offset);
    if (ret_val < 0)
		LOGD("READ error: %d", ret_val);
    
    return ret_val;
}

static int fstr_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
    int ret_val = 0;
    
    LOGD("\nfstr_write(path=\"%s\", .., .., .., ..)\n", path);
    
    ret_val = syscalls2__pwrite(fi->fh, buf, size, offset);
    if (ret_val < 0)
		LOGD("WRITE error: %d", ret_val);
    
    return ret_val;
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
};

int main(int argc, char *argv[]) {
	init_disk_emulator();
    init_superblock();
	return fuse_main(argc, argv, &fstr_fuse_oper, NULL);
}
