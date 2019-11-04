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

        size_t mlv_reader_size = sizeof_MLVReaderFromFILEs(&mlv_file, 1, 0);

        fclose(mlv_file);
    }

    for (int f = 1; f < argc; ++f)
    {
        puts(argv[f]);

        uint64_t mlvsize;

        void * mem_mlv = file2mem(argv[f], &mlvsize);

        sizeof_MLVReaderFromMemory(&mem_mlv, &mlvsize, 1, 0);

        free(mem_mlv);
    }

    return 0;
}