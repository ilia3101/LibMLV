#ifndef _libmlv_write_h_
#define _libmlv_write_h_

#include <stdint.h>

/********************************** Structure *********************************/

/* Each value in MLVWriter_t is one of these, followed by its data */
typedef struct {
    uint32_t ID;
    uint32_t data_size;
} MLVWriter_value_t;

/* In reality the values array will be any size */
typedef struct {
    MLVWriter_value_t values[1];
} MLVWriter_t;

/******************************* Initialisation *******************************/

/* Returns amount of memory you need to allocate for an MLV writer */
size_t sizeof_MLVWriter();

void init_MLVWriter( MLVWriter_t * Writer,
                     int Width,
                     int Height,
                     int BitDepth,
                     int CompressedLJ92 );

void uninit_MLVWriter(MLVWriter_t * Writer);

/********************************** Metadata **********************************/

enum MLVCamPreset {
    Canon_5D_Mark_II,
    Canon_5D_Mark_III,
    Canon_6D,
    Canon_7D
};

/* Set a camera preset from enum MLVCamPreset */
void MLVWriterSetCameraPreset(MLVWriter_t * Writer, enum MLVCamPreset Camera);

/* For manually setting camera data if it is not in the presets.
 * You can pass NULL for any of these if you'd like not to provide that
 * specific info */
void MLVWriterSetCameraInfo( MLVWriter_t * Writer,
                             char * Name,
                             double * MatrixDaylight,
                             double * MatrixTungsten,
                             char * LensName );

/********************************** Writing ***********************************/

#endif