#include <stdio.h>
#include <stdlib.h>

#include "libmlv_write.h"

/* Self documenting code */

int main(int argc, char ** argv)
{
    MLVWriter_t * writer = alloca(sizeof_MLVWriter());

    init_MLVWriter( writer,
                    200, /* Width */
                    200, /* Height */
                    14, /* Bitdepth */
                    0, /* Compressed LJ92? */
                    1792, /* Black levbel */
                    15000 /* White level */ );

    MLVWriterSetCameraPreset(writer, Canon_5D_Mark_II);

    uninit_MLVWriter(writer);

    return 0;
}