#ifndef _libmlv_write_h_
#define _libmlv_write_h_

#include <stdint.h>

#ifndef MLVWriter_src
typedef void MLVWriter_t;
#endif

/******************************* Initialisation *******************************/

/* Returns amount of memory you need to allocate for an MLV writer */
size_t sizeof_MLVWriter();

void init_MLVWriter( MLVWriter_t * Writer,
                     int Width,
                     int Height,
                     int BitDepth,
                     int Compressed,
                     int BlackLevel,
                     int WhiteLevel,
                     int FPSNumerator,
                     int FPSDenominator );

void uninit_MLVWriter(MLVWriter_t * Writer);

/********************************** Metadata **********************************/

/* For manually setting camera data if it is not in the presets.
 * You can pass NULL for any of these if you'd like not to provide that
 * specific info */
void MLVWriterSetCameraInfo( MLVWriter_t * Writer,
                             char * CameraName,
                             uint32_t CameraModelID,
                             double * ColourMatrix );

/********************************** Writing ***********************************/

/* After setting all of the metadata you want to set and want to begin writing,
 * call this to find out size of header data needing to be written */
size_t MLVWriterGetHeaderSize(MLVWriter_t * Writer);

/* After allocating enough memory based on what MLVWriterGetHeaderSize told you,
 * use this function to get all of that data output to HeaderData pointer. Then
 * you must write it to a file yourself */
void MLVWriterGetHeaderData(MLVWriter_t * Writer, void * HeaderData);

/* Size of frame header */
size_t MLVWriterGetFrameHeaderSize(MLVWriter_t * Writer);

/* Returns frame header data for a frame of index FrameIndex. You must write
 * this to a file, followed by the actual frame data */
void MLVWriterGetFrameHeaderData( MLVWriter_t * Writer,
                                  uint64_t FrameIndex,
                                  size_t FrameSize,
                                  void * FrameHeaderData );

#endif