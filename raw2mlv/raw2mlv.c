/*
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 * 
 * For more information, please refer to <http://unlicense.org/>
 */

/*
 * The code in this file is public domain, but LibRaw is still LGPL.
 * This is a demo writing an MLV with LibMLV's MLVWriter.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../LibMLV/LibMLV.h"

#include "librawheaders/libraw.h"

void print_help()
{
    puts(
"Arguments:\n"
" -h                    Print help\n"
" -o <output filename>  Output file name\n"
" -b <bitdepth>         Output bitdepth, 8 to 16, even numbers\n"
" -c <0/1>              Output compression, 0=none, 1=LJ92\n"
" -f <top> <bottom>     Framerate as a fraction, ex: -f 24000 1001\n"
"Example:\n"
" ./write_mlv pic1.raw pic2.raw pic3.raw -o myvid.mlv -b 12 -c 1\n");
}

int main(int argc, char ** argv)
{
    /* Output parameters */
    char * output_name = "output.mlv";
    int num_input_files = 0;
    char ** input_files = alloca(argc * sizeof(char *));
    int output_bits = 14;
    int output_compression = 0;
    int width = 0;
    int height = 0;
    int output_fps_top = 24000;
    int output_fps_bottom = 1001;

    /* Source data info */
    int source_bitdepth = 14; /* Will be figured out later */

    /* Parse arguments */
    for (int i = 1; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-h")) {
            print_help();
            exit(0);
        } else if (!strcmp(argv[i], "-b")) {
            output_bits = atoi(argv[i+1]);
            printf("Bitdepth set to %i\n", output_bits);
            if ((output_bits % 2) != 0 || output_bits>16 || output_bits<10) {
                puts("Unsupported bitdepth, choose 10/12/14/16.");
                exit(1);
            }
            ++i;
        } else if (!strcmp(argv[i], "-o")) {
            output_name = argv[i+1];
            printf("Output name set to %s\n", output_name);
            ++i;
        } else if (!strcmp(argv[i], "-c")) {
            output_compression = atoi(argv[i+1]);
            char * compression = (output_compression) ? "none" : "LJ92";
            printf("Compression set to %i (none)\n", output_compression);
            ++i;
        } else if (!strcmp(argv[i], "-f")) {
            output_fps_top = atoi(argv[i+1]);
            output_fps_bottom = atoi(argv[i+2]);
            printf("FPS set to %.3f\n",(float)output_fps_top/output_fps_bottom);
            i += 2;
        } else {
            input_files[num_input_files++] = argv[i];
        }
    }

    /****************************** MLV creation ******************************/

    /* Allocate memory for the mlv writer object */
    MLVWriter_t * writer = alloca(sizeof_MLVWriter());

    /* Open file and create pointer for packed frame data */
    FILE * mlv_file = fopen(output_name, "wb");
    uint8_t * packed_frame_data = NULL;

    /* Write each frame */
    for (int f = 0; f < num_input_files; ++f)
    {
        /* Open raw file with libraw */
        libraw_data_t * Raw = libraw_init(0);
        printf("Opening file %s\n", input_files[f]);
        if (libraw_open_file(Raw, input_files[f])) puts("failed to open file");
        if (libraw_unpack(Raw)) puts("failed to unpack");

        /* This is the bayer data */
        uint16_t * bayerimage = Raw->rawdata.raw_image;

        /* Initialise MLV writer and write MLV headers when it's first frame */
        if (i == 0)
        {
            /* Round bitdepth up to a multiple of 2 */
            source_bitdepth = (int)ceil(log2(Raw->rawdata.color.maximum)/2) * 2;

            printf("Detected source bitdepth: %i\n", source_bitdepth);

            /*************************** Set values ***************************/

            width = libraw_get_raw_width(Raw);
            height = libraw_get_raw_height(Raw);

            packed_frame_data = malloc((width * height * output_bits) / 8);

            /********************* Initialise MLV writer **********************/

            /* Set basic image parameters */
            init_MLVWriter( writer,
                            width, /* Width */
                            height, /* Height */
                            output_bits, /* Bitdepth */
                            output_compression, /* Compressed LJ92? */
                            Raw->rawdata.color.black, /* Black levbel */
                            Raw->rawdata.color.maximum, /* or data_maximum? */
                            output_fps_top, /* FPS fraction */
                            output_fps_bottom );

            /*********************** Set camera info... ***********************/

            char cam_name[32];
            snprintf(cam_name, 32, "%s %s", Raw->idata.make, Raw->idata.model);

            double matrix[9];
            for (int y = 0; y < 3; ++y) {
                for (int x = 0; x < 3; ++x) {
                    printf("%3.5f, ", Raw->rawdata.color.cam_xyz[y][x]);
                    matrix[y*3+x] = Raw->rawdata.color.cam_xyz[y][x];
                }
                printf("\n");
            }

            MLVWriterSetCameraInfo(writer, cam_name, 0, matrix);

            /************************** Write headers *************************/

            /* After setting all the metadata we choose to set, MLVWriter tells
             * us how much header data there is, gives it to us, and then we
             * must write it to a file ourself */
            size_t header_size = MLVWriterGetHeaderSize(writer);

            {
                /* Allocate array for header data */
                uint8_t header_data[header_size];

                /* Get header data */
                MLVWriterGetHeaderData(writer, header_data);

                /* Write header data to the file */
                fwrite(header_data, header_size, 1, mlv_file);
            }
        }
        else
        {
            /* Make sure everything is right */
            if ( libraw_get_raw_width(Raw) != width
             || libraw_get_raw_height(Raw) != height)
            {
                printf("File %s has different resolution!\n", input_files[f]);
            }
        }

        /* Calculate frame size, TODO: compressed frames will be different */
        size_t frame_size = (width * height * output_bits) / 8;

        /*************************** Write the frame **************************/

        /* Get frame header size, telling MLVWriter how big the frame is */
        size_t frame_header_size = MLVWriterGetFrameHeaderSize(writer);

        {
            /* Create memory for frame header */
            uint8_t frame_header_data[frame_header_size];

            /* Get frame header */
            MLVWriterGetFrameHeaderData(writer,f,frame_size,frame_header_data);

            /* Write it */
            fwrite(frame_header_data, frame_header_size, 1, mlv_file);
        }

        /* Now write actual frame data */
        if (output_bits == 16)
            fwrite(bayerimage, frame_size, 1, mlv_file);
        else 
        {
            if (output_bits == 14)
                MLVPackFrame14(bayerimage, width*height, packed_frame_data);
            else if (output_bits == 12)
                MLVPackFrame12(bayerimage, width*height, packed_frame_data);
            else if (output_bits == 10)
                MLVPackFrame10(bayerimage, width*height, packed_frame_data);

            fwrite(packed_frame_data, frame_size, 1, mlv_file);
        }

        libraw_recycle(Raw);
        libraw_close(Raw);
    }

    if (packed_frame_data != NULL) free(packed_frame_data);
    fclose(mlv_file);
    uninit_MLVWriter(writer);

    return 0;
}