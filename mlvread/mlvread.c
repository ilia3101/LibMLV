#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

/* For mmap */
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

#include "../include/LibMLV.h"

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

/* MLV App 2017 throwback */
void printMlvInfo(MLVReader_t * video)
{
    printf("\nMLV Info\n\n");
    // printf("      MLV Version: %s\n", video->MLVI.versionString);
    printf("      File Blocks: %lu\n", MLVReaderGetNumBlocks(video));
    printf("\nLens Info\n\n");
    char lensname[32], camname[32];
    MLVReaderGetCameraName(video, camname);
    MLVReaderGetLensName(video, lensname);
    printf("       Lens Model: %s\n", lensname);
    // printf("    Serial Number: %s\n", video->LENS.lensSerial);
    printf("\nCamera Info\n\n");
    printf("     Camera Model: %s\n", camname);
    // printf("    Serial Number: %s\n", video->IDNT.cameraSerial);
    printf("\nVideo Info\n\n");
    printf("     X Resolution: %i\n", MLVReaderGetFrameWidth(video));
    printf("     Y Resolution: %i\n", MLVReaderGetFrameHeight(video));
    printf("     Total Frames: %i\n", MLVReaderGetNumBlocksOfType(video, "VIDF"));
    printf("       Frame Rate: %.3f\n", (double)MLVReaderGetFPSNumerator(video)/MLVReaderGetFPSDenominator(video));
    printf("\nExposure Info\n\n");
    // printf("          Shutter: 1/%.1f\n", (float)1000000 / (float)video->EXPO.shutterValue);
    printf("      ISO Setting: %i\n", MLVReaderGetISO(video, 0));
    // printf("     Digital Gain: %i\n", video->EXPO.digitalGain);
    printf("\nRAW Info\n\n");
    printf("      Black Level: %i\n", MLVReaderGetBlackLevel(video));
    printf("      White Level: %i\n", MLVReaderGetWhiteLevel(video));
    printf("     Bits / Pixel: %i\n\n", MLVReaderGetBitdepth(video));
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

    printMlvInfo(reader);

    for (int f = 1; f < argc; ++f)
    {
        fclose(mlv_files[f-1]);
    }

    return 0;
}