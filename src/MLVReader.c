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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>

#include "../include/MLVReader.h"
#include "../include/mlv_structs.h"

#define MLVReader_string "MLVReader 0.1"

#define MLVReader_header_block(BlockType, BlockName) \
struct \
{ \
    /* Store only one */ \
    union { \
        BlockType block; \
        mlv_hdr_t header; \
    }; \
    /* uint8_t exists; */ /* Was this block found in a file */ \
    /* uint16_t empty_space; */ \
    uint32_t num_blocks; /* How many of this block was found */ \
    /* File index and location of the first occurence of this block */ \
    uint8_t file_index; \
    uint64_t file_location; \
} BlockName;

/* This structure has some unused bytes */
typedef struct
{
    uint8_t block_type[4]; /* Block name string */
    uint32_t size; /* Size of block */
    uint8_t file_index; /* Which file the block is in */
    uint64_t file_location; /* Location of the block in file */
    uint64_t time_stamp;

    union
    {
        /* All blocks */
        struct {
            uint32_t offset; /* Offset from frame header to frame data */
            uint32_t size; /* Useful only audio/compressed video frames */
        } frame;

        /* EXPO blocks */
        struct {
            uint32_t iso; /* True ISO of data */
            uint32_t shutter; /* Exposure time in microseconds */
        } expo;
    };
} MLVReader_block_info_t;

struct MLVReader
{
    char string[16];

    /* Header blocks */
    MLVReader_header_block(mlv_file_hdr_t, MLVI)
    MLVReader_header_block(mlv_rawi_hdr_t, RAWI)
    MLVReader_header_block(mlv_wavi_hdr_t, WAVI)
    MLVReader_header_block(mlv_expo_hdr_t, EXPO)
    MLVReader_header_block(mlv_lens_hdr_t, LENS)
    MLVReader_header_block(mlv_rtci_hdr_t, RTCI)
    MLVReader_header_block(mlv_idnt_hdr_t, IDNT)
    MLVReader_header_block(mlv_info_hdr_t, INFO)
    MLVReader_header_block(mlv_diso_hdr_t, DISO)
    MLVReader_header_block(mlv_mark_hdr_t, MARK)
    MLVReader_header_block(mlv_styl_hdr_t, STYL)
    MLVReader_header_block(mlv_elvl_hdr_t, ELVL)
    MLVReader_header_block(mlv_wbal_hdr_t, WBAL)
    MLVReader_header_block(mlv_rawc_hdr_t, RAWC)

    /* Block counters */
    uint32_t num_blocks;
    uint32_t num_misc_blocks;
    uint32_t num_expo_blocks;
    uint32_t num_audio_frames;
    uint32_t num_video_frames;

    /* Some info */
    uint32_t biggest_video_frame; /* Biggest video frame size */

    /* Only used while parsing */
    uint8_t finished_parsing; /* Multiple files only go up to 100 */
    uint8_t file_index; /* Current file (Multiple files only go up to 100) */
    uint64_t file_pos; /* Position in current file */
    uint64_t total_frame_data; /* How many bytes of frame data have been seen */

    /* Array of block info, but sorted in to order by categories, then by
     * timestamp. Misc blocks first, then EXPOs, then AUDFs, then VIDFs */
    MLVReader_block_info_t blocks[/* num_blocks */];

};

static void print_size(uint64_t Size)
{
    if (Size < 1024)
        printf("%i Bytes\n", (int)Size);
    else if (Size < 1024*1024)
        printf("%.1f KiB\n", ((float)Size)/1024.0f);
    else if (Size < 1024*1024*1024)
        printf("%.2f MiB\n", ((float)Size)/(1024.0f*1024.0f));
    else
        printf("%.2f GiB\n", ((float)Size)/((float)(1024*1024*1024)));
}

/* Value for quicksort (so it gets sorted in correct order) */
/* Order: video frames, audio frames, EXPOs, misc blocks... */
static inline uint64_t BlockValue(MLVReader_block_info_t * Block)
{
    /* Number prefix for sorting order (comment above) */
    uint64_t prefix;

    if (memcmp(Block->block_type, "VIDF", 4) == 0) prefix = 0;
    else if (memcmp(Block->block_type, "AUDF", 4) == 0) prefix = 1;
    else if (memcmp(Block->block_type, "EXPO", 4) == 0) prefix = 2;
    else prefix = 3;

    /* TODO: don't crop timestamp */
    return ((prefix << 56UL) | (Block->time_stamp & (uint64_t)0x00ffffffffffffffULL));
}

// #define TypeOfBlock(Block) \
//     (memcmp(Block->block_type, "VIDF", 4)) ? (0) : ( \
//     (memcmp(Block->block_type, "AUDF", 4)) ? (1) : ( \
//     (memcmp(Block->block_type, "EXPO", 4)) ? (2) : 3 \
// ))

// static inline int CompareBlock(MLVReader_block_info_t * Block1, MLVReader_block_info_t * Block2, int Direction)
// {
//     int block1type = TypeOfBlock(Block1);
//     int block2type = TypeOfBlock(Block2);

//     if (direction == 0) /* Block1 smaller than or equal to */
//     {
//         if (block1type < block2type) return 1;
//         // else if (block1type > block2type) return 0;
//         else if (Block2->time_stamp )
//     }
//     else /* Block1 larger than */
//     {
//         return 0;
//     }
// }

/* TODO: find better solution than copied recursive quicksort */
static void quicksort(MLVReader_block_info_t * Blocks, int first, int last)
{
   int i, j, pivot;

   MLVReader_block_info_t temp;

   if(first<last){
      pivot=first;
      i=first;
      j=last;

      while(i<j){
         while(BlockValue(&Blocks[i]) <= BlockValue(&Blocks[pivot]) && i<last)
            i++;
         while(BlockValue(&Blocks[j])>BlockValue(&Blocks[pivot]))
            j--;
         if(i<j){
            temp=Blocks[i];
            Blocks[i]=Blocks[j];
            Blocks[j]=temp;
         }
      }

      temp=Blocks[pivot];
      Blocks[pivot]=Blocks[j];
      Blocks[j]=temp;
      quicksort(Blocks,first,j-1);
      quicksort(Blocks,j+1,last);

   }
}

static void MLVReader_sort_blocks(MLVReader_t * Reader)
{
    // MLVReaderPrintAllBlocks(Reader);
    quicksort(Reader->blocks, 0, Reader->num_blocks-1);
}

/* TODO: this function is pretty great, but could be made even cleaner
 * IDEA: create a separate object/function for mapping blocks of an MLV file */
static size_t init_mlv_reader( MLVReader_t * Reader, size_t ReaderSize,
                               MLVDataSource_t * DataSource, int MaxFrames )
{
    /* TODO: FIX THIS */
    if (MaxFrames == 0) MaxFrames = INT32_MAX;

    /* Check if enough memory at all */
    if (ReaderSize < (sizeof(MLVReader_t) + sizeof(MLVReader_block_info_t)))
    {
        /* Most MLV clips out there are probably less than 800 blocks,
         * so this is a safe suggestion */
        return sizeof(MLVReader_t) + sizeof(MLVReader_block_info_t) * 800;
    }

    /* If not initialised, zero memory and put string at start */
    if (strcmp(MLVReader_string, (char *)Reader))
    {
        memset(Reader, 0, sizeof(MLVReader_t));
        strcpy(Reader->string, MLVReader_string);
    }

    /* How many blocks can fit in the memory given */
    uint32_t max_blocks = (ReaderSize-sizeof(MLVReader_t)) / sizeof(MLVReader_block_info_t);
    MLVReader_block_info_t * block = Reader->blocks; /* Output to here */

    /* File state */
    uint64_t current_file_size = MLVDataSourceGetFileSize(DataSource, Reader->file_index);

    /* While max blocks is not filled */
    while ( Reader->file_pos < current_file_size
         && Reader->num_blocks < max_blocks
         && Reader->file_index < MLVDataSourceGetNumFiles(DataSource)
         && Reader->num_video_frames < MaxFrames
         && !Reader->finished_parsing )
    {
        #define BLOCK_DATA_MAX 64
        union {
            uint8_t block_data[BLOCK_DATA_MAX];
            mlv_hdr_t header;
        } h;

        int read_bytes = (BLOCK_DATA_MAX < current_file_size-Reader->file_pos) ? BLOCK_DATA_MAX : current_file_size-Reader->file_pos;
        MLVDataSourceGetData(DataSource, Reader->file_index, Reader->file_pos, read_bytes, h.block_data);

        /* Check if the file ends before block or if block is too small */
        if ((Reader->file_pos+h.header.blockSize) > current_file_size || h.header.blockSize < 16) {
            /* TODO: deal with this in some way */
        }

        /* Print block (unless its null or vidf) */
        if (memcmp(h.header.blockType, "NULL", 4) && memcmp(h.header.blockType, "VIDF", 4))
        {
            printf("%lu Block '%.4s' ", h.header.timestamp, (char *)h.header.blockType);
            print_size(h.header.blockSize);
        }

        /* Update block counts */
        if (memcmp(h.header.blockType, "VIDF", 4) == 0) ++Reader->num_video_frames;
        if (memcmp(h.header.blockType, "AUDF", 4) == 0) ++Reader->num_audio_frames;
        if (memcmp(h.header.blockType, "EXPO", 4) == 0) ++Reader->num_expo_blocks;
        else ++Reader->num_misc_blocks;

        /* Set info for other default blocks... */
        #define MLVReader_read_block(BlockType) \
        { \
            if (memcmp(h.header.blockType, #BlockType, 4) == 0) \
            { \
                /* Store first instance of each block */ \
                if (Reader->BlockType.num_blocks == 0) \
                { \
                    MLVDataSourceGetData( DataSource, Reader->file_index, \
                                          Reader->file_pos, \
                                          sizeof(Reader->BlockType.block), \
                                          &Reader->BlockType.block ); \
                } \
                Reader->BlockType.num_blocks++; \
            } \
        }

        MLVReader_read_block(MLVI)
        MLVReader_read_block(RAWI)
        MLVReader_read_block(WAVI)
        MLVReader_read_block(EXPO)
        MLVReader_read_block(LENS)
        MLVReader_read_block(RTCI)
        MLVReader_read_block(IDNT)
        MLVReader_read_block(INFO)
        MLVReader_read_block(DISO)
        MLVReader_read_block(MARK)
        MLVReader_read_block(STYL)
        MLVReader_read_block(ELVL)
        MLVReader_read_block(WBAL)
        MLVReader_read_block(RAWC)

        /* Set block info */
        {
            memcpy(block->block_type, h.header.blockType, 4);
            block->time_stamp = h.header.timestamp;
            block->size = h.header.blockSize;
            block->file_index = Reader->file_index;
            block->file_location = Reader->file_pos;

            if (memcmp(h.header.blockType, "AUDF", 4) == 0)
            {
                block->frame.offset = ((mlv_audf_hdr_t *)h.block_data)->frameSpace;
                block->frame.size = h.header.blockSize - (sizeof(mlv_audf_hdr_t) + block->frame.offset);
                Reader->total_frame_data += h.header.blockSize;
            }
            else if (memcmp(h.header.blockType, "VIDF", 4) == 0)
            {
                block->frame.offset = ((mlv_vidf_hdr_t *)h.block_data)->frameSpace;
                block->frame.size = h.header.blockSize - (sizeof(mlv_vidf_hdr_t) + block->frame.offset);
                Reader->total_frame_data += h.header.timestamp;
            }
            else if (memcmp(h.header.blockType, "EXPO", 4) == 0)
            {
                block->expo.iso = ((mlv_expo_hdr_t *)h.block_data)->isoValue;
                block->expo.shutter = (uint32_t)((mlv_expo_hdr_t *)h.block_data)->shutterValue; /* TODO: maybe fix the int64-int32 thing here */
            }
            else {;} /* No special info stored for other types */
        }

        Reader->file_pos += h.header.blockSize;
        ++Reader->num_blocks;
        ++block;

        /* Go to next file if reached end, unless its last file anyway */
        if (Reader->file_pos >= current_file_size && Reader->file_index != (MLVDataSourceGetNumFiles(DataSource)-1))
        {
            ++Reader->file_index;
            Reader->file_pos = 0;
            current_file_size = MLVDataSourceGetFileSize(DataSource, Reader->file_index);
            printf("video frames: %i\n\n", Reader->num_video_frames);
        }
    }

    /* If not scanned through all files, we need more memory */
    if (!Reader->finished_parsing && (Reader->file_pos < current_file_size || Reader->file_index != (MLVDataSourceGetNumFiles(DataSource)-1)))
    {
        /* Estimate memory required for whole MLV to be read */
        uint64_t total_file_size = 0;
        for (int f = 0; f < MLVDataSourceGetNumFiles(DataSource); ++f)
            total_file_size += MLVDataSourceGetFileSize(DataSource, Reader->file_index);

        uint64_t average_frame_size = 100;
        if (Reader->total_frame_data != 0)
        {
            average_frame_size = Reader->total_frame_data / (Reader->num_audio_frames + Reader->num_video_frames);
        }
        else
        {
            /* Lets say it's an EOS M shooting 10 bit lossless (kinda the lowest common denominator) */
            average_frame_size = ((1728 * 978 * 10) / 8) * 0.57;
        }

        /* An estimate: 10 misc blocks per file */
        return ReaderSize + (average_frame_size*total_file_size + 20 + 10*MLVDataSourceGetNumFiles(DataSource)) * sizeof(MLVReader_block_info_t);
    }
    else
    {
        /* Sort blocks, then return memory used, done */
        MLVReader_sort_blocks(Reader);
        puts("great success!");
        Reader->finished_parsing = 1;
        return sizeof(MLVReader_t) + sizeof(MLVReader_block_info_t) * Reader->num_blocks;
    }
}

int64_t init_MLVReader( MLVReader_t * Reader,
                        size_t ReaderSize,
                        MLVDataSource_t * DataSource,
                        int MaxFrames )
{
    if (Reader == NULL) return MLVReader_ERROR_BAD_INPUT;
    if (DataSource == NULL) return MLVReader_ERROR_BAD_INPUT;

    return init_mlv_reader(Reader, ReaderSize, DataSource, MaxFrames);
}

void uninit_MLVReader(MLVReader_t * Reader)
{
    /* Nothing to do as usual */
    Reader = Reader;
    return;
}

int32_t MLVReaderGetNumBlocks(MLVReader_t * Reader)
{
    return Reader->num_blocks;
}

int32_t MLVReaderGetNumBlocksOfType(MLVReader_t * Reader, char * BlockType)
{
    if (BlockType == NULL) return Reader->num_blocks;

    char block_type[4]; /* local */
    memcpy(block_type, BlockType, 4);

    if (memcmp(block_type, "VIDF", 4) == 0) return Reader->num_video_frames;
    if (memcmp(block_type, "AUDF", 4) == 0) return Reader->num_audio_frames;

    #define MLVReader_block_count_check(CheckBlockType) \
    { \
        if (strcmp(block_type, #CheckBlockType) == 0) return Reader->CheckBlockType.num_blocks; \
    }

    MLVReader_block_count_check(MLVI)
    MLVReader_block_count_check(RAWI)
    MLVReader_block_count_check(WAVI)
    MLVReader_block_count_check(EXPO)
    MLVReader_block_count_check(LENS)
    MLVReader_block_count_check(RTCI)
    MLVReader_block_count_check(IDNT)
    MLVReader_block_count_check(INFO)
    MLVReader_block_count_check(DISO)
    MLVReader_block_count_check(MARK)
    MLVReader_block_count_check(STYL)
    MLVReader_block_count_check(ELVL)
    MLVReader_block_count_check(WBAL)
    MLVReader_block_count_check(RAWC)

    /* Now if none of the previous checks returned, actually count blocks */
    uint32_t block_count = 0;
    MLVReader_block_info_t * blocks = Reader->blocks + (Reader->num_blocks - Reader->num_misc_blocks);

    for (int b = 0; b < Reader->num_misc_blocks; ++b)
    {
        if (memcmp(blocks[b].block_type, block_type, 4) == 0) ++block_count;
    }

    return block_count;
}

int64_t MLVReaderGetBlockData( MLVReader_t * Reader,
                               MLVDataSource_t * DataSource,
                               char * BlockType, int BlockIndex,
                               size_t MaxBytes, void * Out )
{
    uint64_t blocks_found = 0;
    uint64_t read_size = 0;
    uint64_t actually_read = 0; /* How mnuch actually got read */

    for (int b = 0; b < Reader->num_misc_blocks; ++b)
    {
        MLVReader_block_info_t * block = &Reader->blocks[b];

        if (memcmp(block->block_type, BlockType, 4) == 0)
        {
            if (blocks_found == BlockIndex)
            {
                read_size = (MaxBytes < block->size) ? MaxBytes : block->size;
                actually_read = MLVDataSourceGetData( DataSource,
                                                      block->file_index,
                                                      block->file_location,
                                                      read_size, Out );
                return actually_read;
            }
            else
            {
                ++blocks_found;
            }
        }
    }

    return 0;
}

/* Returns memory needed for using next two functions (void * DecodingMemory) */
size_t MLVReaderGetFrameDecodingMemorySize(MLVReader_t * MLVReader)
{
    /* TODO: fix this */
    return 1000;
}

/* Gets an undebayered frame from MLV file */
void MLVReaderGetFrame( MLVReader_t * Reader,
                        uint64_t FrameIndex,
                        MLVDataSource_t * DataSource,
                        void * DecodingMemory,
                        void * Out )
{
    // MLVReader_block_info_t * frames = Reader->blocks;

    int bytes_read = MLVReaderGetBlockData(Reader, DataSource, "VIDF", FrameIndex, Reader->biggest_video_frame, DecodingMemory);

    return;
}

/****************************** Metadata getters ******************************/

int MLVReaderGetFrameWidth(MLVReader_t * Reader)
{
    return Reader->RAWI.block.xRes;
}

int MLVReaderGetFrameHeight(MLVReader_t * Reader)
{
    return Reader->RAWI.block.yRes;
}

int MLVReaderGetBlackLevel(MLVReader_t * Reader)
{
    return Reader->RAWI.block.raw_info.black_level;
}

int MLVReaderGetWhiteLevel(MLVReader_t * Reader)
{
    return Reader->RAWI.block.raw_info.white_level;
}

int MLVReaderGetBitdepth(MLVReader_t * Reader)
{
    return Reader->RAWI.block.raw_info.bits_per_pixel;
}

// /* Which pixel bayer pattern starts at in the top left corner (RGGB, 0-3) */
// int MLVReaderGetBayerPixel(MLVReader_t * Reader)
// {
//     return 0;
// }

/* Returns two ints to Out */
int MLVReaderGetPixelAspectRatio(MLVReader_t * Reader, int * Out)
{
    /* TODO: figure out the rawc block */
    // if (Reader->RAWC.num_blocks != 0)
    // {
    //     /* code */
    // }
    // else
    {
        /* TODO: detect aspect on old 3x5 clips before RAWC */
        Out[0] = 1;
        Out[1] = 1;
    }
}

/* FPS value */
double MLVReaderGetFPS(MLVReader_t * Reader);

/* Get top and bottom of the FPS fraction */
int32_t MLVReaderGetFPSNumerator(MLVReader_t * Reader)
{
    return Reader->MLVI.block.sourceFpsNom;
}

int32_t MLVReaderGetFPSDenominator(MLVReader_t * Reader)
{
    return Reader->MLVI.block.sourceFpsDenom;
}

void MLVReaderGetCameraName(MLVReader_t * Reader, char * Out)
{
    strcpy(Out, (char *)Reader->IDNT.block.cameraName);
}

void MLVReaderGetLensName(MLVReader_t * Reader, char * Out)
{
    if (Reader->LENS.block.lensName[0] != 0)
        strcpy(Out, (char *)Reader->LENS.block.lensName);
    else
        strcpy(Out, "No electronic lens");
}

int MLVReaderGetLensFocalLength(MLVReader_t * Reader)
{
    return Reader->LENS.block.focalLength;
}

int MLVReaderGetISO(MLVReader_t * Reader, uint64_t FrameIndex)
{
    if (Reader->num_expo_blocks == 0) return MLVReader_ERROR_DATA_NOT_AVAILABLE;
    if (FrameIndex >= Reader->num_video_frames) return MLVReader_ERROR_BAD_INPUT;

    MLVReader_block_info_t * frame = Reader->blocks + FrameIndex;
    uint64_t frame_timestamp = frame->time_stamp;

    MLVReader_block_info_t * expo_blocks = Reader->blocks + (Reader->num_video_frames + Reader->num_audio_frames);
    // printf("e blocks = %i, %.4s, %i, %i\n", Reader->num_expo_blocks, expo_blocks, Reader->num_video_frames, Reader->num_audio_frames);

    for (int b = 0; b < Reader->num_expo_blocks-1; ++b)
    {
        if ( expo_blocks[ b ].time_stamp < frame_timestamp
          && expo_blocks[b+1].time_stamp > frame_timestamp )
        {
            puts("success!");
            return expo_blocks[b].expo.iso;
        }
    }

    return expo_blocks[Reader->num_expo_blocks-1].expo.iso;
}

void MLVReaderPrintAllBlocks(MLVReader_t * Reader)
{
    for (uint64_t i = 0; i < Reader->num_blocks; ++i)
    {
        printf("%lu: %.4s\n", i, Reader->blocks[i].block_type);
    }
}