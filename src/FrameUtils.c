#include <stdlib.h>
#include <stdint.h>

#include "../include/libmlv_FrameUtils.h"


void MLVPackFrame14(uint16_t * Data, size_t Elements, void * Out)
{
    // mlv_pack_frame(Data, Elements, Out, 14);
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
    // mlv_pack_frame(Data, Elements, Out, 10);
}

// void MLVCompressFrameLJ92( uint16_t * Data,
//                            int Width,
//                            int Height,
//                            int Bitdepth,
//                            void * Out,
//                            size_t * ResultSize );