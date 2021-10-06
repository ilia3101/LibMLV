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
 * This is a demo of writing an MLV with LibMLV's MLVWriter.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Thank You Microsoft, Very Cool! */
#ifdef _WIN32
#define WIN32
#endif

#include "../../include/LibMLV.h"

/* Camid, for getting model ID for magic lantern cameras */
#include "camid.c"

/* LibRaw wrapper */
#include "read_raw.c"

/* Camera matrix data */
#include "camera_matrices.c"

void print_help()
{
    puts(
"Arguments:\n"
" -h, --help                      Print help\n"
" -o <output filename>            Output file name\n"
" -b, --bitdepth <bitdepth>       Output bitdepth, 8 to 16, even numbers\n"
// " --compression <0/1>             Output compression, 0=none, 1=LJ92\n"
" -f, --framerate <top> <bottom>  Framerate as a fraction, ex: -f 24000 1001\n"
// " --crop <left>              Crop\n"
"Example:\n"
#ifdef WIN32
" .\\raw2mlv"
#else
" ./raw2mlv"
#endif
" pic1.raw pic2.raw pic3.raw -o myvid.mlv --bitdepth 12\n");
}

int limit_multiple_of_8(int value)
{
    if ((value % 8) == 0) return value;
    else return value - (value % 8);
}

void do_binning(uint16_t * unbinned_image, uint16_t * binned_image, int binning_x, int binning_y, int width, int height)
{
    if (binning_x == 1 && binning_y == 1)
        memcpy(binned_image, unbinned_image, width*height*sizeof(uint16_t));
    else {
        int divide_by = binning_x * binning_y;
        int output_height = height/binning_y;
        int output_width = limit_multiple_of_8(width/binning_x);
        for (int y = 0, y_src = 0; y < output_height; ++y, y_src += binning_y)
        {
            for (int x = 0, x_src = 0; x < output_width; ++x, x_src += binning_x)
            {
                uint32_t pixel_value = 0;

                for (int y2 = 0; y2 < binning_y; ++y2)
                {
                    for (int x2 = 0; x2 < binning_x; ++x2)
                    {
                        pixel_value += unbinned_image[(y_src+y2*2) * width + x_src + x2*2];
                    }
                }

                pixel_value /= divide_by;

                binned_image[y * output_width + x] = pixel_value;
            }
        }
    }
}

int main(int argc, char ** argv)
{
    /* Output parameters */
    char * output_name = "output.mlv";
    int num_input_files = 0;
    char ** input_files = malloc(argc * sizeof(char *));
    int output_bits = 0;
    int output_compression = 0;
    int width = 0;
    int height = 0;
    int output_fps_top = 24000;
    int output_fps_bottom = 1001;
    int binning = 1;
    char * camera_name = NULL;
    char * camera_make = NULL;
    char * camera_model = NULL;

    /* Source data info */
    int source_bitdepth = 14; /* Will be figured out later */
    int shift_bits = 0; /* How many bits to shift to get to output bitdepth */

    /* Count how many frames have been written */
    int written_frames = 0;

    /* Parse arguments */
    for (int i = 1; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            print_help();
            exit(0);
        } else if (!strcmp(argv[i], "-b") || !strcmp(argv[i], "--bitdepth")) {
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
        } else if (!strcmp(argv[i], "--compression")) {
            output_compression = atoi(argv[i+1]);
            char * compression = (output_compression) ? "none" : "LJ92";
            printf("Compression set to %i (none)\n", output_compression);
            ++i;
        } else if (!strcmp(argv[i], "--camera-name")) {
            camera_name = malloc(strlen(argv[i+1]) + strlen(argv[i+2]) + 10);
            camera_make = argv[i+1]; camera_model = argv[i+2];
            sprintf(camera_name, "%s %s", camera_make, camera_model);
            printf("Camera name set to %i\n", camera_name);
            ++i;
            ++i;
        } else if (!strcmp(argv[i], "-f") || !strcmp(argv[i], "--framerate")) {
            output_fps_top = atoi(argv[i+1]);
            output_fps_bottom = atoi(argv[i+2]);
            printf("FPS set to %.3f\n",(float)output_fps_top/output_fps_bottom);
            i += 2;
        } else if (!strcmp(argv[i], "--binning")) { /* Secret bonus option */
            binning = atoi(argv[i+1]);
            /* if (binning != 3 && binning != 1 && binning != 5) binning = 1;
            else */ printf("Binning set to %i\n", binning);
            ++i;
        } else {
            input_files[num_input_files++] = argv[i];
        }
    }

    /****************************** MLV creation ******************************/

    /* Allocate memory for the mlv writer object */
    MLVWriter_t * writer = malloc(sizeof_MLVWriter());

    /* Open file and create pointer for packed frame data */
    FILE * mlv_file = fopen(output_name, "wb");
    uint8_t * packed_frame_data = NULL;

    /* Write each frame */
    for (int f = 0; f < num_input_files; ++f)
    {
        /* Open raw file with RawReader (wrapper for LibRaw) */
        RawReader_t raw[1];
        if (init_RawReader(raw, input_files[f]) != 0)
            printf("Can't read file %s\n", input_files[f]);

        /* Initialise MLV writer and write MLV headers when it's first frame */
        if (f == 0)
        {
            /* Round bitdepth up to a multiple of 2 */
            #define MAX(a,b) (((a)>(b))?(a):(b))
            source_bitdepth = (int)ceil(log2(MAX(RawGetMaxPixelValue(raw), RawGetWhiteLevel(raw)))/2) * 2;
            /* Set output bitdepth to be same as input if user has not specified anything */
            if (output_bits == 0) output_bits = source_bitdepth;
            shift_bits = source_bitdepth - output_bits;
            if (shift_bits < 0) shift_bits = -shift_bits;
            float lscale = pow(2.0, output_bits - source_bitdepth);

            printf("Detected source bitdepth: %i\n", source_bitdepth);

            /*************************** Set values ***************************/

            width = limit_multiple_of_8(RawGetWidth(raw) / binning);
            height = RawGetHeight(raw) / binning;

            packed_frame_data = malloc((width * height * output_bits) / 8);

            /********************* Initialise MLV writer **********************/

            /* Set basic image parameters */
            init_MLVWriter( writer,
                            width, /* Width */
                            height, /* Height */
                            output_bits, /* Bitdepth */
                            output_compression, /* Compressed LJ92? */
                            RawGetBlackLevel(raw)*lscale, /* Black level */
                            RawGetWhiteLevel(raw)*lscale, /* White level */
                            output_fps_top, /* FPS fraction */
                            output_fps_bottom );

            /*********************** Set camera info... ***********************/

            /* Try to look up adobe matrix */
            CameraMatrixInfo_t * mat;
            if (camera_name == NULL) mat = FindCameraMatrixInfo(RawGetCamName(raw));
            else mat = FindCameraMatrixInfo(camera_name);
            double * camera_matrix = NULL;

            /* If no matrix found from adobe, use libraw one (most likely the same) */
            if (mat != NULL){puts("marix");
                camera_matrix = mat->ColorMatrix2;}
            else{puts("no marix");
                camera_matrix = RawGetMatrix(raw);}

            MLVWriterSetCameraInfo( writer,
                                    (camera_name) ? camera_name : RawGetCamName(raw), /* Camera name string */
                                    camidGetCameraModel(RawGetCamName(raw)), /* Model ID (not necessary for an MLV to be valid) */
                                    camera_matrix /* Daylight camera matrix */);

            /************** Leave blank space in file for headers *************/

            fseek(mlv_file, MLVWriterGetHeaderSize(writer), SEEK_SET);
        }
        else
        {
            /* Make sure everything is right */
            if ( limit_multiple_of_8(RawGetWidth(raw)/binning) != width
             || RawGetHeight(raw)/binning != height)
            {
                printf("File %s has different resolution!\n", input_files[f]);
            }
        }

        /* Calculate frame size, TODO: compressed frames will be different */
        size_t frame_size = (width * height * output_bits) / 8;

        /*************************** Write the frame **************************/

        /* Get frame header size, telling MLVWriter how big the frame is */
        size_t frame_header_size = MLVWriterGetFrameHeaderSize(writer);

        /* Create memory for frame header */
        void * frame_header_data = malloc(frame_header_size);

        /* Get frame header */
        MLVWriterGetFrameHeaderData(writer,f,frame_size,frame_header_data);

        /* Write it */
        fwrite(frame_header_data, frame_header_size, 1, mlv_file);

        free(frame_header_data);

        /* Now write actual frame data */
        uint16_t * unbinned_image = RawGetImageData(raw);

        uint16_t * bayerimage = malloc(width * height * sizeof(uint16_t));

        do_binning(unbinned_image, bayerimage, binning, binning, RawGetWidth(raw), RawGetHeight(raw));

        if (output_bits < source_bitdepth)
            for (int i = 0; i < width*height; ++i) bayerimage[i] >>= shift_bits;
        else
            for (int i = 0; i < width*height; ++i) bayerimage[i] <<= shift_bits;

        if (output_bits == 16)
            packed_frame_data = (void *)bayerimage;
        else if (output_bits == 14)
            MLVPackFrame14(bayerimage, width*height, packed_frame_data);
        else if (output_bits == 12)
            MLVPackFrame12(bayerimage, width*height, packed_frame_data);
        else if (output_bits == 10)
            MLVPackFrame10(bayerimage, width*height, packed_frame_data);

        fwrite(packed_frame_data, frame_size, 1, mlv_file);

        written_frames++;

        uninit_RawReader(raw);
    }


    /***************************** Write headers ******************************/

    /* Now go back to the start of the file */
    fseek(mlv_file, 0, SEEK_SET);

    /* Allocate array for header data */
    size_t header_size = MLVWriterGetHeaderSize(writer);
    void * header_data = malloc(header_size);

    /* Get header data */
    MLVWriterGetHeaderData(writer, header_data, written_frames);

    /* Write header data to the file */
    fwrite(header_data, header_size, 1, mlv_file);

    free(header_data);

    /********************************** DONE **********************************/

    if (packed_frame_data != NULL) free(packed_frame_data);
    fclose(mlv_file);
    uninit_MLVWriter(writer);
    free(writer);
    free(input_files);

    return 0;
}
