#ifndef _libmlv_write_h_
#define _libmlv_write_h_

#include <stdint.h>

#include "mlv_structs.h"

/********************************** Structure *********************************/

#define MLVWriter_header_block(BlockType, BlockName) \
struct \
{ \
    union { \
        BlockType block; \
        mlv_hdr_t header; \
    }; \
    int write; /* Should this block be written? */ \
} BlockName;

typedef struct
{
    int frame_info_set; /* Basic info: width, height, bitdepth, compression */
    int camera_info_added; /* Camera info, like camera matrix and name */
    int lens_info_added;

    /* Header blocks */
    MLVWriter_header_block(mlv_file_hdr_t, MLVI)
    MLVWriter_header_block(mlv_rawi_hdr_t, RAWI)
    MLVWriter_header_block(mlv_wavi_hdr_t, WAVI)
    MLVWriter_header_block(mlv_expo_hdr_t, EXPO)
    MLVWriter_header_block(mlv_lens_hdr_t, LENS)
    MLVWriter_header_block(mlv_rtci_hdr_t, RTCI)
    MLVWriter_header_block(mlv_idnt_hdr_t, IDNT)
    MLVWriter_header_block(mlv_info_hdr_t, INFO)
    MLVWriter_header_block(mlv_diso_hdr_t, DISO)
    MLVWriter_header_block(mlv_mark_hdr_t, MARK)
    MLVWriter_header_block(mlv_styl_hdr_t, STYL)
    MLVWriter_header_block(mlv_elvl_hdr_t, ELVL)
    MLVWriter_header_block(mlv_wbal_hdr_t, WBAL)

    /* Frame blocks */
    mlv_vidf_hdr_t VIDF;
    mlv_audf_hdr_t AUDF;

} MLVWriter_t;

/******************************* Initialisation *******************************/

/* Returns amount of memory you need to allocate for an MLV writer */
size_t sizeof_MLVWriter();

void init_MLVWriter( MLVWriter_t * Writer,
                     int Width,
                     int Height,
                     int BitDepth,
                     int Compressed,
                     int BlackLevel,
                     int WhiteLevel );

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
                             double * ColourMatrix );

/********************************** Writing ***********************************/

/* After setting all of the metadata you want to set and want to begin writing,
 * call this to find out size of header data needing to be written */
size_t MLVWriterGetHeaderSize(MLVWriter_t * Writer);

/* After allocating enough memory based on what MLVWriterGetHeaderSize told you,
 * use this function to get all of that data output to HeaderData pointer. Then
 * you must write it to a file yourself */
void MLVWriterGetHeaderData(MLVWriter_t * Writer, void * HeaderData);

#endif