CC=gcc
FLAGS=-c -fPIC -O3 -Wall -Wextra

main: MLVWriter MLVReader MLVFrameUtils MLVDataSource
	ar rcs lib/libmlv.a lib/*.o
	$(CC) -shared lib/*.o -o lib/libmlv.so

MLVWriter:
	$(CC) src/MLVWriter.c $(FLAGS) -o lib/MLVWriter.o

MLVReader:
	$(CC) src/MLVReader.c $(FLAGS) -o lib/MLVReader.o

MLVFrameUtils:
	$(CC) src/MLVFrameUtils.c $(FLAGS) -o lib/MLVFrameUtils.o

MLVDataSource:
	$(CC) src/MLVDataSource.c $(FLAGS) -o lib/MLVDataSource.o