#ifndef _MLVReader_h_
#define _MLVReader_h_

#include <stdio.h>
#include <stdint.h>

#ifndef MLVReader_src
typedef void MLVReader_t;
#endif

/******************************* Initialisation *******************************/

/* Returns how much memory to allocate for an MLVReader, based on the files you
 * provide. Those files should be part of the same clip (.MLV .M00 .M01 etc) */
size_t sizeof_MLVReaderFromFILEs( FILE ** Files,
                                  int NumFiles,
                                  int MaxFrames );

/* Read MLV files from memory (maybe useful for memory mapped files) */
size_t sizeof_MLVReaderFromMemory( void ** Files,
                                   uint64_t * FileSizes,
                                   int NumFiles,
                                   int MaxFrames );

/* Initialise MLV reader from FILEs */
void init_MLVReaderFromFiles( MLVReader_t * Reader,
                              FILE ** Files,
                              int NumFiles );

/* Initialise MLV reader from memory */
void init_MLVReaderFromMemory( MLVReader_t * Reader,
                               void ** Files,
                               uint64_t * FileSizes,
                               int NumFiles );

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

size_t MLVReaderGetRequiredFrameDecodingMemory(MLVReader_t * MLVReader);

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