#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libmlv.h"
#include "libmlvaux.h"

/* Simple implementations of mlv_Alloc, mlv_Reader and mlv_Close */

static void * mlv_alloc(void * ud, void * ptr, uint64_t osize, uint64_t nsize)
{
    if (nsize == 0)
    {
        free(ptr);
        return NULL;
    }
    else
    {
        return realloc(ptr, nsize);
    }
}

static uint64_t mlv_reader(void * ud, uint64_t pos, uint64_t bytes, void * out)
{
    fseek(ud, pos, SEEK_SET);
    return fread(out, bytes, 1, ud);
}

static void mlv_close(void * ud)
{
    fclose(ud);
}

/******** User functions ********/

mlv_Index * mlvL_newIndex()
{
    return mlv_newIndex(mlv_alloc, NULL);
}

mlv_FrameExtractor * mlvL_newFrameExtractor()
{
    return mlv_newFrameExtractor(mlv_alloc, NULL);
}

static int file_exists(char * path)
{
    FILE * file = fopen(path, "r");
    fclose(file);
    return (file == NULL) ? 0 : 1;
}

mlv_DataSource * mlvL_newDataSource(char * MainChunkFileName,
                                    int SearchForAdditionalChunks)
{
    mlv_DataSource * datasource = NULL;

    if (file_exists(MainChunkFileName))
    {
        char * file_names[MLV_MAX_NUM_CHUNKS] = {NULL};
        int num_chunks = 1;
        file_names[0] = MainChunkFileName;

        if (SearchForAdditionalChunks)
        {
            uint64_t path_length = strlen(MainChunkFileName);
            char * path = malloc(path_length+1);
            strcpy(path, MainChunkFileName);

            do {
                path[path_length-1] = '0' + ((num_chunks - 1) % 10);
                path[path_length-2] = '0' + ((num_chunks - 1) / 10);
                if (file_exists(path))
                {
                    file_names[num_chunks] = malloc(path_length+1);
                    strcpy(file_names[num_chunks], path);
                    ++num_chunks;
                }
                else
                {
                    break;
                }
            } while (num_chunks < MLV_MAX_NUM_CHUNKS);
        }

        datasource = mlvL_newDataSourceFromChunks(file_names, num_chunks);

        for (int c = num_chunks-1; c > 0; --c)
        {
            free(file_names[c]);
        }
    }

    return datasource;
}

mlv_DataSource * mlvL_newDataSourceFromChunks(char ** ChunkFileNames,
                                              int NumFiles)
{
    int err = 0;
    if (NumFiles > MLV_MAX_NUM_CHUNKS) return NULL;

    mlv_DataSource * datasource = mlv_newDataSource(mlv_alloc, NULL);

    if (datasource != NULL)
    {
        mlv_DataSourceSetReader(datasource, mlv_reader);
        mlv_DataSourceSetCloser(datasource, mlv_close);
        mlv_DataSourceSetChunkCount(datasource, NumFiles);

        for (int c = 0; c < NumFiles && !err; ++c)
        {
            if (ChunkFileNames[c] != NULL)
            {
                FILE * file = fopen(ChunkFileNames[c], "r");

                if (file != NULL)
                {
                    fseek(file, 0, SEEK_END);
                    uint64_t size = ftell(file);
                    fseek(file, 0, SEEK_SET);

                    mlv_DataSourceSetChunk(datasource, c, file, size, NULL, NULL);
                }
                else err = 1;
            }
            else err = 1;
        }
    }

    if (err)
    {
        /* Something failed, so delete it and return NULL */
        if (datasource != NULL) mlv_closeDataSource(datasource);
        return NULL;
    }
    else
    {
        return datasource;
    }
}