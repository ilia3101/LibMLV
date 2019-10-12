#include <stdlib.h>
#include <stdint.h>

#include "libmlv_tools.h"

/* Not very nice. Packing function for bitdepths 9-15 */
void mlv_pack_frame(uint16_t * Data, size_t Elements, void * Out, int Bitdepth)
{
    uint8_t * output = (uint8_t *)Out;
    uint64_t bitdepth = Bitdepth;
    uint64_t bit_offset = 0;
    for (size_t i = 0; i < Elements; ++i, bit_offset += bitdepth)
    {
        uint16_t value = Data[i];
        uint64_t byte_offset_start = bit_offset / 8;
        uint64_t byte_offset_end = (bit_offset+bitdepth) / 8;
        uint64_t shift = bit_offset - (byte_offset_start * 8);

        /* This means it spans 3 bytes */
        if ((byte_offset_end-byte_offset_start) == 2)
        {
            output[byte_offset_start] |= value >> (bitdepth-shift);
            output[byte_offset_start+1] = value >> (bitdepth-shift-8);
            output[byte_offset_end] = value >> (bitdepth-shift-16);
        }
        /* Else 2 bytes */
        else if ((byte_offset_end-byte_offset_start) == 1)
        {
            output[byte_offset_start] |= value >> (bitdepth-shift);
            output[byte_offset_end] = value >> (bitdepth-shift-8);
        }
    }
}

void MLVPackFrame14(uint16_t * Data, size_t Elements, void * Out)
{
    mlv_pack_frame(Data, Elements, Out, 14);
}

void MLVPackFrame12(uint16_t * Data, size_t Elements, void * Out)
{
    mlv_pack_frame(Data, Elements, Out, 12);
}

void MLVPackFrame10(uint16_t * Data, size_t Elements, void * Out)
{
    mlv_pack_frame(Data, Elements, Out, 10);
}

// void MLVCompressFrameLJ92( uint16_t * Data,
//                            int Width,
//                            int Height,
//                            int Bitdepth,
//                            void * Out,
//                            size_t * ResultSize );