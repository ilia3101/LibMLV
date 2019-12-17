CC=gcc
FLAGS=-c -fPIC -O3

main: MLVWriter MLVReader MLVFrameUtils
	ar rcs lib/libmlv.a *.o
	$(CC) -shared lib/*.o -o lib/libmlv.so

MLVWriter:
	$(CC) src/MLVWriter.c $(FLAGS) -o lib/MLVWriter.o

MLVReader:
	$(CC) src/MLVReader.c $(FLAGS) -o lib/MLVReader.o

MLVFrameUtils:
	$(CC) src/MLVFrameUtils.c $(FLAGS) -o lib/MLVFrameUtils.o