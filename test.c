// gcc -c -O3 *.c; gcc *.o -o test; rm *.o;
#include <stdio.h>

#include "libmlv.h"
#include "libmlvaux.h"

int main(int argc, char ** argv)
{
    printf("Will read file %s\n", argv[1]);

    mlv_DataSource * datasource = mlvL_newDataSourceFromMainChunk(argv[1], 0);
    mlv_Index * index = mlvL_newIndex();
    mlv_FrameExtractor * frame_extractor = mlvL_newFrameExtractor();

    mlv_IndexBuild(index, datasource, 0, 0);

    return 0;
}