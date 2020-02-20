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

#include <stdlib.h>
#include <stdint.h>

#include "../include/MLVFrameUtils.h"

/* Packing probably only works on little endian (just needs an extra swap at the
 * end to fix this, does not matter right now) */

void MLVPackFrame14(uint16_t * Data, uint32_t Elements, void * Out)
{
    uint16_t * output = Out;
    for (int i = 0; i < Elements; i += 8)
    {
        uint16_t pix_a = Data[ i ] & 0x3FFF;
        uint16_t pix_b = Data[i+1] & 0x3FFF;
        uint16_t pix_c = Data[i+2] & 0x3FFF;
        uint16_t pix_d = Data[i+3] & 0x3FFF;
        uint16_t pix_e = Data[i+4] & 0x3FFF;
        uint16_t pix_f = Data[i+5] & 0x3FFF;
        uint16_t pix_g = Data[i+6] & 0x3FFF;
        uint16_t pix_h = Data[i+7] & 0x3FFF;

        output[0] = (pix_a << 2) | (pix_b >> 12);
        output[1] = (pix_b << 4) | (pix_c >> 10);
        output[2] = (pix_c << 6) | (pix_d >> 8);
        output[3] = (pix_d << 8) | (pix_e >> 6);
        output[4] = (pix_e << 10) | (pix_f >> 4);
        output[5] = (pix_f << 12) | (pix_g >> 2);
        output[6] = (pix_g << 14) | (pix_h);

        output += 7;
    }
}

void MLVPackFrame12(uint16_t * Data, uint32_t Elements, void * Out)
{
    uint16_t * output = Out;
    for (int i = 0; i < Elements; i += 4)
    {
        uint16_t pix_a = Data[ i ] & 0x0FFF;
        uint16_t pix_b = Data[i+1] & 0x0FFF;
        uint16_t pix_c = Data[i+2] & 0x0FFF;
        uint16_t pix_d = Data[i+3] & 0x0FFF;

        output[0] = (pix_a << 4) | (pix_b >> 8);
        output[1] = (pix_b << 8) | (pix_c >> 4);
        output[2] = (pix_c << 12) | (pix_d);

        output += 3;
    }
}

void MLVPackFrame10(uint16_t * Data, uint32_t Elements, void * Out)
{
    uint16_t * output = Out;
    for (int i = 0; i < Elements; i += 8)
    {
        uint16_t pix_a = Data[ i ] & 0x03FF;
        uint16_t pix_b = Data[i+1] & 0x03FF;
        uint16_t pix_c = Data[i+2] & 0x03FF;
        uint16_t pix_d = Data[i+3] & 0x03FF;
        uint16_t pix_e = Data[i+4] & 0x03FF;
        uint16_t pix_f = Data[i+5] & 0x03FF;
        uint16_t pix_g = Data[i+6] & 0x03FF;
        uint16_t pix_h = Data[i+7] & 0x03FF;

        output[0] = (pix_a << 6) | (pix_b >> 4);
        output[1] = (pix_b << 12) | (pix_c << 2) | (pix_d >> 8);
        output[2] = (pix_d << 8) | (pix_e >> 2);
        output[3] = (pix_e << 14) | (pix_f << 4) | (pix_g >> 6);
        output[4] = (pix_g << 10) | (pix_h);

        output += 5;
    }
}

void MLVUnpackFrame14(uint16_t * Data, uint32_t Elements, uint16_t * Out)
{
    for (uint32_t i = 0; i < Elements; i += 8)
    {
        uint16_t word_a = Data[0];
        uint16_t word_b = Data[1];
        uint16_t word_c = Data[2];
        uint16_t word_d = Data[3];
        uint16_t word_e = Data[4];
        uint16_t word_f = Data[5];
        uint16_t word_g = Data[6];

        Out[0] = word_a >> 2;
        Out[1] = ((word_a << 12) | (word_b >>  4)) & 0x3FFF;
        Out[2] = ((word_b << 10) | (word_c >>  6)) & 0x3FFF;
        Out[3] = ((word_c <<  8) | (word_d >>  8)) & 0x3FFF;
        Out[4] = ((word_d <<  6) | (word_e >> 10)) & 0x3FFF;
        Out[5] = ((word_e <<  4) | (word_f >> 12)) & 0x3FFF;
        Out[6] = ((word_f <<  2) | (word_g >> 14)) & 0x3FFF;
        Out[7] = word_g & 0x3FFF;

        Data += 7, Out += 8;
    }
}

void MLVUnpackFrame12(uint16_t * Data, uint32_t Elements, uint16_t * Out)
{
    for (uint32_t i = 0; i < Elements; i += 4)
    {
        uint16_t word_a = Data[0];
        uint16_t word_b = Data[1];
        uint16_t word_c = Data[2];

        Out[0] = word_a >> 4;
        Out[1] = ((word_a << 8) | (word_b >>  8)) & 0x0FFF;
        Out[2] = ((word_b << 4) | (word_b >> 12)) & 0x0FFF;
        Out[3] = word_c & 0x0FFF;

        Data += 3, Out += 4;
    }
}

void MLVUnpackFrame10(uint16_t * Data, uint32_t Elements, uint16_t * Out)
{
    uint16_t * output = Out;
    for (int i = 0; i < Elements; i += 8)
    {
        uint16_t word_a = Data[0];
        uint16_t word_b = Data[1];
        uint16_t word_c = Data[2];
        uint16_t word_d = Data[3];
        uint16_t word_e = Data[4];

        Out[0] = word_a >> 6;
        Out[1] = ((word_a << 4) | (word_b >> 12)) & 0x03FF;
        Out[2] = (word_b >> 2) & 0x03FF;
        Out[3] = ((word_b << 8) | (word_c >> 8)) & 0x03FF;
        Out[4] = (word_c << 2) & 0x03FF;
        Out[5] = ((word_c << 12) | (word_d >> 4)) & 0x03FF;
        Out[6] = ((word_d << 6) | (word_e >> 10)) & 0x03FF;
        Out[7] = word_e & 0x03FF;

        Data += 5, Out += 8;
    }
}

void MLVCompressFrameLJ92( uint16_t * Data,
                           int Width,
                           int Height,
                           int Bitdepth,
                           void * Out,
                           size_t * ResultSize )
{
    return;
}