#include <stdlib.h> // JUST FOR SORT FUNCTION, TEMPORARY
#include <stdint.h>

#include "libmlv.h"

/* For comparing block type strings */
#define BLOCKTYPE_INT(B) ((uint32_t)((B[0]<<24)|(B[1]<<16)|(B[2]<<8)|(B[3])))

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

    /* Data (always excludes the first 16 bytes, as in the header) */
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

/* TODO: maybe re structure thhis fucntion.
 * Allocates an entry in the index, using memory allocation, if required. */
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
                    uint64_t MaxBlocks)
{
    /* Keep count for limiting */
    uint64_t blocks_indexed = 0;

    /* If no limit was given, set limit to U64 max */
    if (MaxBlocks == 0) MaxBlocks = UINT64_MAX;

    int chunk = Index->indexed_up_to.chunk;
    uint64_t pos = Index->indexed_up_to.pos;
    int num_chunks = mlv_DataSourceGetNumChunks(DataSource);

    /* Keep indexing while 'healthy' */
    while (Index->health == 0 && chunk < num_chunks && blocks_indexed < MaxBlocks)
    {
        uint64_t chunk_size = mlv_DataSourceGetChunkSize(DataSource, chunk);

        while ((pos + sizeof(mlv_block)) < chunk_size && blocks_indexed < MaxBlocks)
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

            /* Exclude NULL blocks because they take up most of the index sometimes.
             * Add any other exclusions to this if statement... */
            int allow_nulls = 0;
            if (allow_nulls || BLOCKTYPE_INT(block.type) != BLOCKTYPE_INT("NULL"))
            {
                /* Size of the block's data excluding the header part (16 bytes) */
                int data_size = (block.size - sizeof(mlv_block));
                /* How many parts in the index are needed to represent this (round up) */
                int parts = (data_size - 1) / ENTRY_BYTES + 1;

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

                /* Only count a block if it has been added to the index to make MaxBlocks
                 * parameter more meaningful (there can be a lot of NULLS sometimes) */
                ++blocks_indexed;
            }

            pos += block.size;

            // printf("Block %c%c%c%c, size %llu, pos %llu, timestamp %llu, %s\n",
            //         block.type[0], block.type[1], block.type[2],
            //         block.type[3], (uint64_t)block.size, pos, block.timestamp,
            //         (data_size <= MAX_BLOCK_SIZE_TO_FULLY_STORE_IN_INDEX) ? "Fully stored in index" : "");
        }

        /* DONE????????? TODO: make these only increment IF the chunk has been finished.
         * Currently this will go up every time this function is called which is
         * incorrect */
        if (pos >= chunk_size)
        {
            chunk++; /* DONE??? */
            pos = 0;
        }
    }

    Index->indexed_up_to.chunk = chunk;
    Index->indexed_up_to.pos = pos;
    if (chunk == num_chunks) Index->indexing_is_complete = 1;
}

int mlv_IndexIsComplete(mlv_Index * Index)
{
    return Index->indexing_is_complete;
}

static inline int entry_cmp_for_reading(mlv_IndexEntry * A, mlv_IndexEntry * B);

void mlv_IndexOptimise(mlv_Index * Index)
{
    // Sort the index for faster block finding.
    // TODO: dont use standard library (maybe make this an option)
    if (Index->health == 0)
        qsort(Index->entries, Index->num_entries, sizeof(mlv_IndexEntry), entry_cmp_for_reading);
}

static inline int entry_cmp_for_storage(mlv_IndexEntry * A, mlv_IndexEntry * B);

void mlv_IndexOptimiseForStorage(mlv_Index * Index)
{
    if (Index->health == 0)
        qsort(Index->entries, Index->num_entries, sizeof(mlv_IndexEntry), entry_cmp_for_storage);
}

static inline int does_entry_match(mlv_IndexEntry * Entry,
                                   uint32_t BlockType,
                                   int UseBlockSize, uint32_t MinBlockSize, uint32_t MaxBlockSize,
                                   int UseTimeStamp, uint64_t MinTimestamp, uint64_t MaxTimestamp,
                                   int UseFrameNumber, uint32_t FrameNumber)
{
    return (!BlockType || (BLOCKTYPE_INT(Entry->block_type) == BlockType))
        && (!UseBlockSize || (Entry->block_size >= MinBlockSize && Entry->block_size <= MaxBlockSize))
        && (!UseTimeStamp || (Entry->block_timestamp >= MinTimestamp && Entry->block_timestamp <= MaxTimestamp))
        && (!UseFrameNumber || *((uint32_t *)Entry->data) == FrameNumber); /* Frame number is first uint32 after the block header in both VIDF and AUDF */
}

int64_t mlv_IndexFindEntry( mlv_Index * Index,
                            uint64_t StartPos,
                            uint8_t * BlockType,
                            int UseBlockSize, uint32_t MinBlockSize, uint32_t MaxBlockSize,
                            int UseTimeStamp, uint64_t MinTimestamp, uint64_t MaxTimestamp,
                            int UseFrameNumber, uint32_t FrameNumber,
                            uint64_t EntryNumber )
{
    /* Represent BlockType as int32 for easier comparison */
    uint32_t block_type = 0;
    if (BlockType != NULL) block_type = BLOCKTYPE_INT(BlockType);

    uint64_t entry = StartPos;
    uint64_t num_matches = 0;
    int64_t match_at = -1;

    while (entry < Index->num_entries && num_matches != EntryNumber)
    {
        if (does_entry_match(&Index->entries[entry], block_type,
                             UseBlockSize, MinBlockSize, MaxBlockSize,
                             UseTimeStamp, MinTimestamp, MaxTimestamp,
                             UseFrameNumber, FrameNumber))
        {
            num_matches++;
            match_at = entry;
        }

        /* Now skip to next block in entries, skipping any additional entries for the same block. */
        do { ++entry; } while (entry < Index->num_entries && Index->entries[entry].block_part != 0);
    }

    return match_at;
}

int64_t mlv_IndexGetNextEntry(mlv_Index * Index, uint64_t EntryID)
{
    int64_t next = -1;

    int64_t entry = EntryID;
    do {
        ++entry;
    } while (entry < Index->num_entries && Index->entries[entry].block_part != 0);

    if (entry < Index->num_entries) next = entry;
    return next;
}

/**************** mlv_IndexGetBlockData implementation ****************/

typedef struct {
    uint8_t * data;
    uint32_t num_bytes;
} data_fragment_t;

/* Returns number of copied bytes. TODO: confirm this isn't broken. */
static uint32_t copy_fragmented_data(data_fragment_t * Fragments, int NumFragments, uint32_t Offset, uint32_t BytesToCopy, uint8_t * Output)
{
    int fragments_left = NumFragments;
    data_fragment_t * fragment = Fragments;

    /* Skip first fragments */
    while (Offset > fragment->num_bytes && fragments_left > 0)
    {
        Offset -= fragment->num_bytes;
        fragment += 1;
        fragments_left -= 1;
    }

    uint32_t bytes_copied = 0;
    uint32_t bytes_left = BytesToCopy;

    /* Now do copying. */
    while (fragments_left > 0 && bytes_left > 0)
    {
        uint32_t bytes_this_fragment = fragment->num_bytes - Offset;
        if (bytes_left < bytes_this_fragment) bytes_this_fragment = bytes_left;

        for (uint32_t i = 0; i < bytes_this_fragment; ++i) Output[i] = fragment->data[i];

        Output += bytes_this_fragment;
        bytes_copied += bytes_this_fragment;
        bytes_left -= bytes_this_fragment;
        fragment++;
        fragments_left--;
        Offset = 0; /* Set to 0, as it won't be needed after the first iteration. */
    }

    return bytes_copied;
}

/* Checks if two entries are representing the same block */
static inline int is_entry_same_block(mlv_IndexEntry * A, mlv_IndexEntry * B)
{
    return (A->block_chunk == B->block_chunk) && (A->block_pos == B->block_pos);
}

uint32_t mlv_IndexGetBlockData(mlv_Index * Index,
                               uint64_t EntryID,
                               uint32_t Offset,
                               uint32_t NumBytes,
                               void * Out,
                               mlv_DataSource * DataSource)
{
    mlv_IndexEntry * entry = &Index->entries[EntryID];
    uint8_t * t = entry->block_type;

    /* Reconstruct the block's first 16 bytes */
    mlv_block block_header = {
        .type = {t[0],t[1],t[2],t[3]},
        .size = entry->block_size,
        .timestamp = entry->block_timestamp
    };

    /* This should be enough. Watch out! - It might stop being enough if
     * MAX_BLOCK_SIZE_TO_FULLY_STORE_IN_INDEX is significantly increased!!!! */
#define MAX_FRAGMENTS 11
    /* A nice error to help out... */
#if MAX_FRAGMENTS < ((MAX_BLOCK_SIZE_TO_FULLY_STORE_IN_INDEX-16)/ENTRY_BYTES+1)
#error Not enough fragments to represent data stored in index!\
 May cause unnecessary file reads and is wasteful. Increase the number of fragments.
#endif

    int num_fragments = 1;
    data_fragment_t data_fragments[MAX_FRAGMENTS] = {
        {
            .data = &block_header,
            .num_bytes = sizeof(mlv_block)
        }
    };

    /* Add all index entries as fragments... */
    uint64_t entry_id = EntryID;
    if (Offset+NumBytes > sizeof(mlv_block))
    {
        while (entry_id < Index->num_entries && is_entry_same_block(entry, &Index->entries[entry_id]) && num_fragments < MAX_FRAGMENTS)
        {
            data_fragments[num_fragments] = (data_fragment_t){
                .data = Index->entries[entry_id].data,
                .num_bytes = sizeof(Index->entries[entry_id].data)
            };
            ++num_fragments;
            ++entry_id;
        }
    }

    uint64_t bytes_copied = copy_fragmented_data(data_fragments, num_fragments, Offset, NumBytes, Out);

    if (bytes_copied != NumBytes && DataSource != NULL)
    {
        /* Read the rest from the file now. */
        bytes_copied += mlv_DataSourceGetData(DataSource, entry->block_chunk,
                                              entry->block_pos+Offset+bytes_copied,
                                              NumBytes-bytes_copied,
                                              ((uint8_t *)Out)+bytes_copied);
    }

    return bytes_copied;
}

uint32_t mlv_IndexGetBlockSize(mlv_Index * Index,
                               uint64_t EntryID)
{
    return Index->entries[EntryID].block_size;
}

void mlv_IndexGetBlockLocation(mlv_Index * Index,
                               int64_t EntryID,
                               int * ChunkOut,
                               uint64_t * PosOut)
{
    *ChunkOut = Index->entries[EntryID].block_chunk;
    *PosOut = Index->entries[EntryID].block_pos;
}

uint64_t mlv_IndexGetBlockTimestamp(mlv_Index * Index,
                                    int64_t EntryID)
{
    return Index->entries[EntryID].block_timestamp;
}

void mlv_IndexGetBlockType(mlv_Index * Index,
                           int64_t EntryID,
                           uint8_t * Out)
{
    for (int i = 0; i < 4; ++i) Out[i] = Index->entries[EntryID].block_type[i];
}

uint64_t mlv_IndexGetSize(mlv_Index * Index)
{
    return (Index->num_entries*sizeof(mlv_IndexEntry)) + sizeof(mlv_Index);
}

/* Comparison methods for sorting and searching the index */

static inline int entry_cmp_for_reading(mlv_IndexEntry * A, mlv_IndexEntry * B)
{
    /* Primarily sort by block type */
    uint32_t type_of_a = BLOCKTYPE_INT(A->block_type);
    uint32_t type_of_b = BLOCKTYPE_INT(B->block_type);
    if (type_of_a > type_of_b) return 1;
    else if (type_of_a < type_of_b) return -1;

    /* Then sort by timestamp */
    if (A->block_timestamp > B->block_timestamp) return 1;
    else if (A->block_timestamp < B->block_timestamp) return -1;

    /* Then sort by which part of the entry */
    if (A->block_part > B->block_part) return 1;
    else if (A->block_part < B->block_part) return -1;

    /* If we've reached here, two entries are essentially are the same,
     * and probably means an identical block was written to the MLV, or 
     * something is very wrong */
    return 0;
}

static inline int is_frame(uint32_t type)
{
    return type == BLOCKTYPE_INT("VIDF") || type == BLOCKTYPE_INT("AUDF");
}

static inline int is_MLVI(uint32_t type)
{
    return type == BLOCKTYPE_INT("MLVI");
}

/* Optimal sort for storage */
static inline int entry_cmp_for_storage(mlv_IndexEntry * A, mlv_IndexEntry * B)
{
    /* Primarily sort by block type */
    uint32_t type_of_a = BLOCKTYPE_INT(A->block_type);
    uint32_t type_of_b = BLOCKTYPE_INT(B->block_type);

    /* Puts everything before frame (audio and video) data, and MLVI first. */
    type_of_a = is_frame(type_of_a) ? 2 : (is_MLVI(type_of_a) ? 0 : 1);
    type_of_b = is_frame(type_of_b) ? 2 : (is_MLVI(type_of_b) ? 0 : 1);

    if (type_of_a > type_of_b) return 1;
    else if (type_of_a < type_of_b) return -1;

    /* Then sort by timestamp */
    if (A->block_timestamp > B->block_timestamp) return 1;
    else if (A->block_timestamp < B->block_timestamp) return -1;

    /* Then sort by which part of the entry */
    if (A->block_part > B->block_part) return 1;
    else if (A->block_part < B->block_part) return -1;

    /* If we've reached here, two entries are essentially are the same,
     * and probably means an identical block was written to the MLV, or 
     * something is very wrong */
    return 0;
}


















void mlv_IndexPrint(mlv_Index * Index)
{
    for (uint64_t i = 0; i < Index->num_entries; ++i)
    {
        mlv_IndexEntry entry = Index->entries[i];
        if (entry.block_part != 0) printf(" | P%i", entry.block_part);
        else
        {
            int use_mb = entry.block_size >= (1024*1024*0.2);
            char size_string[100];
            if (use_mb) sprintf(size_string, "%.1lf MiB", (double)entry.block_size/(1024.0*1024.0));
            else sprintf(size_string, "%llu bytes", (uint64_t)entry.block_size);
            printf("\nBlock %c%c%c%c, size %s, pos %llu, timestamp %llu",
                    entry.block_type[0], entry.block_type[1], entry.block_type[2],
                    entry.block_type[3], size_string, entry.block_pos, entry.block_timestamp);
        }
    }
    puts("");
}