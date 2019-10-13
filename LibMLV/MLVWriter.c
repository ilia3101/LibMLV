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
#include <string.h>

#include "mlv_structs.h"

#define MLVWriter_header_block(BlockType, BlockName) \
struct \
{ \
    union { \
        BlockType block; \
        mlv_hdr_t header; \
    }; \
    int write; /* Should this block be written? */ \
} BlockName;

typedef struct
{
    /* Header blocks */
    MLVWriter_header_block(mlv_file_hdr_t, MLVI)
    MLVWriter_header_block(mlv_rawi_hdr_t, RAWI)
    MLVWriter_header_block(mlv_wavi_hdr_t, WAVI)
    MLVWriter_header_block(mlv_expo_hdr_t, EXPO)
    MLVWriter_header_block(mlv_lens_hdr_t, LENS)
    MLVWriter_header_block(mlv_rtci_hdr_t, RTCI)
    MLVWriter_header_block(mlv_idnt_hdr_t, IDNT)
    MLVWriter_header_block(mlv_info_hdr_t, INFO)
    MLVWriter_header_block(mlv_diso_hdr_t, DISO)
    MLVWriter_header_block(mlv_mark_hdr_t, MARK)
    MLVWriter_header_block(mlv_styl_hdr_t, STYL)
    MLVWriter_header_block(mlv_elvl_hdr_t, ELVL)
    MLVWriter_header_block(mlv_wbal_hdr_t, WBAL)

    /* Frame blocks */
    mlv_vidf_hdr_t VIDF;
    mlv_audf_hdr_t AUDF;

} MLVWriter_t;

#define MLVWriter_src
#include "MLVWriter.h"
#undef MLVWriter_src

/* Sets string and block size */
#define mlv_init_block(Block, String) \
{ \
    char * block_name = String; \
    mlv_hdr_t * mlv_block = (mlv_hdr_t *)&Block; \
    for (int i = 0; i < 4; ++i) mlv_block->blockType[i] = String[i]; \
    mlv_block->blockSize = sizeof(Block); \
}

/* Marks a block as to be written */
#define mlv_set_write_block(Block) \
{ \
    Block.write = 1; \
}

size_t sizeof_MLVWriter()
{
    return sizeof(MLVWriter_t);
}

void init_MLVWriter( MLVWriter_t * Writer,
                     int Width,
                     int Height,
                     int BitDepth,
                     int Compressed,
                     int BlackLevel,
                     int WhiteLevel,
                     int FPSNumerator,
                     int FPSDenominator )
{
    /* Zerro everything */
    for (int i = 0; i < sizeof(MLVWriter_t); ++i) ((uint8_t *)Writer)[i] = 0;

    /* Set correct strings and block sizes in all mlv block headers */
    mlv_init_block(Writer->MLVI.block, "MLVI");
    mlv_init_block(Writer->RAWI.block, "RAWI");
    mlv_init_block(Writer->WAVI.block, "WAVI");
    mlv_init_block(Writer->EXPO.block, "EXPO");
    mlv_init_block(Writer->LENS.block, "LENS");
    mlv_init_block(Writer->RTCI.block, "RTCI");
    mlv_init_block(Writer->IDNT.block, "IDNT");
    mlv_init_block(Writer->INFO.block, "INFO");
    mlv_init_block(Writer->DISO.block, "DISO");
    mlv_init_block(Writer->MARK.block, "MARK");
    mlv_init_block(Writer->STYL.block, "STYL");
    mlv_init_block(Writer->ELVL.block, "ELVL");
    mlv_init_block(Writer->WBAL.block, "WBAL");

    /* Initialise the frame blocks, though not as important */
    mlv_init_block(Writer->VIDF, "VIDF");
    mlv_init_block(Writer->AUDF, "AUDF");

    /* Set resolution in all possible fields */
    Writer->RAWI.block.xRes = Width;
    Writer->RAWI.block.yRes = Height;
    Writer->RAWI.block.raw_info.width = Width;
    Writer->RAWI.block.raw_info.height = Height;
    Writer->RAWI.block.raw_info.jpeg.width = Width;
    Writer->RAWI.block.raw_info.jpeg.height = Height;
    Writer->RAWI.block.raw_info.active_area.x2 = Width;
    Writer->RAWI.block.raw_info.active_area.y2 = Height;

    /* Set default bayer pattern */
    Writer->RAWI.block.raw_info.cfa_pattern = 0x02010100;

    /* Compression and bit depth */
    Writer->RAWI.block.raw_info.bits_per_pixel = BitDepth;
    Writer->MLVI.block.videoClass = MLV_VIDEO_CLASS_RAW;
    if (Compressed) Writer->MLVI.block.videoClass |= MLV_VIDEO_CLASS_FLAG_LJ92;

    /* White and black level */
    Writer->RAWI.block.raw_info.black_level = BlackLevel;
    Writer->RAWI.block.raw_info.white_level = WhiteLevel;

    /* MLV version */
    char * ver = MLV_VERSION_STRING;
    for (int i=0; ver[i]!=0; ++i) Writer->MLVI.block.versionString[i] = ver[i];

    /* Framerate */
    Writer->MLVI.block.sourceFpsNom = FPSNumerator;
    Writer->MLVI.block.sourceFpsDenom = FPSNumerator;

    /* Set MLVI and RAWI to be written */
    mlv_set_write_block(Writer->MLVI)
    mlv_set_write_block(Writer->RAWI)
}

void uninit_MLVWriter(MLVWriter_t * Writer)
{
    /* Do nothing, as nothing has been created */
    return;
}

void MLVWriterSetCameraInfo( MLVWriter_t * Writer,
                             char * CameraName,
                             uint32_t CameraModelID,
                             double * ColourMatrix )
{
    if (CameraName != NULL)
        strcpy((char *)&Writer->IDNT.block.cameraName, CameraName);
    Writer->IDNT.block.cameraModel = CameraModelID;

    int32_t * matrix = Writer->RAWI.block.raw_info.color_matrix1;
    if (ColourMatrix != NULL)
    {
        for (int i = 0; i < 9; ++i) {
            matrix[i * 2] = (int32_t)(ColourMatrix[i]*10000 + 0.5);
            matrix[i*2+1] = 10000;
        }
    }
    else
    {
        /* By default, set 5D2 matrix as it is the most important camera */
        int32_t mat5D2[] = { 5309, 10000,  -229, 10000, -336, 10000,
                            -6241, 10000, 13265, 10000, 3337, 10000,
                             -817, 10000,  1215, 10000, 6664, 10000 };
        for (int i = 0; i < 18; ++i) matrix[i] = mat5D2[i];
    }

    /* Set IDNT block to be written */
    mlv_set_write_block(Writer->IDNT)
}

/* Checks if Block should be written, and if it is, the block's size is added
 * to SizeVariable. For calculating size of header data that will be written */
#define mlv_add_block_size(Block, SizeVariable) \
{ \
    if (Block.write) {SizeVariable += Block.header.blockSize; } \
}

size_t MLVWriterGetHeaderSize(MLVWriter_t * Writer)
{
    size_t header_size = 0;

    mlv_add_block_size(Writer->MLVI, header_size)
    mlv_add_block_size(Writer->RAWI, header_size)
    mlv_add_block_size(Writer->WAVI, header_size)
    mlv_add_block_size(Writer->EXPO, header_size)
    mlv_add_block_size(Writer->LENS, header_size)
    mlv_add_block_size(Writer->RTCI, header_size)
    mlv_add_block_size(Writer->IDNT, header_size)
    mlv_add_block_size(Writer->INFO, header_size)
    mlv_add_block_size(Writer->DISO, header_size)
    mlv_add_block_size(Writer->MARK, header_size)
    mlv_add_block_size(Writer->STYL, header_size)
    mlv_add_block_size(Writer->ELVL, header_size)
    mlv_add_block_size(Writer->WBAL, header_size)

    return header_size;
}

/* Copies Block to Pointer and increases pointer by block size, making the
 * pointer ready for the next block to be copied to it */
#define mlv_copy_header_block(Block, Pointer) \
{ \
    if (Block.write) \
    { \
        memcpy(Pointer, &Block, sizeof(Block.block)); \
        Pointer += sizeof(Block.block); \
    } \
}

void MLVWriterGetHeaderData(MLVWriter_t * Writer, void * HeaderData)
{
    uint8_t * pointer = HeaderData;

    mlv_copy_header_block(Writer->MLVI, pointer)
    mlv_copy_header_block(Writer->RAWI, pointer)
    mlv_copy_header_block(Writer->WAVI, pointer)
    mlv_copy_header_block(Writer->EXPO, pointer)
    mlv_copy_header_block(Writer->LENS, pointer)
    mlv_copy_header_block(Writer->RTCI, pointer)
    mlv_copy_header_block(Writer->IDNT, pointer)
    mlv_copy_header_block(Writer->INFO, pointer)
    mlv_copy_header_block(Writer->DISO, pointer)
    mlv_copy_header_block(Writer->MARK, pointer)
    mlv_copy_header_block(Writer->STYL, pointer)
    mlv_copy_header_block(Writer->ELVL, pointer)
    mlv_copy_header_block(Writer->WBAL, pointer)
}

size_t MLVWriterGetFrameHeaderSize(MLVWriter_t * Writer)
{
    return sizeof(mlv_vidf_hdr_t);
}

void MLVWriterGetFrameHeaderData( MLVWriter_t * Writer,
                                  uint64_t FrameIndex,
                                  size_t FrameSize,
                                  void * FrameHeaderData )
{
    Writer->VIDF.blockSize = sizeof(Writer->VIDF) + FrameSize;
    Writer->VIDF.frameNumber = FrameIndex;
    Writer->VIDF.timestamp = FrameIndex*100;

    memcpy(FrameHeaderData, &Writer->VIDF, sizeof(Writer->VIDF));
}