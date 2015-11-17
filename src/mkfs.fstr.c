#include "common.h"
#include "mkfs.h"

int main(int argc, char const *argv[]) {

    if(argc != 1) {
        printf("specify mount point\n");
        return -1;
    }

    const char *path = argv[0];
    (void) path;    // TODO

    return create_fs();
}
