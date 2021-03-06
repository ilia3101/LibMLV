#ifndef _MLVReader_h_
#define _MLVReader_h_

#include <stdio.h>
#include <stdint.h>

#include "mlv_structs.h"

#include "MLVDataSource.h"

typedef struct MLVReader MLVReader_t;

/*********************************** ERRORS ***********************************/

/* Error codes for MLVReader */
#define MLVReader_ERROR_FILE_IO                     -1
#define MLVReader_ERROR_UNSUPPORTED_IMAGE_FORMAT    -2
#define MLVReader_ERROR_UNSUPPORTED_AUDIO_FORMAT    -3
#define MLVReader_ERROR_SKIPPED_FRAME               -4
#define MLVReader_ERROR_CORRUPTED_FRAME             -5
#define MLVReader_ERROR_CORRUPTED_DATA              -6
#define MLVReader_ERROR_DATA_NOT_AVAILABLE          -7
#define MLVReader_ERROR_BAD_INPUT                   -8
#define MLVReader_ERROR_DROPPED_FRAME               -9

/******************************* Initialisation *******************************/

/* Initialise MLV reader functions, provide block of memory to Reader argument,
 * If it returns a positive value smaller than what you allocated, you do not
 * need to call it again, if it returns a larger value, you must reallocate
 * the memory to be that much and call it again, keep repeating if necessary.
 * Any negative return value is an error. */
int64_t init_MLVReaderFromFILEs( MLVReader_t * Reader,
                                 size_t ReaderSize, /* How many bytes Reader is */
                                 FILE ** Files, /* Will read from these files */
                                 int NumFiles,
                                 int MaxFrames );

int64_t init_MLVReaderFromMemory( MLVReader_t * Reader,
                                  size_t ReaderSize,
                                  void ** Files, /* File(s) in memory */
                                  uint64_t * FileSizes, /* Size of each file */
                                  int NumFiles,
                                  int MaxFrames );

int64_t init_MLVReader( MLVReader_t * Reader,
                        size_t ReaderSize,
                        MLVDataSource_t * DataSource,
                        int MaxFrames );

/* Uninitialise MLVReader */
void uninit_MLVReader(MLVReader_t * Reader);

/******************************** Data getters ********************************/

/* Get block data for block of BlockType, BlockIndex = 0 to get first
 * instance of that block, 1 to get second, etc. MaxBytes argument is maximum
 * number of bytes to read. Positive return value is number of bytes read. */
int64_t MLVReaderGetBlockData( MLVReader_t * Reader,
                               MLVDataSource_t * DataSource,
                               char * BlockType, int BlockIndex,
                               size_t MaxBytes, void * Out );

/* Returns memory needed for using next function (void * DecodingMemory) */
size_t MLVReaderGetFrameDecodingMemorySize( MLVReader_t * MLVReader,
                                            MLVDataSource_t * DataSource );

/* Gets an undebayered, unprocessed frame */
void MLVReaderGetFrame( MLVReader_t * Reader,
                        MLVDataSource_t * DataSource,
                        uint64_t FrameIndex,
                        void * DecodingMemory,
                        uint16_t * Out );

/****************************** Metadata getters ******************************/

/* How many blocks the MLV file has in total */
int32_t MLVReaderGetNumBlocks(MLVReader_t * Reader);

/* Counts how many times blocks of BlockType occur in the MLV */
int32_t MLVReaderGetNumBlocksOfType(MLVReader_t * Reader, char * BlockType);

/* Get frame width and height */
int MLVReaderGetFrameWidth(MLVReader_t * Reader);
int MLVReaderGetFrameHeight(MLVReader_t * Reader);

/* Get black/ white level and bitdepth */
int MLVReaderGetBlackLevel(MLVReader_t * Reader);
int MLVReaderGetWhiteLevel(MLVReader_t * Reader);
int MLVReaderGetBitdepth(MLVReader_t * Reader);

// /* Which pixel bayer pattern starts at in the top left corner (RGGB, 0-3) */
// int MLVReaderGetBayerPixel(MLVReader_t * Reader);

/* Returns two ints to Out */
int MLVReaderGetPixelAspectRatio(MLVReader_t * Reader, int * Out);

/* FPS value */
double MLVReaderGetFPS(MLVReader_t * Reader);
/* Get top and bottom of the FPS fraction */
int32_t MLVReaderGetFPSNumerator(MLVReader_t * Reader);
int32_t MLVReaderGetFPSDenominator(MLVReader_t * Reader);

/* Out should be 32 bytes in length */
void MLVReaderGetCameraName(MLVReader_t * Reader, char * Out);

/* Out should be 32 bytes in length */
void MLVReaderGetLensName(MLVReader_t * Reader, char * Out);

int MLVReaderGetLensFocalLength(MLVReader_t * Reader);

int MLVReaderGetISO(MLVReader_t * Reader, uint64_t FrameIndex);

/************************************ MISC ************************************/

/* TODO: remove this function */
void MLVReaderPrintAllBlocks(MLVReader_t * Reader);

#endif