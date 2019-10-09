#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "libmlv_write.h"

static void set_mlv_block_string(void * block, char * string)
{
    /* C standard says char is always 1 byte */
    for (int i = 0; i < 4; ++i) ((uint8_t *)block)[i] = string[i];
}

/* Sets string and block size */
#define init_mlv_block(Block, String) \
{ \
    char * block_name = String; \
    mlv_hdr_t * block = &block;
    set_mlv_block_string(&Block, String); \
    ((uint32_t *)&Block)[1] = sizeof(typeof(Block)); \
}

size_t sizeof_MLVWriter()
{
    return sizeof(MLVWriter_t);
}

void init_MLVWriter( MLVWriter_t * Writer,
                     int Width,
                     int Height,
                     int BitDepth,
                     int CompressedLJ92,
                     int BlackLevel,
                     int WhiteLevel )
{
    /* Zerro everything */
    for (int i = 0; i < sizeof(MLVWriter_t); ++i) ((uint8_t *)Writer)[i] = 0;

    /* Set correct strings and block sizes in all mlv block headers */
    init_mlv_block(Writer->MLVI, "MLVI");
    init_mlv_block(Writer->VIDF, "VIDF");
    init_mlv_block(Writer->AUDF, "AUDF");
    init_mlv_block(Writer->RAWI, "RAWI");
    init_mlv_block(Writer->WAVI, "WAVI");
    init_mlv_block(Writer->EXPO, "EXPO");
    init_mlv_block(Writer->LENS, "LENS");
    init_mlv_block(Writer->RTCI, "RTCI");
    init_mlv_block(Writer->IDNT, "IDNT");
    init_mlv_block(Writer->INFO, "INFO");
    init_mlv_block(Writer->DISO, "DISO");
    init_mlv_block(Writer->MARK, "MARK");
    init_mlv_block(Writer->STYL, "STYL");
    init_mlv_block(Writer->ELVL, "ELVL");
    init_mlv_block(Writer->WBAL, "WBAL");

    /* Set resolution in all possible fields */
    Writer->RAWI.xRes = Width;
    Writer->RAWI.yRes = Height;
    Writer->RAWI.raw_info.width = Width;
    Writer->RAWI.raw_info.height = Height;
    Writer->RAWI.raw_info.jpeg.width = Width;
    Writer->RAWI.raw_info.jpeg.height = Height;
    Writer->RAWI.raw_info.active_area.x2 = Width;
    Writer->RAWI.raw_info.active_area.y2 = Height;

    /* Set default bayer pattern */
    Writer->RAWI.raw_info.cfa_pattern = 0x02010100;

    /* Compression and bit depth */
    Writer->RAWI.raw_info.bits_per_pixel = BitDepth;
    Writer->MLVI.videoClass = MLV_VIDEO_CLASS_RAW;
    if (CompressedLJ92) Writer->MLVI.videoClass |= MLV_VIDEO_CLASS_FLAG_LJ92;

    /* White and black level */
    Writer->RAWI.raw_info.black_level = BlackLevel;
    Writer->RAWI.raw_info.white_level = WhiteLevel;

    /* MLV version */
    char * ver = MLV_VERSION_STRING;
    for (int i = 0; ver[i] != 0; ++i) Writer->MLVI.versionString[i] = ver[i];
}

void uninit_MLVWriter(MLVWriter_t * Writer)
{
    /* Do nothing, as nothing has been created */
    return;
}

void MLVWriterSetCameraPreset(MLVWriter_t * Writer, enum MLVCamPreset Camera)
{
    switch (Camera)
    {
        case Canon_5D_Mark_II:
            MLVWriterSetCameraInfo( Writer, "Canon EOS 5D Mark II", 0x80000218,
                                    NULL );
            break;
        case Canon_5D_Mark_III:
            MLVWriterSetCameraInfo( Writer, "Canon EOS 5D Mark III", 0x80000285,
                                    NULL );
            break;
        case Canon_6D:
            MLVWriterSetCameraInfo( Writer, "Canon EOS 6D", 0x80000302,
                                    NULL );
            break;
        case Canon_7D:
            MLVWriterSetCameraInfo( Writer, "Canon EOS 7D", 0x80000250,
                                    NULL );
            break;
        case Canon_50D:
            MLVWriterSetCameraInfo( Writer, "Canon EOS 50D", 0x80000261,
                                    NULL );
            break;
        case Canon_60D:
            MLVWriterSetCameraInfo( Writer, "Canon EOS 60D", 0x80000287,
                                    NULL );
            break;
        case Canon_500D:
            MLVWriterSetCameraInfo( Writer, "Canon EOS 500D", 0x80000252,
                                    NULL );
            break;
        case Canon_550D:
            MLVWriterSetCameraInfo( Writer, "Canon EOS 550D", 0x80000270,
                                    NULL );
            break;
        case Canon_600D:
            MLVWriterSetCameraInfo( Writer, "Canon EOS 600D", 0x80000286,
                                    NULL );
            break;
        case Canon_650D:
            MLVWriterSetCameraInfo( Writer, "Canon EOS 650D", 0x80000301,
                                    NULL );
            break;
        case Canon_700D:
            MLVWriterSetCameraInfo( Writer, "Canon EOS 700D", 0x80000326,
                                    NULL );
            break;
        case Canon_EOSM:
            MLVWriterSetCameraInfo( Writer, "Canon EOS M", 0x80000331,
                                    NULL );
            break;
    }
}

void MLVWriterSetCameraInfo( MLVWriter_t * Writer,
                             char * CameraName,
                             uint32_t CameraModelID,
                             double * ColourMatrix )
{
    if (CameraName != NULL) strcpy((char *)Writer->IDNT.cameraName, CameraName);
    Writer->IDNT.cameraModel = CameraModelID;

    int32_t * matrix = Writer->RAWI.raw_info.color_matrix1;
    if (ColourMatrix != NULL)
    {
        for (int i = 0; i < 9; ++i) {
            matrix[i * 2] = (int32_t)(ColourMatrix[i]*10000 + 0.5);
            matrix[i*2+1] = 10000;
        }
    }
    else
    {
        /* By default, set 5D2 matrix as it is the most important camera */
        int32_t mat5D2[] = { 5309, 10000,  -229, 10000, -336, 10000,
                            -6241, 10000, 13265, 10000, 3337, 10000,
                             -817, 10000,  1215, 10000, 6664, 10000 };
        for (int i = 0; i < 18; ++i) matrix[i] = mat5D2[i];
    }
}

