/* 
    LibRaw wrapper, makes raw2mlv code nicer to read
 */

#include "LibRaw/libraw/libraw.h"

typedef struct {
    libraw_data_t * libraw;
    double matrix[9];
    char cam_name[32];
} RawReader_t;

int init_RawReader(RawReader_t * Raw, char * File)
{
    memset(Raw, 0, sizeof(RawReader_t));

    libraw_data_t * libraw = libraw_init(0);
    printf("Opening file %s\n", File);
    if (libraw_open_file(libraw, File) != 0) {
        puts("failed to open file");
        return 1;
    } if (libraw_unpack(libraw) != 0) {
        puts("failed to unpack");
        return 1;
    }

    /* Get matrix */
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            printf("%3.5f, ", libraw->rawdata.color.cam_xyz[y][x]);
            Raw->matrix[y*3+x] = libraw->rawdata.color.cam_xyz[y][x];
        } printf("\n");
    }

    /* Get camera name */
    snprintf(Raw->cam_name, 32, "%s %s",libraw->idata.make,libraw->idata.model);

    Raw->libraw = libraw;
    return 0;
}

void uninit_RawReader(RawReader_t * Raw)
{
    libraw_recycle(Raw->libraw);
    libraw_close(Raw->libraw);
}

uint16_t * RawGetImageData(RawReader_t * Raw)
{
    return Raw->libraw->rawdata.raw_image;
}

int RawGetBlackLevel(RawReader_t * Raw)
{
    return Raw->libraw->rawdata.color.black;
}

int RawGetWhiteLevel(RawReader_t * Raw)
{
    return Raw->libraw->rawdata.color.maximum;
}

int RawGetWidth(RawReader_t * Raw)
{
    return libraw_get_raw_width(Raw->libraw);
}

int RawGetHeight(RawReader_t * Raw)
{
    return libraw_get_raw_height(Raw->libraw);
}

double * RawGetMatrix(RawReader_t * Raw)
{
    return Raw->matrix;
}

char * RawGetCamName(RawReader_t * Raw)
{
    return Raw->cam_name;
}