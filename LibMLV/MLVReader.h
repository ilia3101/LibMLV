#ifndef _MLVReader_h_
#define _MLVReader_h_

#include <stdio.h>
#include <stdint.h>

#ifndef MLVReader_src
typedef void MLVReader_t;
#endif

/*********************************** ERRORS ***********************************/

/* Error bits, invert returned values to get these codes */
#define MLVReader_ERROR_FILE_IO                     0x0001
#define MLVReader_ERROR_UNSUPPORTED_IMAGE_FORMAT    0x0002
#define MLVReader_ERROR_UNSUPPORTED_AUDIO_FORMAT    0x0004
#define MLVReader_ERROR_SKIPPED_FRAME               0x0008
#define MLVReader_ERROR_CORRUPTED_FRAME             0x0010
#define MLVReader_ERROR_CORRUPTED_METADATA          0x0020
#define MLVReader_ERROR_NOT_ENOUGH_METADATA         0x0040
#define MLVReader_ERROR_TOO_MANY_FILES              0x0080
#define MLVReader_ERROR_FILE_TOO_BIG                0x0100
#define MLVReader_ERROR_BAD_ARGUMENT                0x0200 /* You have provided shit input */
#define MLVReader_ERROR_NULL_POINTER                0x0400 /* You are dumb */

/* Generates a description of the error, output string should be 512 bytes */
void MLVReaderGetErrorString(MLVReader_t * MLVReader, int Error, char * Out);

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

/* Uninitialise MLVReader */
void uninit_MLVReader(MLVReader_t * Reader);

/****************************** Metadata getters ******************************/

/* Counts how many blocks of type BlockType there are */
int32_t MLVReaderGetNumBlocks(MLVReader_t * Reader, char * BlockType);
/* Get actual block data for block of BlockType, BlockIndex = 0 to get first
 * instance of that block, 1 to get second, etc. Bytes argument is maximum bytes
 * to get of that block, outputs to Out. */
int64_t MLVReaderGetBlockDataFromFiles( MLVReader_t * Reader, FILE ** Files,
                                        char * BlockType, int BlockIndex,
                                        size_t Bytes, void * Out );
int64_t MLVReaderGetBlockDataFromMemory( MLVReader_t * Reader, void ** Files,
                                         char * BlockType, int BlockIndex, 
                                         size_t Bytes, void * Out );

/* Get frame width and height */
int MLVReaderGetFrameWidth(MLVReader_t * Reader);
int MLVReaderGetFrameHeight(MLVReader_t * Reader);

/* Returns two ints to Out */
int MLVReaderGetPixelAspectRatio(MLVReader_t * Reader, int * Out);

/* FPS value */
double MLVReaderGetFPS(MLVReader_t * Reader);
/* Get top and bottom of the FPS fraction */
int32_t MLVReaderGetFPSNumerator(MLVReader_t * Reader);
int32_t MLVReaderGetFPSDenominator(MLVReader_t * Reader);

char * MLVReaderGetCameraName(MLVReader_t * Reader);

char * MLVReaderGetLensName(MLVReader_t * Reader);
int MLVReaderGetLensFocalLength(MLVReader_t * Reader);

int MLVReaderGetISO(MLVReader_t * Reader, uint64_t FrameIndex);
// int MLVReaderGetISO(MLVReader_t * Reader, uint64_t FrameIndex);

/******************************** Frame getters *******************************/

size_t MLVReaderGetFrameDecodingMemorySize(MLVReader_t * MLVReader);

/* Gets an undebayered frame from MLV file */
void MLVReaderGetFrameFromFile( MLVReader_t * MLVReader,
                                FILE ** Files,
                                void * DecodingMemory,
                                uint64_t FrameIndex,
                                uint16_t * FrameOutput );

/* Gets undebayered frame from MLV in memory */
void MLVReaderGetFrameFromMemory( MLVReader_t * MLVReader,
                                  void ** Files,
                                  void * DecodingMemory,
                                  uint64_t FrameIndex,
                                  uint16_t * FrameOutput );

#endif