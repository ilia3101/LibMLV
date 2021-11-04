#include "libmlv.h"

typedef struct
{
    uint64_t size;
    void * ud;
    mlv_Reader reader;
    mlv_Close closer;
}
mlv_DataSource_Chunk;

struct mlv_DataSource
{
    uint8_t num_chunks;
    mlv_Reader reader;
    mlv_Close closer;
    mlv_DataSource_Chunk * chunks;
};

mlv_DataSource * mlv_newDataSource(mlv_Alloc Allocator, void * AllocatorUD)
{
    mlv_DataSource * data_source = mlv_Malloc(Allocator, AllocatorUD, sizeof(mlv_DataSource));

    data_source->num_chunks = 1;
    data_source->chunks = mlv_Malloc2(data_source, sizeof(mlv_DataSource_Chunk));

    return data_source;
}

void mlv_closeDataSource(mlv_DataSource * DataSource)
{
    for (int i = 0; i < DataSource->num_chunks; ++i)
    {
        mlv_DataSource_Chunk * chunk = DataSource->chunks + i;
        mlv_Close closer = (chunk->closer != NULL) ? chunk->closer : DataSource->closer;
        if (closer != NULL) closer(chunk->ud);
    }
    mlv_Free(DataSource->chunks);
    mlv_Free(DataSource);
    return;
}

void mlv_DataSourceSetReader(mlv_DataSource * DataSource, mlv_Reader Reader)
{
    DataSource->reader = Reader;
}

void mlv_DataSourceSetCloser(mlv_DataSource * DataSource, mlv_Close Closer)
{
    DataSource->closer = Closer;
}

void mlv_DataSourceSetChunk(mlv_DataSource * DataSource,
                            int Chunk,
                            void * Data,
                            uint64_t Size,
                            mlv_Reader Reader,
                            mlv_Close Closer)
{
    DataSource->chunks[Chunk].ud = Data;
    DataSource->chunks[Chunk].size = Size;
    DataSource->chunks[Chunk].reader = Reader;
    DataSource->chunks[Chunk].closer = Closer;
}

void mlv_DataSourceSetChunkCount(mlv_DataSource * DataSource, int ChunkCount)
{
    DataSource->chunks = mlv_Realloc(DataSource->chunks, sizeof(mlv_DataSource_Chunk) * ChunkCount);
    DataSource->num_chunks = ChunkCount;
}

int mlv_DataSourceGetNumChunks(mlv_DataSource * DataSource)
{
    return DataSource->num_chunks;
}

uint64_t mlv_DataSourceGetChunkSize(mlv_DataSource * DataSource, int Chunk)
{
    if (Chunk < DataSource->num_chunks)
    {
        return DataSource->chunks[Chunk].size;
    }
    else
    {
        return 0;
    }
}

uint64_t mlv_DataSourceGetData(mlv_DataSource * DataSource,
                               int Chunk,
                               uint64_t Pos,
                               uint64_t Bytes,
                               void * Out)
{
    mlv_DataSource_Chunk * chunk = DataSource->chunks + Chunk;

    mlv_Reader reader = (chunk->reader != NULL) ? chunk->reader : DataSource->reader;

    if (reader != NULL) {
        return reader(chunk->ud, Pos, Bytes, Out);
    } else {
        return 0;
    }
}