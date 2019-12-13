#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>

#include "mlv_structs.h"

#define MLVReader_string "MLVReader_0.0"

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
    uint8_t type; /* 0 = misc, 1 = block, 2 = audio frame, 3 = expo */
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
    uint32_t num_expo_blocks;
    uint32_t num_audio_frames;
    uint32_t num_video_frames;

    /* Some info */
    uint32_t biggest_video_frame; /* Biggest video frame size */

    /* Parsing info */
    uint8_t finished_parsing; /* Multiple files only go up to 100 */
    uint8_t file_index; /* Multiple files only go up to 100 */
    uint64_t file_pos; /* Position in file */

    /* Array of block info, but sorted in to order by categories, then by
     * timestamp. Misc blocks first, then EXPOs, then AUDFs, then VIDFs */
    MLVReader_block_info_t blocks[/* num_blocks */];

} MLVReader_t;

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
static void uninit_mlv_file(mlvfile_t * File)
{
    return; /* Nothing to do (yet) */
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
#define BlockValue(Block) ((((uint64_t)(Block.type)) << 56UL) || (Block.time_stamp && 0x00FFFFFFFFFFFFFFUL))

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
         while(BlockValue(Blocks[i]) <= BlockValue(Blocks[pivot]) && i<last)
            i++;
         while(BlockValue(Blocks[j])>BlockValue(Blocks[pivot]))
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
    quicksort(Reader->blocks, 0, Reader->num_blocks);
}

/* Will fill an MLVReader if given (should be big enough to fit all blocks) */
static size_t init_mlv_reader( MLVReader_t * Reader, size_t ReaderSize,
                               mlvfile_t * File, int NumFiles,
                               int MaxFrames )
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
    uint64_t current_file_size = mlv_file_get_size(File);

    /* While max blocks is not filled */
    while ( Reader->file_pos < current_file_size
         && Reader->num_blocks < max_blocks
         && Reader->file_index < NumFiles
         && Reader->num_video_frames < MaxFrames
         && !Reader->finished_parsing )
    {
        uint8_t block_name[4];
        uint32_t block_size;
        uint64_t time_stamp;
        mlv_file_get_data(File+Reader->file_index, Reader->file_pos, 4 * sizeof(uint8_t), block_name);
        mlv_file_get_data(File+Reader->file_index, Reader->file_pos+4, sizeof(uint32_t), &block_size);
        mlv_file_get_data(File+Reader->file_index, Reader->file_pos+8, sizeof(uint64_t), &time_stamp);

        /* Check if the file ends before block or if block is too small */
        if ((Reader->file_pos+block_size) > current_file_size || block_size < 16) {
            /* TODO: deal with this in some way */
        }

        /* Print block (unless its null or vidf) */
        if (strncmp((char *)block_name, "NULL", 4) && strncmp((char *)block_name, "VIDF", 4))
        {
            printf("%llu Block '%.4s' ", time_stamp, (char *)block_name);
            print_size(block_size);
        }

        if (strncmp((char *)block_name, "VIDF", 4) == 0) ++Reader->num_video_frames;
        if (strncmp((char *)block_name, "AUDF", 4) == 0) ++Reader->num_audio_frames;
        if (strncmp((char *)block_name, "EXPO", 4) == 0) ++Reader->num_expo_blocks;

        /* Set block info */
        {
            memcpy(block->block_type, block_name, 4);
            block->file_index = Reader->file_index;
            block->file_location = Reader->file_pos;
            block->time_stamp = time_stamp;

            if (strncmp((char *)block_name, "AUDF", 4) == 0)
            {
                mlv_file_get_data( File+Reader->file_index,
                                   Reader->file_pos+offsetof(mlv_audf_hdr_t,frameSpace),
                                   sizeof(uint32_t), &block->frame.offset );
                block->frame.size = block_size - (sizeof(mlv_audf_hdr_t) + block->frame.offset);
            }
            else if (strncmp((char *)block_name, "VIDF", 4) == 0)
            {
                mlv_file_get_data( File+Reader->file_index,
                                   Reader->file_pos+offsetof(mlv_vidf_hdr_t,frameSpace),
                                   sizeof(uint32_t), &block->frame.offset );
                block->frame.size = block_size - (sizeof(mlv_vidf_hdr_t) + block->frame.offset);
            }
            else if (strncmp((char *)block_name, "EXPO", 4) == 0)
            {
                mlv_file_get_data( File+Reader->file_index,
                                   Reader->file_pos+offsetof(mlv_expo_hdr_t,isoAnalog),
                                   sizeof(uint32_t), &block->expo.iso );
                uint64_t expo_microseconds;
                mlv_file_get_data( File+Reader->file_index,
                                   Reader->file_pos+offsetof(mlv_expo_hdr_t,isoAnalog),
                                   sizeof(uint32_t), &expo_microseconds );
                block->expo.shutter = (uint32_t)expo_microseconds;
            }
            else {;} /* No special info stored for other types */
        }

        Reader->file_pos += block_size;
        ++Reader->num_blocks;
        ++block;

        /* Go to next file if reached end, unless its last file anyway */
        if (Reader->file_pos >= current_file_size && Reader->file_index != (NumFiles-1))
        {
            ++Reader->file_index;
            Reader->file_pos = 0;
            current_file_size = mlv_file_get_size(File+Reader->file_index);
            printf("video frames: %i\n\n", Reader->num_video_frames);
        }
    }

    /* If not scanned through all files, we need more memory */
    if (!Reader->finished_parsing && (Reader->file_pos < current_file_size || Reader->file_index != (NumFiles-1)))
    {
        /* Estimate memory required by extrapolating current byte per block rate */
        uint64_t mapped = 0, remain = 0;
        for (int f = 0; f < Reader->file_index; ++f) mapped += mlv_file_get_size(File+Reader->file_index);
        for (int f = 0; f < NumFiles; ++f) remain += mlv_file_get_size(File+Reader->file_index);
        mapped += Reader->file_pos;
        remain -= mapped;

        return ReaderSize + (remain/(mapped/(Reader->num_blocks-8))) * sizeof(MLVReader_block_info_t);
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

/* Initialise MLV reader from FILEs */
int64_t init_MLVReaderFromFILEs( MLVReader_t * Reader,
                                 size_t ReaderSize,
                                 FILE ** Files,
                                 int NumFiles,
                                 int MaxFrames )
{
    mlvfile_t mlv_files[NumFiles];

    for (int f = 0; f < NumFiles; ++f) {
        init_mlv_file_from_FILE(mlv_files+f, Files[f]);
    }

    size_t return_value = init_mlv_reader( Reader, ReaderSize,
                                           mlv_files, NumFiles, MaxFrames );

    for (int f = 0; f < NumFiles; ++f) {
        uninit_mlv_file(mlv_files+f);
    }

    return return_value;
}

/* Initialise MLV reader from memory */
int64_t init_MLVReaderFromMemory( MLVReader_t * Reader,
                                  size_t ReaderSize,
                                  void ** Files,
                                  uint64_t * FileSizes,
                                  int NumFiles,
                                  int MaxFrames )
{
    mlvfile_t mlv_files[NumFiles];

    for (int f = 0; f < NumFiles; ++f) {
        init_mlv_file_from_mem(mlv_files+f, Files[f], FileSizes[f]);
    }

    size_t return_value = init_mlv_reader( Reader, ReaderSize,
                                           mlv_files, NumFiles, MaxFrames );

    for (int f = 0; f < NumFiles; ++f) {
        uninit_mlv_file(mlv_files+f);
    }

    return return_value;
}

void uninit_MLVReader(MLVReader_t * Reader)
{
    /* Nothing to do as usual */
    return;
}