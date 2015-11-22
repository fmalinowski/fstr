#include "common.h"
#include "mkfs.h"
#include "disk_emulator.h"

int main(int argc, char const *argv[]) {
	// Remove existing file
	remove(DISK_STORE_PATH);

	// Prepare dd cmd
	char cmd[1024];
	big_int fs_size = FS_SIZE;
	sprintf(cmd, "dd if=/dev/zero of=%s count=1 bs=1 seek=%" PRIu64, DISK_STORE_PATH, fs_size);
	
	// Create the file
	if(system(cmd) == 0) {
		init_disk_emulator();
	    create_fs();
	    free_disk_emulator();
	    return 0;
	}
    return -1;
}
