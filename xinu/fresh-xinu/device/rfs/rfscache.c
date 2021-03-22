/* rfscache - Collection of functions needed for the remote file system cache */
#include <xinu.h>

/* byte_to_index - convertes a desired byte number to the correct index/offset to look for in the cache */
uint32 byte_to_index(
    uint32 byte,
    struct rfs_cpos *result) 
{
    if (byte < 0) {
        return SYSERR;
    }
    result->index = byte / RFS_CBLOCK_SIZE;
    result->offset = byte % RFS_CBLOCK_SIZE;
    return OK;
}

void dump_cache_blocks(struct rflcblk * rfptr) {
    kprintf("Dumping cache\n");
    for (uint32 i = 0; i < MAX_RFS_CBLOCKS; i++) {
        kprintf("Block at index %d has %d\n", i, (uint32) rfptr->cache[i]);
    }
    kprintf("Done dumping cache\n");
}

/* rfs_cache_fetch - Copies over as many bytes as possible from cache into buffer */
uint32 rfs_cache_fetch(
    struct rflcblk *rfptr,      /* pointer to remote file's control block */
    uint32 position,            /* position of requested data */
    char *buff,                 /* user's buffer */
    int32 count)                /* count of desired bytes read */ 
{
    struct rfs_cpos cache_loc; /* location of target bytes in cache */
    uint32 status;              /* used to verify status of various helper calls */
    uint32 bytes_found;         /* number of bytes of file data found */
    struct rfs_cblock *cur_block; /* current block we are examining */
    char *from, *to;            /* Used for copying from cache to user buffer */

    /* check that position is legal */
    if (position < 0 || position > rfptr->rfsize) {
        return SYSERR;
    }

    /* find correct location in cache */
    status = byte_to_index(position, &cache_loc);
    if (status == SYSERR) {
        return SYSERR;
    }

    // dump_cache_blocks(rfptr);

    #if RFS_CACHE_DEBUG
    kprintf("RFS_CACHE_FETCH: looking in index %d, offset %d\n", cache_loc.index, cache_loc.offset);
    #endif
    /* if we reach here, we know all requested bytes lie in same cache block */
    if (cache_loc.index < MAX_RFS_CBLOCKS) {
        /* block resides in array structure, directly index in */
        if (rfptr->cache[cache_loc.index] == NULL || rfptr->cache[cache_loc.index]->valid_bytes == 0) {
            /* desired block not in cache */
            /* we'll make a network request to bring this block into cache, then try fetching again, so this fetch fails */
            #if RFS_CACHE_DEBUG
            kprintf("RFS_CACHE_FETCH: desired block %d not in this remote file's cache\n", cache_loc.index);
            #endif

            return 0;
        }
        else {
            /* retrieve pointer to cache block */
            cur_block = rfptr->cache[cache_loc.index];

            /* copy until no longer in valid byte range or exceed count */
            bytes_found = 0;
            to = buff;
            from = (char *) ((uint32) cur_block->block_data + cache_loc.offset);

            #if RFS_CACHE_DEBUG
            kprintf("RFS_CACHE_FETCH: Copying ");
            #endif

            while (cache_loc.offset < cur_block->valid_bytes && bytes_found < count) {
                #if RFS_CACHE_DEBUG
                kprintf("%c", *from);
                #endif

                *to++ = *from++;
                cache_loc.offset++;
                bytes_found++;
            }
            #if RFS_CACHE_DEBUG
            kprintf("\n");
            #endif

            return bytes_found;
        }
    }
    else {
        #if RFS_CACHE_DEBUG
        kprintf("RFS_CACHE_FETCH: Looking in linked list!\n");
        #endif
        /* have to look in linked list, search for desired block */
        uint32 target_file_start = cache_loc.index * RF_DATALEN;
        cur_block = rfptr->cache_list;

        /* search through sorted list until find block with correct file start */
        while (cur_block->file_start < target_file_start && cur_block != NULL) {
            cur_block = cur_block->next;
        }

        if (cur_block == NULL || cur_block->file_start != target_file_start) {
            /* block not found, we'll try making a network request then try again */
            #if RFS_CACHE_DEBUG
            kprintf("RFS_CACHE_FETCH: desired block %d not in this remote file's cache\n", cache_loc.index);
            #endif
            return 0;
        }

        /* found the block we were looking for */
        /* copy until no longer in valid byte range or exceed count */
        bytes_found = 0;
        to = buff;
        from = (char *) ((uint32) cur_block->block_data + cache_loc.offset);

        #if RFS_CACHE_DEBUG
        kprintf("RFS_CACHE_FETCH: Copying ");
        #endif

        while (cache_loc.offset < cur_block->valid_bytes && bytes_found < count) {
            #if RFS_CACHE_DEBUG
            kprintf("%c", *from);
            #endif

            *to++ = *from++;
            cache_loc.offset++;
            bytes_found++;
        }
        #if RFS_CACHE_DEBUG
        kprintf("\n");
        #endif

        return bytes_found;
    }

    return bytes_found;
}

/* print_cache_list - debugging function to print current cache list for a specified remote file */
void print_cache_list(
    struct rflcblk *rfptr) 
{
    struct rfs_cblock *cur = rfptr->cache_list;
    uint32 num = 0;
    kprintf("Printing cache_list: ");
    while (cur != NULL) {
        kprintf("{Node %d: file_start = %d, valid_bytes = %d} ->", num, cur->file_start, cur->valid_bytes);
        cur = cur->next;
        num++;
    }
    kprintf("\n");
    return;
}

/* rfs_cache_store - function to place contents of specified read request into a cache block for specified remote file */
uint32 rfs_cache_store(
    struct rflcblk *rfptr,
    struct rf_msg_rres * resp)
{
    struct rfs_cpos cache_loc;     /* position in cache to place this block */
    uint32 status;                  /* used for checking status of function calls */
    uint32 i;                       /* used for indexing into block */

    status = byte_to_index(ntohl(resp->rf_pos), &cache_loc);
    if (status == SYSERR) {
        return SYSERR;
    }

    /* offset should always be zero since we always copy from start of block */
    if (cache_loc.offset != 0) {
        return SYSERR;
    }

    #if RFS_CACHE_DEBUG
    kprintf("RFS_CACHE_STORE: storing response in index %d, offset %d\n", cache_loc.index, cache_loc.offset);
    #endif
    if (cache_loc.index < MAX_RFS_CBLOCKS) {
        /* block should be placed in the array structure */
        struct rfs_cblock *new_block = NULL;

        if (rfptr->cache[cache_loc.index] == NULL) {
            /* no memory allocated for block */
            /* TODO: switch to using memory buffer instead of getmem so there is a limit on how much memory cache blocks can use */
            new_block = (struct rfs_cblock *) getmem(sizeof(struct rfs_cblock));
            if (new_block == NULL) {
                kprintf("Error allocating block!\n");
                return SYSERR;
            }
        }
        else {
            /* there is already an allocated block, we will just overwrite */
            new_block = rfptr->cache[cache_loc.index];
        }

        #if RFS_CACHE_DEBUG
        kprintf("RFS_CACHE_STORE: copying the following to block %d: ", cache_loc.index);
        #endif

        /* copy contents into block */
        for (i = 0; i < RF_DATALEN && i < ntohl(resp->rf_len); i++) {
            new_block->block_data[i] = resp->rf_data[i];
            #if RFS_CACHE_DEBUG
            kprintf("%c", new_block->block_data[i]);
            #endif
        }
        #if RFS_CACHE_DEBUG
        kprintf("\n");
        #endif

        /* set up block metadata */
        new_block->file_start = ntohl(resp->rf_pos);
        new_block->is_dirty = 0;

        /* we copy min(RF_DATALEN, resp->rf_len) bytes into block */
        new_block->valid_bytes = ntohl(resp->rf_len) <= RF_DATALEN ? ntohl(resp->rf_len) : RF_DATALEN;

        /* setup block's devnum to associate it with this remote file */
        new_block->rfl_devnum = rfptr->rfdev;

        /* TODO: setup block's lru list */
        new_block->lru_prev = NULL;
        new_block->lru_next = NULL;

        /* place block into array structure */
        rfptr->cache[cache_loc.index] = new_block;

        #if RFS_CACHE_DEBUG
        kprintf("RFS_CACHE_STORE: New block at address %d, cache contains address %d\n", (uint32) new_block, (uint32) rfptr->cache[cache_loc.index]);
        #endif

        return OK;
    }
    else {
        /* have to add block to linked list of blocks */
        #if RFS_CACHE_DEBUG
        kprintf("RFS_CACHE_STORE: Adding to linked list!\n");
        #endif
        /* TODO: switch to using memory buffer and evicting blocks if necessary */

        /* allocate a new block */
        struct rfs_cblock *new_block = NULL;
        new_block = (struct rfs_cblock *) getmem(sizeof(struct rfs_cblock));
        if (new_block == NULL) {
            kprintf("Error allocating block!\n");
            return SYSERR;
        }

        /* copy contents into block */
        for (i = 0; i < RF_DATALEN && i < ntohl(resp->rf_len); i++) {
            new_block->block_data[i] = resp->rf_data[i];
            #if RFS_CACHE_DEBUG
            kprintf("%c", new_block->block_data[i]);
            #endif
        }
        #if RFS_CACHE_DEBUG
        kprintf("\n");
        #endif

        /* set up block metadata */
        new_block->file_start = ntohl(resp->rf_pos);
        new_block->is_dirty = 0;

        /* we copy min(RF_DATALEN, resp->rf_len) bytes into block */
        new_block->valid_bytes = ntohl(resp->rf_len) <= RF_DATALEN ? ntohl(resp->rf_len) : RF_DATALEN;

        /* setup block's devnum to associate it with this remote file */
        new_block->rfl_devnum = rfptr->rfdev;

        /* TODO: setup block's lru list */
        new_block->lru_prev = NULL;
        new_block->lru_next = NULL;

        new_block->next = NULL;
        new_block->prev = NULL;

        /* place node into list */
        if (rfptr->cache_list == NULL) {
            /* list is empty */
            rfptr->cache_list = new_block;

            // #if RFS_CACHE_DEBUG
            print_cache_list(rfptr);
            // #endif
            return OK;
        }

        /* place node into list in sorted order */
        struct rfs_cblock *cur = rfptr->cache_list;
        while (cur->file_start < new_block->file_start && cur->next != NULL) {
            cur = cur->next;
        }

        if (cur->file_start > new_block->file_start) {
            /* place before */
            if (cur == rfptr->cache_list) {
                /* new_node is head of list now */
                cur->prev = new_block;
                new_block->next = cur;
                rfptr->cache_list = new_block;

                // #if RFS_CACHE_DEBUG
                print_cache_list(rfptr);
                // #endif
                return OK;
            }

            /* new_node is not head of list now, update both sides */
            struct rfs_cblock *prev = cur->prev;
            prev->next = new_block;
            cur->prev = new_block;
            new_block->prev = prev;
            new_block->next = cur;

            #if RFS_CACHE_DEBUG
            print_cache_list(rfptr);
            #endif
            return OK;
        }

        /* new_node is tail of list now */
        cur->next = new_block;
        new_block->prev = cur;

        // #if RFS_CACHE_DEBUG
        print_cache_list(rfptr);
        // #endif
        return OK;
    }

    return OK;
}