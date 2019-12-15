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
    FILE * mlv_files[argc-1];
    
    for (int f = 1; f < argc; ++f)
    {
        mlv_files[f-1] = fopen(argv[f], "r");
        if (mlv_files[f-1] == NULL) printf("Failed to open file %s\n", argv[f]);
        puts(argv[f]);
    }

    size_t allocated_size = 22;
    MLVReader_t * reader = malloc(allocated_size);

    size_t return_val = allocated_size;

    do {
        reader = realloc(reader, return_val);
        allocated_size = return_val;
        return_val = init_MLVReaderFromFILEs(reader, allocated_size, mlv_files, argc-1, 0);
        printf("Allocated = %i, requested = %i\n", allocated_size, return_val);
    } while (return_val != allocated_size);

    for (int f = 1; f < argc; ++f)
    {
        fclose(mlv_files[f-1]);
    }

    return 0;
}