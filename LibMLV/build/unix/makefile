CC=gcc
FLAGS=-c -fPIC -O3

main: MLVWriter MLVReader MLVFrameUtils
	ar rcs libMLV.a *.o
	gcc -shared *.o -o libMLV.so

MLVWriter:
	$(CC) ../../MLVWriter.c $(FLAGS)

MLVReader:
	$(CC) ../../MLVReader.c $(FLAGS)

MLVFrameUtils:
	$(CC) ../../MLVFrameUtils.c $(FLAGS)