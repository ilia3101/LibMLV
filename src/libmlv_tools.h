#ifndef _libmlv_tools_h_
#define _libmlv_tools_h_

#include <stdint.h>

/* Packing fucntions for 14, 12 and 10 bit, output memory size should be
 * bitdepth/16 the original size */
void MLVPackFrame14(uint16_t * Data, size_t Elements, void * Out);
void MLVPackFrame12(uint16_t * Data, size_t Elements, void * Out);
void MLVPackFrame10(uint16_t * Data, size_t Elements, void * Out);

/* Compress LJ92, Out memory should be same size as data, resulting compressed
 * size is returned to ResultSize */
void MLVCompressFrameLJ92( uint16_t * Data,
                           int Width,
                           int Height,
                           int Bitdepth,
                           void * Out,
                           size_t * ResultSize );

#endif