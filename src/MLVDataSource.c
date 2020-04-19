/* 
 * MIT License
 *
 * Copyright (C) 2019 Ilia Sibiryakov
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stddef.h>
#include <string.h>
#include <stdint.h>

#include "../include/MLVDataSource.h"

typedef struct {
    uint64_t size;
    uint8_t * pointer;
} MLVDataSource_memory_t;

typedef struct {
    uint64_t size;
    void * handle;
} MLVDataSource_file_t;

struct MLVDataSource
{
    uint8_t type; /* 0=Memory, 1=File */
    int num_files;

    union
    {
        /* FILES */
        struct {
            int64_t (* fread)(void *, int64_t, int64_t, void *);
            int     (* fseek)(void *, int64_t, int);
            int64_t (* ftell)(void *);
            int SEEK_SET_value;
            int SEEK_END_value;
            MLVDataSource_file_t files[];
        };

        /* MEMORY */
        struct {
            int this_makes_error_go_away; /* Having this allows the following flexible array... */
            MLVDataSource_memory_t memory_chunks[];
        };
    };
};

uint64_t sizeof_MLVDataSource(int SourceType, int NumFiles)
{
    if (SourceType == MLVDataSource_TYPE_MEMORY)
    {
        /* TODO: fix this value being too big */
        return sizeof(MLVDataSource_t) + NumFiles * sizeof(MLVDataSource_memory_t);
    }
    else if (SourceType == MLVDataSource_TYPE_FILE)
    {
        return sizeof(MLVDataSource_t) + NumFiles * sizeof(MLVDataSource_file_t);
    }
    else
    {
        return 0;
    }
}

/* You must provide file handles, and fread/fseek/ftell shaped functions */
void init_MLVDataSourceWithFiles( MLVDataSource_t * Source,
                                  void ** FileHandles,
                                  int NumFiles,
                                  int64_t (* Func_fread)(void *, int64_t, int64_t, void *),
                                  int     (* Func_fseek)(void *, int64_t, int),
                                  int64_t (* Func_ftell)(void *),
                                  int SEEK_SET_Value,
                                  int SEEK_END_Value )
{
    Source->type = MLVDataSource_TYPE_FILE;

    Source->num_files = NumFiles;

    Source->SEEK_SET_value = SEEK_SET_Value;
    Source->SEEK_END_value = SEEK_END_Value;

    Source->fread = Func_fread;
    Source->fseek = Func_fseek;
    Source->ftell = Func_ftell;

    for (int f = 0; f < NumFiles; ++f)
    {
        Func_fseek(FileHandles[f], 0, SEEK_END_Value);
        Source->files[f].handle = FileHandles[f];
        Source->files[f].size = Func_ftell(FileHandles[f]);
    }
}

void init_MLVDataSourceWithMemory( MLVDataSource_t * Source,
                                   void ** Files,
                                   int NumFiles,
                                   uint64_t * Sizes )
{
    Source->type = MLVDataSource_TYPE_MEMORY;

    Source->num_files = NumFiles;

    for (int f = 0; f < NumFiles; ++f)
    {
        Source->memory_chunks[f].size = Sizes[f];
        Source->memory_chunks[f].pointer = Files[f];
    }

    return;
}

void uninit_MLVDataSource(MLVDataSource_t * Source)
{
    /* As usual, nothing must be done */
    return;
}

int MLVDataSourceGetType(MLVDataSource_t * Source)
{
    return Source->type;
}

uint64_t MLVDataSourceGetData( MLVDataSource_t * Source,
                               int FileIndex, uint64_t FilePos,
                               uint64_t Bytes, void * Output )
{
    if (Source->type == MLVDataSource_TYPE_FILE)
    {
        if (Source->ftell(Source->files[FileIndex].handle) != FilePos)
        {
            Source->fseek(Source->files[FileIndex].handle, FilePos, Source->SEEK_SET_value);
        }
        uint64_t read = Source->fread(Output, 1, Bytes, Source->files[FileIndex].handle);
        return read;
    }
    else if (Source->type == MLVDataSource_TYPE_MEMORY)
    {
        memcpy(Output, Source->memory_chunks[FileIndex].pointer+FilePos, Bytes);
        return Bytes;
    }
    else
    {
        return 0;
    }
}

void * MLVDataSourceGetDataPointer( MLVDataSource_t * Source,
                                    int FileIndex, uint64_t FilePos )
{
    if (Source->type == MLVDataSource_TYPE_MEMORY)
    {
        return Source->memory_chunks[FileIndex].pointer + FilePos;
    }
    else
    {
        return NULL;
    }
}

uint64_t MLVDataSourceGetFileSize(MLVDataSource_t * Source, int FileIndex)
{
    switch (Source->type)
    {
        case MLVDataSource_TYPE_MEMORY:
            return Source->memory_chunks[FileIndex].size;

        case MLVDataSource_TYPE_FILE:
            return Source->files[FileIndex].size;
        
        default:
            return 0;
    }
}

int MLVDataSourceGetNumFiles(MLVDataSource_t * DataSource)
{
    return DataSource->num_files;
}