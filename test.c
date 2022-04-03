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
    char * mlv_path = argv[1];

    if (mlv_path)
    {
        printf("Will read file %s\n", argv[1]);

        CREATE_TIMER(timer)
        START_TIMER(timer)

        mlv_DataSource * datasource = mlvL_newDataSource(argv[1], 1);
        mlv_Index * index = mlvL_newIndex();
        mlv_FrameExtractor * frame_extractor = mlvL_newFrameExtractor();

        /* Index the entire file all at once... */
        // mlv_IndexBuild(index, datasource, 0);

        /* ... or index the file in sections of 20 blocks */
        while (!mlv_IndexIsComplete(index))
            mlv_IndexBuild(index, datasource, 20);

        /* OPtimise index */
        mlv_IndexOptimise(index);

        END_TIMER(timer)

        /* Print the index */
        mlv_IndexPrint(index);

        int frame_number_to_find = 2;
        int64_t frame_index = mlv_IndexFindEntry(index, 0,
                                                 "VIDF",
                                                 0, 0, 0,
                                                 0, 0, 0,
                                                 1, frame_number_to_find,
                                                 1 );

        printf("\nFrame %i at %lld\n", frame_number_to_find, frame_index);

        /* The timer can say 0 milliseconds sometimes cause its so fast */
        printf("\n%.1fKiB index size, took %.1f milliseconds to build and optimise\n", mlv_IndexGetSize(index)/1024.0, GET_TIMER_RESULT(timer));
    }
    else
    {
        puts("specify file when running thanks");
    }

    return 0;
}