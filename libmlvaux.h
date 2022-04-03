#ifndef _libmlv_aux_h_
#define _libmlv_aux_h_

#include "libmlv.h"

mlv_Index * mlvL_newIndex();

mlv_FrameExtractor * mlvL_newFrameExtractor();

mlv_DataSource * mlvL_newDataSource(char * MainChunkFileName,
                                    int SearchForAdditionalChunks);

mlv_DataSource * mlvL_newDataSourceFromChunks(char ** ChunkFileNames,
                                              int NumFiles);

/* Prints the index. For debugging. */
void mlvL_IndexPrint(mlv_Index * Index);

#endif