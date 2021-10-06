#ifndef _MLVDataSource_h_
#define _MLVDataSource_h_

#include <stdint.h>

typedef struct MLVDataSource MLVDataSource_t;
#define MLVDataSource_TYPE_MEMORY 0
#define MLVDataSource_TYPE_FILE   1

/* Get how much to allocate for an MLVDataSource */
uint64_t sizeof_MLVDataSource(int SourceType, int NumFiles);

/* You must provide file handles, and fread/fseek/ftell shaped functions */
void init_MLVDataSourceWithFiles( MLVDataSource_t * Source,
                                  void ** FileHandles,
                                  int NumFiles,
                                  int64_t (* Func_fread)(void *, int64_t, int64_t, void *),
                                  int     (* Func_fseek)(void *, int64_t, int),
                                  int64_t (* Func_ftell)(void *),
                                  int SEEK_SET_Value,
                                  int SEEK_END_Value );

void init_MLVDataSourceWithMemory( MLVDataSource_t * Source,
                                   void ** Files,
                                   int NumFiles,
                                   uint64_t * Sizes );

/* Call this before freeing memory */
void uninit_MLVDataSource(MLVDataSource_t * Source);



/*********** The rest of the functions are only for use by MLVReader **********/

/* Returns MLVDataSource_TYPE_MEMORY or MLVDataSource_TYPE_FILE */
int MLVDataSourceGetType(MLVDataSource_t * Source);

/* Return value is how many bytes of data have been retrieved */
uint64_t MLVDataSourceGetData( MLVDataSource_t * Source,
                               int FileIndex, uint64_t FilePos,
                               uint64_t Bytes, void * Output );

/* Will return NULL if type is FILE */
void * MLVDataSourceGetDataPointer( MLVDataSource_t * Source,
                                    int FileIndex, uint64_t FilePos );

/* Size of file */
uint64_t MLVDataSourceGetFileSize(MLVDataSource_t * DataSource, int FileIndex);

/* How many fils it contains */
int MLVDataSourceGetNumFiles(MLVDataSource_t * DataSource);

#endif