#include "libmlv.h"

struct mlv_FrameExtractor
{
    void * lj92_decoder;
    void * encoded_data;
    void * u16_data;
};

mlv_FrameExtractor * mlv_newFrameExtractor(mlv_Alloc Allocator, void * AllocatorUD)
{
    mlv_FrameExtractor * frame_extractor = mlv_Malloc(Allocator, AllocatorUD, sizeof(mlv_FrameExtractor));

    frame_extractor->lj92_decoder = NULL;
    frame_extractor->encoded_data = NULL;
    frame_extractor->u16_data = NULL;

    return frame_extractor;
}

void * mlv_FrameExtractorGetFrameData(mlv_FrameExtractor * FrameExtractor,
                                      mlv_Index * Index,
                                      mlv_DataSource * DataSource,
                                      uint64_t FrameNumber,
                                      uint64_t * NumBytesOut,
                                      int AllowIndexing);

uint16_t * mlv_FrameExtractorGetFrame(mlv_FrameExtractor * FrameExtractor,
                                      mlv_Index * Index,
                                      mlv_DataSource * DataSource,
                                      uint64_t FrameNumber,
                                      int AllowIndexing);

uint16_t * mlv_FrameExtractorGetAudioData(mlv_FrameExtractor * FrameExtractor,
                                          uint64_t AudioFrameNumber,
                                          mlv_Index * Index,
                                          mlv_DataSource * DataSource,
                                          uint64_t * NumSamplesOut,
                                          int AllowIndexing);

/* Will free any frame data (happens automatically anyway on next frame) */
void mlv_FrameExtractorFree(mlv_FrameExtractor * FrameExtractor);