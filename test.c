// gcc -c -O3 *.c; gcc *.o -o test; rm *.o;
#include <stdio.h>
#include <unistd.h>

#include <time.h>
#define CREATE_TIMER(TName) clock_t start##TName, diff##TName; int msec##TName;
#define START_TIMER(TName) start##TName = clock();
#define END_TIMER(TName) diff##TName=clock()-start##TName;msec##TName=diff##TName*1000.0/CLOCKS_PER_SEC;
#define GET_TIMER_RESULT(TName) (float)(msec##TName)

#include "libmlv.h"
#include "libmlvaux.h"

int main(int argc, char ** argv)
{
    printf("Will read file %s\n", argv[1]);

    CREATE_TIMER(timer)
    START_TIMER(timer)

    mlv_DataSource * datasource = mlvL_newDataSource(argv[1], 0);
    mlv_Index * index = mlvL_newIndex();
    mlv_FrameExtractor * frame_extractor = mlvL_newFrameExtractor();

    /* Index the entire file */
    mlv_IndexBuild(index, datasource, 0, 0);

    /* OPtimise index */
    mlv_IndexOptimise(index);

    END_TIMER(timer)

    /* Print the index */
    mlv_IndexPrint(index);

    /* The timer can say 0 milliseconds sometimes cause its so fast */
    printf("%.1fKiB index size, took %.1f milliseconds to build and optimise\n", mlv_IndexGetSize(index)/1024.0, GET_TIMER_RESULT(timer));

    return 0;
}