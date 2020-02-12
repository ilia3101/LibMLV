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

#ifndef _MLVWriter_h_
#define _MLVWriter_h_

#include <stdint.h>

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

/* Time and date */
//void MLVWriterSetTimeAndDate( MLVWriter_t * Writer,
//                              )

/********************************** Writing ***********************************/

/* After setting all of the metadata you need to set and want to begin writing,
 * call this to find out size of header data needing to be written. Leave
 * this much free at the start of the file, then begin writing frames */
size_t MLVWriterGetHeaderSize(MLVWriter_t * Writer);

/* Size of frame header */
size_t MLVWriterGetFrameHeaderSize(MLVWriter_t * Writer);

/* Returns frame header data for a frame of index FrameIndex. You must write
 * this to a file, followed by the actual frame data */
void MLVWriterGetFrameHeaderData( MLVWriter_t * Writer,
                                  uint64_t FrameIndex,
                                  size_t FrameDataSize,
                                  void * FrameHeaderData );

/* Gets mlv header data, size can be got with MLVWriterGetHeaderSize. Seek to
 * the start of the file after writing all frames and write the header. */
void MLVWriterGetHeaderData( MLVWriter_t * Writer,
                             void * HeaderData,
                             int NumFrames );

#endif
