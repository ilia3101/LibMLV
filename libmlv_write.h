#ifndef _libmlv_write_h_
#define _libmlv_write_h_

#include <stdint.h>

#include "mlv_structs.h"

/********************************** Structure *********************************/

typedef struct
{
    int frame_info_initialised;
    int camera_info_initialised;
    int headers_written;

    /* MLV Headers */
    mlv_file_hdr_t MLVI;

    mlv_vidf_hdr_t VIDF;
    mlv_audf_hdr_t AUDF;

    mlv_rawi_hdr_t RAWI;
    mlv_wavi_hdr_t WAVI;

    mlv_expo_hdr_t EXPO;
    mlv_lens_hdr_t LENS;
    mlv_rtci_hdr_t RTCI;
    mlv_idnt_hdr_t IDNT;
    mlv_info_hdr_t INFO;
    mlv_diso_hdr_t DISO;
    mlv_mark_hdr_t MARK;
    mlv_styl_hdr_t STYL;
    mlv_elvl_hdr_t ELVL;
    mlv_wbal_hdr_t WBAL;

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
    Canon_7D,
    Canon_50D,
    Canon_60D,
    Canon_500D,
    Canon_550D,
    Canon_600D,
    Canon_650D,
    Canon_700D,
    Canon_EOSM
};

/* Set a camera preset from enum MLVCamPreset */
void MLVWriterSetCameraPreset(MLVWriter_t * Writer, enum MLVCamPreset Camera);

/* For manually setting camera data if it is not in the presets.
 * You can pass NULL for any of these if you'd like not to provide that
 * specific info */
void MLVWriterSetCameraInfo( MLVWriter_t * Writer,
                             char * CameraName,
                             uint32_t CameraModelID,
                             double * MatrixDaylight,
                             double * MatrixTungsten );

/********************************** Writing ***********************************/

#endif