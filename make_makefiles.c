#include <stdio.h>
#include <string.h>

#define OBJECT_FOLDER "lib"
#define RESULT_NAME "libmlv"

char * source_files[][10] =
{
    {"src/MLVWriter.c",      "include/MLVWriter.h", "src/liblj92/lj92.h", "include/MLVFrameUtils.h", NULL},
    {"src/MLVReader.c",      "include/MLVReader.h", "src/liblj92/lj92.h", NULL},
    {"src/MLVFrameUtils.c",  "include/MLVFrameUtils.h", NULL},
    {"src/MLVDataSource.c",  "include/MLVDataSource.h", NULL},
    {"src/liblj92/lj92.c",   "src/liblj92/lj92.h", NULL}
};

#define NUM_SOURCE_FILES (sizeof(source_files)/sizeof(source_files[0]))

void write_rules(FILE * makefile, char * ObjectFileExtension, char * CompilerOutSetting, char * rm_Command, char * MainRule)
{
    char object_names[NUM_SOURCE_FILES][100];

    for (int i = 0; i < NUM_SOURCE_FILES; ++i)
    {
        char filename_str[100];
        strcpy(filename_str, source_files[i][0]);
        char * filename = filename_str + strlen(filename_str);
        while (*filename != '/') filename--; ++filename;
        char * dot = filename + strlen(filename);
        while (*dot != '.') dot--;
        strcpy(dot, ObjectFileExtension);

        sprintf(object_names[i], "$(OBJ_FOLDER)/%s", filename);
    }

    /* Main rule */
    fprintf(makefile, "main:");
    for (int i = 0; i < NUM_SOURCE_FILES; ++i) fprintf(makefile, " %s", object_names[i]);
    fprintf(makefile, "\n%s", MainRule);

    for (int i = 0; i < NUM_SOURCE_FILES; ++i)
    {
        fprintf(makefile, "\n\n%s:", object_names[i]);

        for (int j = 0; j < 100 && source_files[i][j] != NULL; ++j)
        {
            fprintf(makefile, " %s", source_files[i][j]);
        }

        fprintf(makefile,"\n\t$(CC) $(CFLAGS) %s %s%s",
                source_files[i][0], CompilerOutSetting, object_names[i]);
    }

    /* Clean rule */
    fprintf(makefile, "\n\nclean:\n\t%s", rm_Command);
    for (int i = 0; i < NUM_SOURCE_FILES; ++i) fprintf(makefile, " %s", object_names[i]);
    fprintf(makefile, " $(OBJ_FOLDER)/$(NAME).*");
}

int main()
{
    /* Unix */
    FILE * makefile = fopen("makefile", "w");
    fprintf(makefile,
        "CC=gcc\n"
        "CFLAGS=-c -fPIC -O3 -std=c99\n"
        "NAME="RESULT_NAME"\n"
        "OBJ_FOLDER="OBJECT_FOLDER"\n\n"
    );
    write_rules(makefile, ".o", "-o ", "rm",
	"\tar rcs $(OBJ_FOLDER)/$(NAME).a $(OBJ_FOLDER)/*.o\n"
	"\t$(CC) -shared $(OBJ_FOLDER)/*.o -o $(OBJ_FOLDER)/$(NAME).so");
    fclose(makefile);


    /* MSVC */
    makefile = fopen("makefile.msvc", "w");
    fprintf(makefile,
        "CC=CL\n"
        "CFLAGS=/c /Ox\n"
        "NAME="RESULT_NAME"\n"
        "OBJ_FOLDER="OBJECT_FOLDER"\n\n"
    );
    write_rules(makefile, ".obj", "/Fo:", "DEL",
	"\tLIB $(OBJFOLDER)/*.obj /OUT:$(OBJ_FOLDER)/$(NAME).lib\n"
	"\tLINK /DLL $(OBJ_FOLDER)/*.obj /OUT:$(OBJ_FOLDER)/$(NAME).dll");
    fclose(makefile);


    return 0;
}
