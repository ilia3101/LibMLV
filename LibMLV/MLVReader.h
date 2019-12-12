#ifndef _MLVReader_h_
#define _MLVReader_h_

#include <stdio.h>
#include <stdint.h>

#ifndef MLVReader_src
typedef void MLVReader_t;
#endif

/*********************************** ERRORS ***********************************/

/* Combined errors are these OR'd in positive number space */
#define MLVReader_ERROR_FILE_IO                     -1
#define MLVReader_ERROR_UNSUPPORTED_VIDEO_FORMAT    -2
#define MLVReader_ERROR_UNSUPPORTED_AUDIO_FORMAT    -4
#define MLVReader_ERROR_SKIPPED_FRAME               -8
#define MLVReader_ERROR_CORRUPTED_FRAME             -16
#define MLVReader_ERROR_NOT_ENOUGH_METADATA         -32

/* Generates a description of the error, output string should be 512 bytes */
void MLVReaderGetErrorString(MLVReader_t * MLVReader, int Error, char * Out);

/******************************* Initialisation *******************************/

/* Initialise MLV reader functions, provide block of memory to Reader argument,
 * If it returns a positive value smaller than what you allocated, you do not
 * need to call it again, if it returns a larger valuer, you must reallocate
 * the memory to be that much and call it again, keep repeating if necessary.
 * Any negative return value is an error. */
int64_t init_MLVReaderFromFILEs( MLVReader_t * Reader,
                                 size_t ReaderSize, /* How many bytes Reader is */
                                 FILE ** Files,
                                 int NumFiles,
                                 int MaxFrames );

int64_t init_MLVReaderFromMemory( MLVReader_t * Reader,
                                  size_t ReaderSize,
                                  void ** Files,
                                  uint64_t * FileSizes,
                                  int NumFiles,
                                  int MaxFrames );

/* Uninitialise MLVReader */
void uninit_MLVReader(MLVReader_t * Reader);

/****************************** Metadata getters ******************************/

int MLVReaderGetFrameWidth(MLVReader_t * Reader);
int MLVReaderGetFrameHeight(MLVReader_t * Reader);

double MLVReaderGetFPS(MLVReader_t * Reader);
uint32_t MLVReaderGetFPSNumerator(MLVReader_t * Reader);
uint32_t MLVReaderGetFPSDenominator(MLVReader_t * Reader);

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