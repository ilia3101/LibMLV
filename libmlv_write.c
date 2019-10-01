#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "libmlv_write.h"

static void set_mlv_block_string(void * block, char * string)
{
    /* C standard says char is always 1 byte */
    for (int i = 0; i < 4; ++i) ((uint8_t *)block)[i] = string[i];
}

size_t sizeof_MLVWriter()
{
    return sizeof(MLVWriter_t);
}

void init_MLVWriter( MLVWriter_t * Writer,
                     int Width,
                     int Height,
                     int BitDepth,
                     int CompressedLJ92 )
{
    { /* Zerro everything */
        uint8_t * data = (uint8_t *)Writer;
        for (int i = 0; i < sizeof(MLVWriter_t); ++i) data[i] = 0;
    }

    /* Set correct strings in all mlv block headers */
    set_mlv_block_string((void *)&Writer->MLVI, "MLVI");
    set_mlv_block_string((void *)&Writer->VIDF, "VIDF");
    set_mlv_block_string((void *)&Writer->AUDF, "AUDF");
    set_mlv_block_string((void *)&Writer->RAWI, "RAWI");
    set_mlv_block_string((void *)&Writer->WAVI, "WAVI");
    set_mlv_block_string((void *)&Writer->EXPO, "EXPO");
    set_mlv_block_string((void *)&Writer->LENS, "LENS");
    set_mlv_block_string((void *)&Writer->RTCI, "RTCI");
    set_mlv_block_string((void *)&Writer->IDNT, "IDNT");
    set_mlv_block_string((void *)&Writer->INFO, "INFO");
    set_mlv_block_string((void *)&Writer->DISO, "DISO");
    set_mlv_block_string((void *)&Writer->MARK, "MARK");
    set_mlv_block_string((void *)&Writer->STYL, "STYL");
    set_mlv_block_string((void *)&Writer->ELVL, "ELVL");
    set_mlv_block_string((void *)&Writer->WBAL, "WBAL");
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
                                    NULL, NULL );
            break;
        case Canon_5D_Mark_III:
            MLVWriterSetCameraInfo( Writer, "Canon EOS 5D Mark III", 0x80000285,
                                    NULL, NULL );
            break;
        case Canon_6D:
            MLVWriterSetCameraInfo( Writer, "Canon EOS 6D", 0x80000302,
                                    NULL, NULL );
            break;
        case Canon_7D:
            MLVWriterSetCameraInfo( Writer, "Canon EOS 7D", 0x80000250,
                                    NULL, NULL );
            break;
        case Canon_50D:
            MLVWriterSetCameraInfo( Writer, "Canon EOS 50D", 0x80000261,
                                    NULL, NULL );
            break;
        case Canon_60D:
            MLVWriterSetCameraInfo( Writer, "Canon EOS 60D", 0x80000287,
                                    NULL, NULL );
            break;
        case Canon_500D:
            MLVWriterSetCameraInfo( Writer, "Canon EOS 500D", 0x80000252,
                                    NULL, NULL );
            break;
        case Canon_550D:
            MLVWriterSetCameraInfo( Writer, "Canon EOS 550D", 0x80000270,
                                    NULL, NULL );
            break;
        case Canon_600D:
            MLVWriterSetCameraInfo( Writer, "Canon EOS 600D", 0x80000286,
                                    NULL, NULL );
            break;
        case Canon_650D:
            MLVWriterSetCameraInfo( Writer, "Canon EOS 650D", 0x80000301,
                                    NULL, NULL );
            break;
        case Canon_700D:
            MLVWriterSetCameraInfo( Writer, "Canon EOS 700D", 0x80000326,
                                    NULL, NULL );
            break;
        case Canon_EOSM:
            MLVWriterSetCameraInfo( Writer, "Canon EOS M", 0x80000331,
                                    NULL, NULL );
            break;
    }
}

void MLVWriterSetCameraInfo( MLVWriter_t * Writer,
                             char * CameraName,
                             uint32_t CameraModelID,
                             double * MatrixDaylight,
                             double * MatrixTungsten )
{
    if (CameraName != NULL) strcpy((char *)Writer->IDNT.cameraName, CameraName);
    Writer->IDNT.cameraModel = CameraModelID;
}