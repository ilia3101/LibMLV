CC=gcc
FLAGS=-c -std=c99 -fPIC -O3 -Wall -Wextra
OBJ_FOLDER=lib/objects

main: MLVWriter MLVReader MLVFrameUtils MLVDataSource
	ar rcs lib/libmlv.a $(OBJ_FOLDER)/*.o
	$(CC) -shared $(OBJ_FOLDER)/*.o -o lib/libmlv.so

MLVWriter:
	$(CC) src/MLVWriter.c $(FLAGS) -o $(OBJ_FOLDER)/MLVWriter.o

MLVReader:
	$(CC) src/MLVReader.c $(FLAGS) -o $(OBJ_FOLDER)/MLVReader.o

MLVFrameUtils:
	$(CC) src/MLVFrameUtils.c $(FLAGS) -o $(OBJ_FOLDER)/MLVFrameUtils.o

MLVDataSource:
	$(CC) src/MLVDataSource.c $(FLAGS) -o $(OBJ_FOLDER)/MLVDataSource.o

clean:
	rm $(OBJ_FOLDER)/*.o lib/libmlv.so lib/libmlv.a