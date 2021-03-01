/* rfscache - Collection of functions needed for the remote file system cache */
#include <xinu.h>

/* byte_to_index - convertes a desired byte number to the correct index to look for in the cache */
void byte_to_index(
    uint32 byte,
    struct cache_pos *result) 
{
    if (byte < 0) {
        return SYSERR;
    }
    // result->index = byte / RFL_BLOCK_SIZE;
    // result->offset = byte % RFL_BLOCK_SIZE;
    result->index = byte >> 10;
    result->offset = byte & RF_DATALEN - 1;
    return OK;
}

/* rfs_cache_fetch - Copies over as many bytes as possible from cache into buffer */
uint32 rfs_cache_fetch(
    struct rflcblk *rfptr,      /* pointer to remote file's control block */
    uint32 position,            /* position of requested data */
    char *buff,                 /* user's buffer */
    int32 count)                /* count of desired bytes read */ 
{
    struct cache_pos cache_loc; /* location of target bytes in cache */
    uint32 status;              /* used to verify status of various helper calls */
    uint32 bytes_found;         /* number of bytes of file data found */
    struct rfl_block *cur_block;/* current block we are examining */
    char *from, *to;            /* Used for copying from cache to user buffer */

    /* check that position is legal */
    if (position < 0 || position > rfptr->rfsize) {
        return SYSERR;
    }

    /* find correct location in cache */
    byte_to_index(rfptr->rfpos, &cache_loc);

    /* if we reach here, we know all requested bytes lie in same cache block */
    if (cache_loc->index < MAX_BLOCKS) {
        /* block resides in array structure, directly index in */
        if (rfptr->rfl_cache[cache_loc.index] == NULL || rfptr->rfl_cache[cache_loc.index]->valid_bytes == 0) {
            /* desired block not in cache */
            /* we'll make a network request to bring this block into cache, then try fetching again, so this fetch fails */
            return 0;
        }
        else {
            /* retrieve pointer to cache block */
            cur_block = rfptr->rfl_cache[cache_loc.index];

            /* copy until no longer in valid byte range or exceed count */
            bytes_found = 0;
            to = buff;
            from = cur_block->block_data[cache_loc.offset];
            while (cache_loc.offset < cur_block->valid_bytes && bytes_found < count) {
                *to++ = *from++;
                cache_loc.offset++;
                bytes_found++;
            }

            return bytes_found;
        }
    }
    else {
        /* have to look in linked list, search for desired block */
    }

    return bytes_found;
}