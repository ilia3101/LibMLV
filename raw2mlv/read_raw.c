/* 
    LibRaw wrapper, makes raw2mlv code nicer to read
 */

#include "LibRaw/libraw/libraw.h"

typedef struct {
    libraw_data_t * libraw;
    double matrix[9];
    char cam_name[32];

    int width;
    int height;
} RawReader_t;

int init_RawReader(RawReader_t * Raw, char * File)
{
    memset(Raw, 0, sizeof(RawReader_t));

    libraw_data_t * libraw = libraw_init(0);

    /* No flipping */
    libraw->params.user_flip = 0;

    printf("Opening file %s\n", File);
    if (libraw_open_file(libraw, File) != 0) {
        puts("failed to open file");
        return 1;
    } if (libraw_unpack(libraw) != 0) {
        puts("failed to unpack");
        return 1;
    }

    /* Get matrix */
    double mat1[9];
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            printf("%3.5f, ", libraw->rawdata.color.cam_xyz[y][x]);
            mat1[y*3+x] = libraw->rawdata.color.cam_xyz[y][x];
        } printf("\n");
    }

    /* Get camera name */
    snprintf(Raw->cam_name, 32, "%s %s",libraw->idata.make,libraw->idata.model);

    /* Get margins and width/height, canons for example have black area */
    int crop_top = libraw->rawdata.sizes.top_margin;
    int crop_left = libraw->rawdata.sizes.left_margin;
    int width = libraw_get_iwidth(libraw);
    /* Round width down to a multiple of 8 for MLV compatibility */
    width = width - (width % 8);
    int height = libraw_get_iheight(libraw);
    int raw_width = libraw_get_raw_width(libraw);

    /* HACK - memmove the image rows to remove the margins.
     * Not a great solution, as we are modifying inside libraw's memory */

    uint16_t * dst = libraw->rawdata.raw_image;
    uint16_t * src = dst + crop_top*raw_width + crop_left;

    for (int i = 0; i < height; ++i)
    {
        memmove(dst, src, width*sizeof(uint16_t));
        dst += width;
        src += raw_width;
    }

    Raw->width = width;
    Raw->height = height;

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
    return Raw->width;
}

int RawGetHeight(RawReader_t * Raw)
{
    return Raw->height;
}

double * RawGetMatrix(RawReader_t * Raw)
{
    return Raw->matrix;
}

char * RawGetCamName(RawReader_t * Raw)
{
    return Raw->cam_name;
}