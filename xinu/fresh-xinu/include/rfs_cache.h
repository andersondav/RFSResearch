/* rfs_cache.h - Contains constants and data structure definitions relevant to caching for the remote file system */
#define RFS_CACHE_DEBUG 0
/* Constants */

#ifndef MAX_RFS_CBLOCKS
#define MAX_RFS_CBLOCKS 10 /* maximum number of cached blocks a remote file can access through direct indexing */
#endif

#ifndef RFS_CBLOCK_SIZE
#define RFS_CBLOCK_SIZE 1024 /* maximum number of file data bytes a cache block can contain */
#endif

/* struct rfs_cblock - The data structure for a block of cached remote file data */
struct rfs_cblock {
    uint32 file_start; /* the beginning file position of this chunk, it will hold bytes [file_start, file_start + RFL_CHUNK_SIZE-1] */
    bool8 is_dirty;      /* True if the chunk has been written to */
    uint32 valid_bytes; /* number of bytes in the chunk that correspond to meaningful file data */
    char block_data[RFS_CBLOCK_SIZE];  /* file data contained in the chunk */

    uint32 rfl_devnum;         /* device number for the rfl device this block belongs to */
    struct rfs_cblock * lru_next;    /* The next node in the lru list */
    struct rfs_cblock * lru_prev;    /* The prev node in the lru list */

    /* the following fields are used when the block is in the linked list portion of the remote file's cache */
    struct rfs_cblock * next; /* The next node in the linked list */
    struct rfs_cblock * prev; /* The prev node in the linked list */
};

/* struct rfs_cache_list_node - Node for linked list of cache blocks
 * Use case 1: for large files that require more than MAX_BLOCKS blocks to cache)
 * Use case 2: to track least recently used block, for eviction purposes when cache gets full
 */
// struct rfl_cache_list_node {
//     struct rfl_block * block; /* The cache block this list node points to */
//     struct rfl_cache_list_node * next; /* The next node in the linked list */
//     struct rfl_cache_list_node * prev; /* The prev node in the linked list */
// };

/* struct rfs_cpos - Used to pinpoint a location in a cache block by specifying the
 * cache block index and the offset within that block
 */
struct rfs_cpos {
    uint32 index;
    uint32 offset;
};