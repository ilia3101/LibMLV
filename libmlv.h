#ifndef _libmlv_h_
#define _libmlv_h_

#include <stdint.h> /* For integer types */
#include <stddef.h> /* For NULL */
#include <stdio.h>

/******************************* Callback types *******************************/

/* Allocator */
typedef void * (* mlv_Alloc) (void * ud,
                              void * ptr,
                              uint64_t osize,
                              uint64_t nsize);

/* Return: number of bytes read */
typedef uint64_t (* mlv_Reader) (void * ud,
                                 uint64_t pos,
                                 uint64_t bytes,
                                 void * out);

typedef void (* mlv_Close) (void * ud);

/******************************************************************************/

/******************************* MLV DataSource *******************************/

typedef struct mlv_DataSource mlv_DataSource;

mlv_DataSource * mlv_newDataSource(mlv_Alloc Allocator, void * AllocatorUD);
void mlv_closeDataSource(mlv_DataSource * DataSource);

/* Reader function, absolutely required for data source to work */
void mlv_DataSourceSetReader(mlv_DataSource * DataSource, mlv_Reader Reader);

/* Optional, can be NULL if you 'close' the data sources/files afterwards */
void mlv_DataSourceSetCloser(mlv_DataSource * DataSource, mlv_Close Closer);

/* Set data pointer for chunk by index, (will be passed to reader and closer) */
void mlv_DataSourceSetChunk(mlv_DataSource * DataSource,
                            int Chunk,
                            void * Data,
                            uint64_t Size,
                            /* The following are optional (chunk specific) */
                            mlv_Reader Reader,
                            mlv_Close Closer);

/* Set chunk count */
void mlv_DataSourceSetChunkCount(mlv_DataSource * DataSource, int ChunkCount);

/* Returns number of chunks in the MLV */
int mlv_DataSourceGetNumChunks(mlv_DataSource * DataSource);

/* Returns the size of a chunk */
uint64_t mlv_DataSourceGetChunkSize(mlv_DataSource * DataSource, int Chunk);

/* Get data, used by index and frame extractor. Returns number of bytes read */
uint64_t mlv_DataSourceGetData(mlv_DataSource * DataSource,
                               int Chunk,
                               uint64_t Pos,
                               uint64_t Bytes,
                               void * Out);

/******************************************************************************/

/********************************* MLV Index **********************************/

typedef struct mlv_Index mlv_Index;

mlv_Index * mlv_newIndex(mlv_Alloc Allocator, void * AllocatorUD);
void mlv_closeIndex(mlv_Index * Index);

/* Will perform indexing.  MaxBlocks can be used to limit how much
 * indexing this function will do in one go, if you pass zero, it will
 * index the whole MLV. */
void mlv_IndexBuild(mlv_Index * Index,
                    mlv_DataSource * DataSource,
                    uint64_t MaxBlocks);

/* Checks if indexing is complete */
int mlv_IndexIsComplete(mlv_Index * Index);

/* This will optimise (sort) the index for better performance.
 * Call this after the clip is fully indexed otherwise it's a waste, as more
 * indexing will just undo the sorting just done by this. */
void mlv_IndexOptimise(mlv_Index * Index);

/* Returns position of an entry (represnting a block). Returns negative value
 * if what you asked for is not found. If multiple blocks match your search
 * parameters, you will get the first one of the matches. */
int64_t mlv_IndexFindEntry( mlv_Index * Index,
                            /* StartFrom - Pass zero, unless you are querying the same,
                             * but want to find the next matching entry, then you may pass
                             * the previous call's return value, and 2 to EntryNumber to get the next one. */
                            uint64_t StartEntry,
                            /* BlockType - Required, string such as "VIDF" */
                            uint8_t * BlockType,
                            /* Use block size as search criteria? - yes/no, min, max */
                            int UseBlockSize, uint32_t MinBlockSize, uint32_t MaxBlockSize,
                            /* Use block size as search criteria? - yes/no, min, max */
                            int UseTimeStamp, uint64_t MinTimestamp, uint64_t MaxTimestamp,
                            /* Use frame number as search criteria? - yes/no, value
                             * Only use for AUDF and VIDF! */
                            int UseFrameNumber, uint32_t FrameNumber,
                            /* EntryNumber - which entry do you want to be found.
                             * Pass 1 to get the first one that matches. Passing 2
                             * will return the second matching entry and so on... */
                            uint64_t EntryNumber );

/* Retrieves data of a block. Will return how many bytes were output.
 * This function is able to retreive smaller blocks fully from the index without
 * using the datasource. If you do not pass a DataSource, this function will
 * do whatever is possible using only the index. */
uint32_t mlv_IndexGetBlockData(mlv_Index * Index,
                               uint64_t EntryID, /* Return value of mlv_IndexFindEntry */
                               uint32_t Offset, /* Offset within the block from which to return data */
                               uint32_t NumBytes, /* How many bytes to get */
                               void * Out, /* Data output */
                               mlv_DataSource * DataSource);

/* Returns size of a block based on it's entry ID, which you may get by using mlv_IndexFindEntry */
uint32_t mlv_IndexGetBlockSize(mlv_Index * Index,
                               uint64_t EntryID);

/* Returns the file chunk and position of a block through output arguments */
void mlv_IndexGetBlockLocation(mlv_Index * Index,
                               int64_t EntryID,
                               int * ChunkOut,
                               uint64_t * PosOut);

/* Returns timestamp of a block */
uint64_t mlv_IndexGetBlockTimestamp(mlv_Index * Index,
                                    int64_t EntryID);

// TEMPORARY
void mlv_IndexPrint(mlv_Index * Index);
uint64_t mlv_IndexGetSize(mlv_Index * Index); // how many entries

/******************************************************************************/

/***************************** MLV Frame Extractor ****************************/

typedef struct mlv_FrameExtractor mlv_FrameExtractor;

mlv_FrameExtractor * mlv_newFrameExtractor(mlv_Alloc Allocator, void * AllocatorUD);

void * mlv_FrameExtractorGetFrameData(mlv_FrameExtractor * FrameExtractor,
                                      mlv_Index * Index,
                                      mlv_DataSource * DataSource,
                                      uint64_t FrameNumber,
                                      uint64_t * NumBytesOut,
                                      int AllowIndexing);

uint16_t * mlv_FrameExtractorGetFrame(mlv_FrameExtractor * FrameExtractor,
                                      mlv_Index * Index,
                                      mlv_DataSource * DataSource,
                                      uint64_t FrameNumber,
                                      int AllowIndexing);

uint16_t * mlv_FrameExtractorGetAudioData(mlv_FrameExtractor * FrameExtractor,
                                          uint64_t AudioFrameNumber,
                                          mlv_Index * Index,
                                          mlv_DataSource * DataSource,
                                          uint64_t * NumSamplesOut,
                                          int AllowIndexing);

/* Will free any frame data (happens automatically anyway on next frame) */
void mlv_FrameExtractorFree(mlv_FrameExtractor * FrameExtractor);

/******************************************************************************/

/* MLV Constants */
#define MLV_MAX_NUM_CHUNKS 101

/* Private utility functions */
void * mlv_Malloc(mlv_Alloc Allocator, void * AllocatorUD, uint64_t Size);
void * mlv_Malloc2(void * UseAllocatorFrom, uint64_t Size);
void mlv_Free(void * Pointer);
void * mlv_Realloc(void * Pointer, uint64_t NewSize);

#endif