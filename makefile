CC=gcc
FLAGS=-c -fPIC -O3

main: MLVWriter.o MLVReader.o MLVFrameUtils.o MLVDataSource.o lj92.o
	ar rcs lib/libmlv.a lib/*.o
	$(CC) -shared lib/*.o -o lib/libmlv.so

MLVWriter.o:
	$(CC) src/MLVWriter.c $(FLAGS) -o lib/MLVWriter.o

MLVReader.o:
	$(CC) src/MLVReader.c $(FLAGS) -o lib/MLVReader.o

MLVFrameUtils.o:
	$(CC) src/MLVFrameUtils.c $(FLAGS) -o lib/MLVFrameUtils.o

MLVDataSource.o:
	$(CC) src/MLVDataSource.c $(FLAGS) -o lib/MLVDataSource.o

lj92.o:
	$(CC) src/liblj92/lj92.c $(FLAGS) -o lib/lj92.o