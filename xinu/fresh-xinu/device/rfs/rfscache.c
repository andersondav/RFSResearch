// /* rfscache - Collection of functions needed for the remote file system cache */
// #include <xinu.h>

// /* byte_to_index - convertes a desired byte number to the correct index to look for in the cache */
// void byte_to_index(
//     uint32 byte,
//     struct cache_pos *result) 
// {
//     if (byte < 0) {
//         return SYSERR;
//     }
//     result->index = byte / RFL_BLOCK_SIZE;
//     result->offset = byte % RFL_BLOCK_SIZE;
//     return OK;
// }

// /* rfs_cache_fetch - Copies over as many bytes as possible from cache into buffer */
// uint32 rfs_cache_fetch(
//     struct rflcblk *rfptr,     /* pointer to remote file's control block */
//     char *buff,                 /* user's buffer */
//     int32 count)                /* count of desired bytes read */ 
// {
//     struct cache_pos cache_loc; /* location of target bytes in cache */
//     uint32 status;              /* used to verify status of various helper calls */
//     uint32 bytes_found;         /* number of bytes of file data found */
//     struct rfl_block *cur_block;/* current block we are examining */
//     char *from, *to;            /* Used for copying from cache to user buffer */

//     /* find correct location in cache */
//     status = byte_to_index(rfptr->rfpos, &cache_loc);
//     if (status == SYSERR) {
//         /* invalid byte position */
//         return 0;
//     }

//     /* check if desired bytes lie in multiple cache blocks */
//     if (cache_loc->offset + count > RFL_BLOCK_SIZE) {
//         /* user needs bytes from multiple cache blocks */

//         /* fetch all bytes from first block */
//         bytes_found = rfs_cache_fetch(rfptr, buff, RFL_BLOCK_SIZE - cache_loc->offset);
//         if (bytes_found == RFL_BLOCK_SIZE - cache_loc->offset) {
//             /* found all bytes from first block, can proceed to look for remaining bytes to fill rest of buff */
//             bytes_found += rfs_cache_fetch(rfptr, buff + bytes_found, count - bytes_found); /* should really try to avoid this recursion */
//             return bytes_found;
//         }
//         else {
//             /* fetch from first block failed for some reason, either because ran into EOF or that block isn't in cache */
//             /* (should try to truncate read to file length so we know that we get here b/c block isn't in cache) */

//             /* we'll fetch all required blocks, then try fetching again, so this fetch fails */
//             return 0;
//         }
//     }

//     /* if we reach here, we know all requested bytes lie in same cache block */
//     if (cache_loc->index < MAX_BLOCKS) {
//         /* block resides in array structure, directly index in */
//         if (rfptr->rfl_cache[cache_loc.index] == NULL) {
//             /* desired block not in cache */
//             /* we'll fetch the block later, then try fetching again, so this fetch fails */
//             return 0;
//         }
//         else {
//             /* retrieve pointer to cache block */
//             cur_block = rfptr->rfl_cache[cache_loc.index];

//             /* copy until no longer in valid byte range */
//             bytes_found = 0;
//             to = buff;
//             from = cur_block->block_data[cache_loc.offset];
//             while (cache_loc.offset < cur_block->valid_bytes) {
//                 *to++ = *from++;
//                 cache_loc.offset++;
//                 bytes_found++;
//             }

//             return bytes_found;
//         }
//     }
//     else {
//         /* have to look in linked list, search for desired block */
//     }

//     return bytes_found;
// }