/* This file is GPLv2 or GPLv3 */

#include <stdio.h>
#include <stdlib.h>

#include "libmlv_write.h"

#include "librawheaders/libraw.h"

void writebmp(unsigned char * data, int width, int height, char * filename) {
    int rowbytes = width*3+(4-(width*3%4))%4, imagesize = rowbytes*height, y;
    unsigned short header[] = {0x4D42,0,0,0,0,26,0,12,0,width,height,1,24};
    *((unsigned int *)(header+1)) = 26 + imagesize-((4-(width*3%4))%4);
    FILE * file = fopen(filename, "wb");
    fwrite(header, 1, 26, file);
    for (y = 0; y < height; ++y) fwrite(data+(y*width*3), rowbytes, 1, file);
    fwrite(data, width*3, 1, file);
    fclose(file);
}

void print_help()
{
    puts("Arguments:");
    puts(" -h                       Print help");
    puts(" -o <output filename>     Output file name");
    puts(" -b <bitdepth>            Output bitdepth, 8 to 16, even numbers");
    puts(" -c <0/1>                 Output compression, 0=none, 1=LJ92");
}

int main(int argc, char ** argv)
{
    char * output_name = "output.mlv";
    int num_input_files = 0;
    char ** input_files = alloca(argc * sizeof(char *));
    int output_bitdepth = 14;
    int output_compression = 0;
    int width = 0;
    int height = 0;

    /* Parse arguments */
    for (int i = 1; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-h"))
        {
            print_help();
            exit(0);
        }
        else if (!strcmp(argv[i], "-o"))
        {
            output_name = argv[i+1];
            ++i;
        }
        else if (!strcmp(argv[i], "-o"))
        {
            output_name = argv[i+1];
            ++i;
        }
        else
        {
            input_files[num_input_files] = argv[i];
            num_input_files++;
        }
    }

    /****************************** MLV creation ******************************/

    MLVWriter_t * writer = alloca(sizeof_MLVWriter());

    /* Write each frame */
    for (int i = 0; i < num_input_files; ++i)
    {
        /* Get raw files */
        libraw_data_t * Raw = libraw_init(0);
        if (libraw_open_file(Raw, argv[1])) puts("failed to open file");
        if (libraw_unpack(Raw)) puts("failed to unpack");

        /* This is the bayer data */
        uint16_t * bayerimage = Raw->rawdata.raw_image;
        width = libraw_get_iwidth(Raw);
        height = libraw_get_iheight(Raw);

        /* Initialise MLV writer on the first frame */
        if (i == 0)
        {
            init_MLVWriter( writer,
                            width, /* Width */
                            height, /* Height */
                            output_bitdepth, /* Bitdepth */
                            output_compression, /* Compressed LJ92? */
                            Raw->imgdata.color.black, /* Black levbel */
                            Raw->imgdata.color.maximum /* or maybe data_maximum */ /* White level */ );

            MLVWriterSetCameraInfo();
        }

        // /* Subtract black or whatever */
        // libraw_raw2image(Raw);
        // libraw_subtract_black(Raw);

        libraw_recycle(Raw);
        libraw_close(Raw);
    }

    MLVWriterSetCameraPreset(writer, Canon_5D_Mark_II);

    uninit_MLVWriter(writer);

    return 0;
}