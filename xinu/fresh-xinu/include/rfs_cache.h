/* rfs_cache.h - Contains constants and data structure definitions relevant to caching for the remote file system */

/* Constants */

#ifndef MAX_BLOCKS
#define MAX_BLOCKS 100 /* maximum number of cached blocks a remote file can access through direct indexing */
#endif

#ifndef RFL_BLOCK_SIZE
#define RFL_BLOCK_SIZE 1024 /* maximum number of file data bytes a cache block can contain */
#endif

/* struct rfl_block - The data structure for a block of cached data */
struct rfl_block {
    uint32 file_start; /* the beginning file position of this chunk, it will hold bytes [file_start, file_start + RFL_CHUNK_SIZE-1] */
    bool8 is_dirty;      /* True if the chunk has been written to */
    uint32 valid_bytes; /* number of bytes in the chunk that correspond to meaningful file data */
    char block_data[RFL_BLOCK_SIZE];  /* file data contained in the chunk */
};

/* struct rfs_cache_list_node - Node for linked list of cache blocks
 * Use case 1: for large files that require more than MAX_BLOCKS blocks to cache)
 * Use case 2: to track least recently used block, for eviction purposes when cache gets full
 */
struct rfs_cache_list_node {
    struct rfl_block * block; /* The cache block this list node points to */
    struct rfs_cache_list_node * next; /* The next node in the linked list */
};

/* struct cache_pos - Used to pinpoint a location in a cache block by specifying the
 * cache block index and the offset within that block
 */
struct cache_pos {
    uint32 index;
    uint32 offset;
};