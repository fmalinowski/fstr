#include "common.h"
#include "disk_emulator.h"
#include "mkfs.h"

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";

static int fstr_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;

    memset(stbuf, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else if (strcmp(path, hello_path) == 0) {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(hello_str);
    } else
        res = -ENOENT;

    return res;
}

static int fstr_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){
	int ret_val = 0;
    
    filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	printf("you called readdir\n");
	return ret_val;
}

static int fstr_open(const char *path, struct fuse_file_info *fi){
    int ret_val = 0;
    int fd;
     
    printf("\nfstr_open(path\"%s\",..)\n", path);
    
    fd = open(path, fi->flags);
    if (fd < 0){
    	printf("file open error");
    	LOGD("OPEN error, fd: %d, fd");
    }
    fi->fh = fd;
    
    return ret_val;
}

static int fstr_mkdir(const char *path, mode_t mode){
    int ret_val = 0;
    
    printf("\nfstr_mkdir(path=\"%s\", ..)\n", path);
    
    ret_val = mkdir(path, mode);
    if (ret_val < 0){
    	printf("mkdir error");
    	LOGD("MKDIR error: %d", ret_val);
    }
    return ret_val;
}

static int fstr_mknod(const char *path, mode_t mode, dev_t dev){ // dev_t is device type; I suppose we wouldn't need it
	int ret_val = 0;
    
    printf("\nfstr_mknod(path=\"%s\", .., ..)\n", path);
    
    ret_val = mknod(path, mode, dev);

    if(ret_val < 0){
    	printf("mknod error");
    	LOGD("MKNOD error: %d", ret_val);	
    }
    return ret_val;
}

static int fstr_unlink(const char *path)
{
    int ret_val = 0;

    printf("fstr_unlink(path=\"%s\")\n", path);
    
    ret_val = unlink(path);
    if (ret_val < 0)
		printf("UNLINK error: %d", ret_val);
    
    return ret_val;
}

static int fstr_release(const char *path, struct fuse_file_info *fi){
    int ret_val = 0;
    
    printf("\nfstr_release(path=\"%s\", ..)\n", path);

    ret_val = close(fi->fh); // In our implementation, this is unlink
    
    return ret_val;
}

static int fstr_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
    int ret_val = 0;
    
    printf("\nfstr_read(path=\"%s\", .., .., .., ..)\n", path);
    
    ret_val = pread(fi->fh, buf, size, offset);
    if (ret_val < 0)
		printf("READ error: %d", ret_val);
    
    return ret_val;
}

static int fstr_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
    int ret_val = 0;
    
    printf("\nfstr_write(path=\"%s\", .., .., .., ..)\n", path);
    
    ret_val = pwrite(fi->fh, buf, size, offset);
    if (ret_val < 0)
		printf("WRITE error: %d", ret_val);
    
    return ret_val;
}


static struct fuse_operations fstr_fuse_oper = {
	.getattr	= fstr_getattr,
	.mkdir		= fstr_mkdir,
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
	create_fs();
	return fuse_main(argc, argv, &fstr_fuse_oper, NULL);
}