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

#ifndef _libmlv_tools_h_
#define _libmlv_tools_h_

#include <stdint.h>

/* Packing fucntions for 14, 12 and 10 bit, output memory size should be
 * bitdepth/16 the original size. TODO: check if works */
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