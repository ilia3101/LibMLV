#include <stdio.h>
#include <stdlib.h>

#include "libmlv_write.h"

/* Self documenting code */

int main(int argc, char ** argv)
{
    MLVWriter_t * writer = alloca(sizeof_MLVWriter());

    init_MLVWriter(writer, 200, 200, 14, 0);

    MLVWriterSetCameraPreset(writer, Canon_5D_Mark_II);

    uninit_MLVWriter(writer);

    return 0;
}