CC=gcc
CFLAGS=-c -fPIC -O3 -std=c99
NAME=libmlv
OBJ_FOLDER=lib

main: $(OBJ_FOLDER)/MLVWriter.o $(OBJ_FOLDER)/MLVReader.o $(OBJ_FOLDER)/MLVFrameUtils.o $(OBJ_FOLDER)/MLVDataSource.o $(OBJ_FOLDER)/lj92.o
	ar rcs $(OBJ_FOLDER)/$(NAME).a $(OBJ_FOLDER)/*.o
	$(CC) -shared $(OBJ_FOLDER)/*.o -o $(OBJ_FOLDER)/$(NAME).so

$(OBJ_FOLDER)/MLVWriter.o: src/MLVWriter.c include/MLVWriter.h src/liblj92/lj92.h include/MLVFrameUtils.h
	$(CC) $(CFLAGS) src/MLVWriter.c -o $(OBJ_FOLDER)/MLVWriter.o

$(OBJ_FOLDER)/MLVReader.o: src/MLVReader.c include/MLVReader.h src/liblj92/lj92.h
	$(CC) $(CFLAGS) src/MLVReader.c -o $(OBJ_FOLDER)/MLVReader.o

$(OBJ_FOLDER)/MLVFrameUtils.o: src/MLVFrameUtils.c include/MLVFrameUtils.h
	$(CC) $(CFLAGS) src/MLVFrameUtils.c -o $(OBJ_FOLDER)/MLVFrameUtils.o

$(OBJ_FOLDER)/MLVDataSource.o: src/MLVDataSource.c include/MLVDataSource.h
	$(CC) $(CFLAGS) src/MLVDataSource.c -o $(OBJ_FOLDER)/MLVDataSource.o

$(OBJ_FOLDER)/lj92.o: src/liblj92/lj92.c src/liblj92/lj92.h
	$(CC) $(CFLAGS) src/liblj92/lj92.c -o $(OBJ_FOLDER)/lj92.o

clean:
	rm $(OBJ_FOLDER)/MLVWriter.o $(OBJ_FOLDER)/MLVReader.o $(OBJ_FOLDER)/MLVFrameUtils.o $(OBJ_FOLDER)/MLVDataSource.o $(OBJ_FOLDER)/lj92.o $(OBJ_FOLDER)/$(NAME).*