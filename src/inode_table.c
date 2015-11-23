#include "common.h"
#include "inode_table.h"
#include "inodes_handler.h"
#include "uthash.h"

#define MAX_CACHE_SIZE 1024

struct inode_entry *cache = NULL;

int get_inode(int inode_id, struct inode *inode) {
    if(get_inode_from_cache(inode_id, inode) == -1) {
        if(iget(inode_id, inode) == 0) {
            put_inode_in_cache(inode);
            return 0;
        }
        return -1;
    }
	return 0;
}

int get_inode_from_cache(int inode_id, struct inode *inode) {
    struct inode_entry *inode_entry;

    HASH_FIND_INT(cache, &inode_id, inode_entry);
    if (inode_entry) {
        LOGD("Cache hit for inode %d", inode_id);
        // Found in cache. Move it to front.
        HASH_DEL(cache, inode_entry);
        HASH_ADD_INT(cache, inode_id, inode_entry);

        memcpy(inode, inode_entry->inode, sizeof(struct inode));
        return 0;
    }
    LOGD("Cache miss for inode %d", inode_id);
    return -1;
}

int put_inode(struct inode *inode) {
    if(iput(inode) == 0) {
       put_inode_in_cache(inode);
       return 0;
    }
	return -1;
}

int put_inode_in_cache(struct inode *inode) {
    if(inode == NULL) {
        return -1;
    }

    struct inode_entry *inode_entry, *temp_inode_entry;

    inode_entry = (struct inode_entry*) malloc(sizeof(struct inode_entry));
    inode_entry->inode_id = inode->inode_id;
    inode_entry->inode = (struct inode*) malloc(sizeof(struct inode));;
    memcpy(inode_entry->inode, inode, sizeof(struct inode));

    LOGD("Adding to cache inode %d", inode->inode_id);
    HASH_REPLACE_INT(cache, inode->inode_id, inode_entry, temp_inode_entry);
    free_inode_entry(temp_inode_entry);
    temp_inode_entry = NULL;
    
    // prune the cache to MAX_CACHE_SIZE
    if (HASH_COUNT(cache) >= MAX_CACHE_SIZE) {
        HASH_ITER(hh, cache, inode_entry, temp_inode_entry) {
            // prune the first entry (loop is based on insertion order so this deletes the oldest item)
            LOGD("Removing from cache inode %d", inode_entry->inode_id);
            HASH_DEL(cache, inode_entry);
            free_inode_entry(inode_entry);
            break;
        }
    }
    return 0;
}

void purge_inode_table(void) {
    struct inode_entry *inode_entry, *tmp;

    HASH_ITER(hh, cache, inode_entry, tmp) {
        HASH_DEL(cache, inode_entry);
        free_inode_entry(inode_entry);
    }
}

void free_inode_entry(struct inode_entry *inode_entry) {
    if(inode_entry != NULL) {
        free(inode_entry->inode);
        free(inode_entry);
    }
}
