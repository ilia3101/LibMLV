#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>

#include "mlv_structs.h"

#define MLVReader_header_block(BlockType, BlockName) \
struct \
{ \
    /* Store only one */ \
    union { \
        BlockType block; \
        mlv_hdr_t header; \
    }; \
    uint8_t exists; /* Was this block found in a file */ \
    uint8_t file_index; /* Which file (for split up MLVs - M00... M99) */ \
    uint16_t empty_space; \
    uint32_t num_blocks; /* How many of this block was found */ \
    uint64_t file_location; \
} BlockName;

/* This structure has some unused bytes */
typedef struct
{
    uint8_t block_type[4]; /* Block name string */
    uint8_t file_index; /* Which clip it is in (for split up files M00...M99) */
    uint64_t file_location; /* File location of block */
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

typedef struct
{
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

    /* Frame blocks */
    mlv_vidf_hdr_t VIDF;
    mlv_audf_hdr_t AUDF;

    /* How many of what block there is */
    uint32_t num_misc_blocks;
    uint32_t num_expo_blocks;
    uint32_t num_audio_frames;
    uint32_t num_video_frames;

    /* Array of block info, but sorted in to order by categories, then by
     * timestamp. Misc blocks first, then EXPOs, then AUDFs, then VIDFs */
    MLVReader_block_info_t blocks[];

} MLVReader_t;

int a234sdg243df52gadf5245ghs = sizeof(MLVReader_block_info_t);
int asd425jhjkhjkhk2345432n23 = sizeof(MLVReader_t);

#define MLVReader_src
#include "MLVReader.h"
#undef MLVReader_src

/***** Wrapper for memory or a FILE, so they can be treated the same way ******/
typedef struct {
    int file_or_memory; /* 0 = file, 1 = memory */
    uint64_t size;
    union {
        uint8_t * mem;
        FILE * file;
    };
} mlvfile_t;
static void init_mlv_file_from_FILE(mlvfile_t * File, FILE * FileObject)
{
    File->file_or_memory = 0;
    File->file = FileObject;
    fseek(FileObject, 0, SEEK_END);
    File->size = ftell(FileObject);
}
static void init_mlv_file_from_mem(mlvfile_t * File, void * Mem, size_t Size)
{
    File->file_or_memory = 1;
    File->size = Size;
    File->mem = Mem;
}
static void mlv_file_get_data(mlvfile_t*F,uint64_t Pos,uint64_t Size,void*Out)
{
    if (F->file_or_memory == 1) memcpy(Out, F->mem + Pos, Size);
    else { fseek(F->file, Pos, SEEK_SET); fread(Out, 1, Size, F->file); }
}
static uint64_t mlv_file_get_size(mlvfile_t * File)
{
    return File->size;
}
/******************************************************************************/

/* static void print_size(uint64_t Size)
{
    if (Size < 1024)
        printf("%i Bytes\n", (int)Size);
    else if (Size < 1024*1024)
        printf("%.1f KiB\n", ((float)Size)/1024.0f);
    else if (Size < 1024*1024*1024)
        printf("%.2f MiB\n", ((float)Size)/(1024.0f*1024.0f));
    else
        printf("%.2f GiB\n", ((float)Size)/((float)(1024*1024*1024)));
} */

/* Will fill an MLVReader if given (should be big enough to fit all blocks) */
static int parse_mlv_file( mlvfile_t * File,
                           uint32_t * NumBlocksOut,
                           uint32_t * NumVideoFramesOut,
                           uint32_t * NumAudioFramesOut,
                           uint32_t * NumExpoBlocksOut,
                           MLVReader_block_info_t * BlockOutput,
                           int FileIndex )
{
    int return_value = 0;

    /* Counters */
    int num_video_frames = 0;
    int num_audio_frames = 0;
    int num_expo_blocks = 0;
    int num_blocks = 0;

    /* Get file size */
    uint64_t mlv_file_size = mlv_file_get_size(File);

    /* Go back to start */
    uint64_t file_pos = 0;

    while (file_pos < mlv_file_size)
    {
        uint8_t block_name[4];
        uint32_t block_size;
        uint64_t time_stamp;
        mlv_file_get_data(File, file_pos, 4*sizeof(uint8_t), block_name);
        mlv_file_get_data(File, file_pos+4, sizeof(uint32_t), &block_size);
        mlv_file_get_data(File, file_pos+8, sizeof(uint64_t), &time_stamp);

        /* Check if the size overruns file or is too small */
        if ((file_pos+block_size) > mlv_file_size || block_size < 16)
        {
            return_value = 1;
            break;
        }

        /* Print block (unless its null) */
        /* if (strncmp((char *)block_name, "NULL", 4)) {
            printf("%llu Block '%.4s' ", time_stamp, (char *)block_name);
            print_size(block_size);
        } */

        if (strncmp((char *)block_name, "VIDF", 4) == 0) ++num_video_frames;
        if (strncmp((char *)block_name, "AUDF", 4) == 0) ++num_audio_frames;
        if (strncmp((char *)block_name, "EXPO", 4) == 0) ++num_expo_blocks;

        /* Set block info if it exists */
        if (BlockOutput != NULL)
        {
            MLVReader_block_info_t * block = BlockOutput + num_blocks;
            memcpy(block->block_type, block_name, 4);
            block->file_index = FileIndex;
            block->file_location = file_pos;
            block->time_stamp = time_stamp;

            if (strncmp((char *)block_name, "AUDF", 4) == 0)
            {
                mlv_file_get_data( File,
                                   file_pos+offsetof(mlv_audf_hdr_t,frameSpace),
                                   sizeof(uint32_t),
                                   &block->frame.offset );
                block->frame.size = block_size - (sizeof(mlv_audf_hdr_t) + block->frame.offset);
            }
            else if (strncmp((char *)block_name, "VIDF", 4) == 0)
            {
                mlv_file_get_data( File,
                                   file_pos+offsetof(mlv_vidf_hdr_t,frameSpace),
                                   sizeof(uint32_t),
                                   &block->frame.offset );
                block->frame.size = block_size - (sizeof(mlv_vidf_hdr_t) + block->frame.offset);
            }
            else if (strncmp((char *)block_name, "EXPO", 4) == 0)
            {
                mlv_file_get_data( File,
                                   file_pos+offsetof(mlv_expo_hdr_t,isoAnalog),
                                   sizeof(uint32_t),
                                   &block->expo.iso );
                uint64_t expo_microseconds;
                mlv_file_get_data( File,
                                   file_pos+offsetof(mlv_expo_hdr_t,isoAnalog),
                                   sizeof(uint32_t),
                                   &expo_microseconds );
                block->expo.shutter = (uint32_t)expo_microseconds;
            }
            else
            {
                /* We don't store any specific data for any */
            }
        }

        file_pos += block_size;
        ++num_blocks;
    }

    if (NumBlocksOut != NULL) *NumBlocksOut += num_blocks;
    if (NumVideoFramesOut != NULL) *NumVideoFramesOut += num_video_frames;
    if (NumAudioFramesOut != NULL) *NumAudioFramesOut += num_audio_frames;
    if (NumExpoBlocksOut != NULL) *NumExpoBlocksOut += num_expo_blocks;

    return 0;
}

size_t sizeof_MLVReaderFromFILEs( FILE ** Files,
                                  int NumFiles,
                                  int MaxFrames )
{
    uint32_t num_blocks = 0;

    for (int f = 0; f < NumFiles; ++f)
    {
        mlvfile_t mlv_file;
        init_mlv_file_from_FILE(&mlv_file, Files[f]);

        if (parse_mlv_file(&mlv_file, &num_blocks, NULL, NULL, NULL, NULL, f))
        {
            return 0;
        }
    }

    return sizeof(MLVReader_t) + num_blocks * sizeof(MLVReader_block_info_t);
}

/* Read MLV files from memory (maybe useful for memory mapped files) */
size_t sizeof_MLVReaderFromMemory( void ** Files,
                                   uint64_t * FileSizes,
                                   int NumFiles,
                                   int MaxFrames )
{
    uint32_t num_blocks = 0;

    for (int f = 0; f < NumFiles; ++f)
    {
        mlvfile_t mlv_file;
        init_mlv_file_from_mem(&mlv_file, Files[f], FileSizes[f]);

        if (parse_mlv_file(&mlv_file, &num_blocks, NULL, NULL, NULL, NULL, f))
        {
            return 0;
        }
    }

    return sizeof(MLVReader_t) + num_blocks * sizeof(MLVReader_block_info_t);
}