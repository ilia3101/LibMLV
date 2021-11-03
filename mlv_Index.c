#include "libmlv.h"

/* How much data one index entry holds */
#define ENTRY_BYTES 38

/* Blocks below or equal to this size will be fully stored in the index, so the
 * file will not have to be accessed to get their data, as it will already be
 * in the index. There is a 69% memory overhead to doing this!!!
 * The chosen value allows blocks to occupy up to 10 entries, as you can see! */
#define MAX_BLOCK_SIZE_TO_FULLY_STORE_IN_INDEX (ENTRY_BYTES*10)

/* How many entries to allocate memory for (so that realloc wont have to be
 * called for adding every single entry) */
#define ENTRY_ALLOCATION_GRANULARITY 25

/* Index entry, 64 bytes size */
typedef struct {
    /* Basic identifying information */
    uint8_t block_type[4];
    uint32_t block_size;
    uint64_t block_timestamp;

    /* These two identify which block this entry represents,
     * by file position and chunk number */
    uint64_t block_pos;
    uint8_t block_chunk;

    /* One block might have a few index entries to store the entire block's
     * data in the index (only for blocks below a size threshold) */
    uint8_t block_part;

    /* Data (always excludes the first 16 bytes) */
    uint8_t data[ENTRY_BYTES];
} mlv_IndexEntry;

struct mlv_Index
{
    /* How many blocks have been indexed */
    uint64_t num_blocks_indexed;
    /* How many index entries there is memory for */
    uint64_t index_size;

    /* How far we've indexed up to */
    struct {
        uint64_t pos;
        uint8_t chunk;
    } indexed_up_to;

    /* Is indexing complete */
    uint8_t indexing_is_complete;

    /* Health. If not zero, do not allow operations */
    uint8_t health;

    /* How many entries memory has been allocated for */
    uint64_t num_entries_memory;
    /* How many entries there actually is */
    uint64_t num_entries;
    /* Index Entries */
    mlv_IndexEntry * entries;
    // mlv_IndexEntry entries2[];
};

typedef struct {
    uint8_t type[4];
    uint32_t size;
    uint64_t timestamp;
} mlv_block;

mlv_Index * mlv_newIndex(mlv_Alloc Allocator, void * AllocatorUD)
{
    mlv_Index * index = mlv_Malloc(Allocator, AllocatorUD, sizeof(mlv_Index));

    index->num_blocks_indexed = 0;
    index->index_size = 0;
    index->indexed_up_to.pos = 0;
    index->indexed_up_to.chunk = 0;
    index->indexing_is_complete = 0;
    index->health = 0;
    index->num_entries_memory = 0;
    index->num_entries = 0;
    index->entries = mlv_Malloc(Allocator, AllocatorUD, 0);

    return index;
}

void mlv_closeIndex(mlv_Index * Index)
{
    mlv_Free(Index->entries);
    mlv_Free(Index);
}

/* TODO: maybe re structure thhis fucntion */
static mlv_IndexEntry * new_entry(mlv_Index * Index)
{
    if (Index->health != 0 || Index->entries == NULL)
    {
        return NULL;
    }

    if (Index->num_entries == Index->num_entries_memory)
    {
        Index->num_entries_memory += ENTRY_ALLOCATION_GRANULARITY;
        Index->entries = mlv_Realloc(Index->entries, sizeof(mlv_IndexEntry) * Index->num_entries_memory);
        
        if (Index->entries == NULL)
        {
            return NULL;
        }
    }
    else if (Index->num_entries > Index->num_entries_memory)
    {
        /* This cannot and should not happen */
        return NULL;
    }

    Index->num_entries++;
    return Index->entries + (Index->num_entries-1);
}

void mlv_IndexBuild(mlv_Index * Index,
                    mlv_DataSource * DataSource,
                    uint64_t MaxBytes,
                    uint64_t MaxBlocks)
{
    /* Reads into here */

    int chunk = Index->indexed_up_to.chunk;
    uint64_t pos = Index->indexed_up_to.pos;
    int num_chunks = mlv_DataSourceGetNumChunks(DataSource);

    /* Keep indexing while 'healthy' */
    while (Index->health == 0 && chunk < num_chunks)
    {
        uint64_t chunk_size = mlv_DataSourceGetChunkSize(DataSource, chunk);

        while ((pos + sizeof(mlv_block)) < chunk_size)
        {
            mlv_block block;

            /************ Check stuff ************/

            /* Handle blocks cut off at end of file. TODO: think about this */
            if ((pos + block.size) > chunk_size)
            {
                /* (Temporary?) solution: reduce block's claimed size in index */
                block.size -= ((pos + block.size) - chunk_size);
            }

            /* Read the block header, making sure a whole header worth of bytes is read */
            if (sizeof(mlv_block) != mlv_DataSourceGetData(DataSource, chunk, pos, sizeof(mlv_block), &block))
            {
                // STOP!!! couldn't read enough data. Perhaps could still continue with other chunks
            }

            /* If the block claims to be smaller than possible, the file is fucked */
            if (block.size < sizeof(block))
            {
                // STOP!!! TODO: decide if/when continuing to other chunks makes sense
            }

            /* Size of the block's data excluding the header part (16 bytes) */
            int data_size = (block.size - sizeof(mlv_block));
            /* How many parts in the index are needed to represent this (round up) */
            int parts = (block.size - 1) / ENTRY_BYTES + 1;

            /* If the block is huge, do not store all of it, so just use one entry */
            if (data_size > MAX_BLOCK_SIZE_TO_FULLY_STORE_IN_INDEX) parts = 1;

            for (int part = 0; Index->health == 0 && part < parts; ++part)
            {
                mlv_IndexEntry * entry = new_entry(Index);

                if (entry == NULL)
                {
                    // STOP!!! allocation error / memory error. Cannot continue at all.
                    Index->health = 1;
                }
                else
                {
                    entry->block_chunk = chunk;
                    entry->block_part = part;
                    entry->block_pos = pos;
                    entry->block_size = block.size;
                    entry->block_timestamp = block.timestamp;
                    for (int i = 0; i < 4; ++i) entry->block_type[i] = block.type[i];

                    // TODO: check the return of this
                    mlv_DataSourceGetData(DataSource, chunk, pos + sizeof(mlv_block) + ENTRY_BYTES * part, ENTRY_BYTES, entry->data);
                }
            }

            pos += block.size;

// #ifdef DEBUG
            printf("Block %c%c%c%c, size %llu, pos %llu, timestamp %llu, %s\n",
                    block.type[0], block.type[1], block.type[2],
                    block.type[3], block.size, pos, block.timestamp, (data_size <= MAX_BLOCK_SIZE_TO_FULLY_STORE_IN_INDEX) ? "Fully stored in index" : "");
// #endif
        }

        /* TODO: make these only increment IF the chunk has been finished.
         * Currently this will go up every time this function is called which is
         * incorrect */
        chunk++;
        pos = 0;
    }

    Index->indexed_up_to.chunk = chunk;
    Index->indexed_up_to.pos = pos;
}

void mlv_IndexOptimise(mlv_Index * Index)
{
    // Sort the index for faster block finding. TODO: decide sorting comparison rules
    // sort();
}

uint32_t mlv_IndexGetBlockData(mlv_Index * Index,
                               uint8_t * BlockType,
                               void * Out,
                               uint32_t Offset,
                               uint32_t NumBytes,
                               uint64_t BlockIndex,
                               mlv_DataSource * DataSource)
{
    // for (int i = 0; i < mlv)

    mlv_IndexEntry * entry = Index->entries;
}