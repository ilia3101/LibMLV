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

mlv_DataSource * mlvL_newDataSourceFromMainChunk(char * MainChunkFileName,
                                                 int SearchForAdditionalChunks)
{
    mlv_DataSource * datasource = NULL;
    FILE * files[MLV_MAX_NUM_CHUNKS] = {NULL};
    uint64_t sizes[MLV_MAX_NUM_CHUNKS] = {0};

    /* Try opening first file */    
    files[0] = fopen(MainChunkFileName, "r");
    
    if (files[0] != NULL)
    {
        int num_files = 1;

        if (SearchForAdditionalChunks)
        {
            // TODO

            // size_t path_length = strlen(MainChunkFileName);
            // char * path = malloc(path_length+1);
            // strcpy(MainChunkFileName, path_copy);

            // int chunk_number = 0;

            // do {
            //     path[path_length-1] = '0' + (chunk_number % 10);
            //     path[path_length-2] = '0' + (chunk_number / 10);
            // } while (chunk_number < 100 && files[1+chunk_number] = fopen(path, "r"));
        }

        for (int i = 0; files[i] != NULL; ++i)
        {
            fseek(files[i], 0, SEEK_END);
            sizes[i] = ftell(files[i]);
            fseek(files[i], 0, SEEK_SET);
        }

        datasource = mlv_newDataSource(mlv_alloc, NULL);
        mlv_DataSourceSetChunkCount(datasource, num_files);
        mlv_DataSourceSetReader(datasource, mlv_reader);
        mlv_DataSourceSetCloser(datasource, mlv_close);

        for (int c = 0; c < num_files; ++c)
        {
            mlv_DataSourceSetChunk(datasource, c, files[c], sizes[c], NULL, NULL);
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

                    mlv_DataSourceSetChunk(datasource,
                                           c,
                                           file,
                                           size,
                                           NULL,
                                           NULL);
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
