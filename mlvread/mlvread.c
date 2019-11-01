#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

/* For mmap */
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

#include "../LibMLV/MLVReader.h"

/* Wrapper for memory or FILE, so they can be treated the same way */
typedef struct {
    int file_or_memory; /* 0 = file, 1 = memory */
    uint64_t size;
    union {
        uint8_t * mem;
        FILE * file;
    };
} mlv_file_t;
static void init_mlv_file_from_FILE(mlv_file_t * File, FILE * FileObject) {
    File->file_or_memory = 0;
    File->file = FileObject;
    fseek(FileObject, 0, SEEK_END);
    File->size = ftell(FileObject);
} static void init_mlv_file_from_mem(mlv_file_t * File, void * Mem, size_t Size) {
    File->file_or_memory = 1;
    File->size = Size;
    File->mem = Mem;
} static void mlv_file_get_data(mlv_file_t * File, uint64_t Pos, uint64_t Size, void * Out) {
    if (File->file_or_memory == 1) memcpy(Out, File->mem + Pos, Size);
    else { fseek(File->file, Pos, SEEK_SET); fread(Out, 1, Size, File->file); }
} static uint64_t mlv_file_get_size(mlv_file_t * File) {
    return File->size;
}

void print_size(uint64_t Size)
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

static int mlv_scan(mlv_file_t*File/* , uint64_t*NumFrames, uint64_t*NumFrames */)
{
    int return_value = 0;
    int num_video_frames = 0;
    int num_audio_frames = 0;
    int num_expo_frames = 0;
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

        /* See if the size overruns file or is too small */
        if ((file_pos+block_size) > mlv_file_size || block_size < 16)
        {
            return_value = 1;
            break;
        }

        if (strncmp((char *)block_name, "NULL", 4))
        {
            printf("%llu Block '%.4s' ", time_stamp, (char *)block_name);
            print_size(block_size);
        }

        file_pos += block_size;
        ++num_blocks;
    }

    return 0;
}

void * file2mem(char * FilePath, uint64_t * SizeOutput)
{
    FILE * file = fopen(FilePath, "r");
    fseek(file, 0, SEEK_END);
    uint64_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    void * memory = malloc(file_size);
    fread(memory, file_size, 1, file);
    fclose(file);
    *SizeOutput = file_size;
    return memory;
}

int main(int argc, char ** argv)
{
    for (int f = 1; f < argc; ++f)
    {
        FILE * mlv_file = fopen(argv[f], "r");

        puts(argv[f]);

        mlv_file_t mlvfile;
        init_mlv_file_from_FILE(&mlvfile, mlv_file);

        size_t mlv_reader_size = sizeof_MLVReaderFromFILEs(&mlv_file, 1, 0);

        mlv_scan(&mlvfile);

        fclose(mlv_file);
    }

    for (int f = 1; f < argc; ++f)
    {
        puts(argv[f]);

        uint64_t mlvsize;

        void * mem_mlv = file2mem(argv[f], &mlvsize);

        mlv_file_t mlvfile;
        init_mlv_file_from_mem(&mlvfile, mem_mlv, mlvsize);

        mlv_scan(&mlvfile/* , mlvsize */);

        free(mem_mlv);
    }

    return 0;
}