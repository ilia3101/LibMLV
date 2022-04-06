// gcc -c -O3 *.c; gcc *.o -o test; rm *.o;
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

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
        puts("Timer started...");

        mlv_DataSource * datasource = mlvL_newDataSource(argv[1], 1);

        if (datasource != NULL)
        {
            mlv_Index * index = mlvL_newIndex();
            mlv_FrameExtractor * frame_extractor = mlvL_newFrameExtractor();

            /* Index the entire file all at once... */
            // mlv_IndexBuild(index, datasource, 0);

            /* Alternatively ... index the file 5 blocks at a time */
            while (!mlv_IndexIsComplete(index))
                mlv_IndexBuild(index, datasource, 5);

            /* OPtimise index for fastest read access */
            // mlv_IndexOptimise(index);
            /* Optimise index for storage (re-writing the file) */
            mlv_IndexOptimiseForStorage(index);

            END_TIMER(timer)

            /* Print the index */
            // mlv_IndexPrint(index);

            /* The timer can say 0 milliseconds sometimes cause its so fast */
            printf("\n%.1fKiB index size, took %.1f milliseconds to build and optimise\n", mlv_IndexGetSize(index)/1024.0, GET_TIMER_RESULT(timer));
            int num_chunks = mlv_DataSourceGetNumChunks(datasource);
            printf("This MLV has %i known chunk%s.\n", num_chunks, num_chunks==1 ? "" : "s");


            // int use_framenumber_to_search = 1;
            // int frame_number_to_find = 0;
            // int64_t entry_MLVI = mlv_IndexFindEntry(index, 0,
            //                                         "MLVI",
            //                                         0, 0, 0,
            //                                         0, 0, 0,
            //                                         use_framenumber_to_search, frame_number_to_find,
            //                                         1 );

            /* Re-write the MLV to a file 😎😎😎 */
            uint8_t * data = malloc(1024*1024*1024); /* All blocks should fit in 1GiB (it won't actually take up that much) */
            FILE * outfile = fopen("out.MLV", "w");
            int64_t entry_id = 0;
            int blocks_written = 0;
            do {
                if (entry_id >= 0)
                {
                    uint8_t btype[5] = {0};
                    mlv_IndexGetBlockType(index, entry_id, btype);
                    /* Only write an MLVI block if 0 blocks written. This stops multiple MLVI blocks from multiple chunks being written. */
                    if ((blocks_written == 0) != (memcmp("MLVI", btype, 4) != 0))
                    {
                        // printf("blocks_written = %i, block_type = %s\n", blocks_written, btype);
                        uint32_t blocks_size = mlv_IndexGetBlockSize(index, entry_id);
                        mlv_IndexGetBlockData(index, entry_id, 0, blocks_size, data, datasource);
                        fwrite(data, blocks_size, 1, outfile);
                        ++blocks_written;
                    }
                }
                entry_id = mlv_IndexGetNextEntry(index, entry_id);
            } while (entry_id >= 0);
            fclose(outfile);
            free(data);
        }
        else puts("Couldn't create datasource, file probably doesn't exist.");
    }
    else
    {
        puts("specify file when running thanks");
    }

    return 0;
}