#include <stdlib.h>
#include <stdint.h>

#include "../include/libmlv_FrameUtils.h"

/* This probably only works on little endian */

void MLVPackFrame14(uint16_t * Data, size_t Elements, void * Out)
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

void MLVPackFrame12(uint16_t * Data, size_t Elements, void * Out)
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

void MLVPackFrame10(uint16_t * Data, size_t Elements, void * Out)
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

// void MLVCompressFrameLJ92( uint16_t * Data,
//                            int Width,
//                            int Height,
//                            int Bitdepth,
//                            void * Out,
//                            size_t * ResultSize );