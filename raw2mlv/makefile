CC=gcc
FLAGS=-c -O3
LINKFLAGS=-lm -lstdc++

ifeq ($(shell uname -s), Linux)
main: raw2mlv_c LibMLV GetLibRAW
	$(CC) raw2mlv.o libraw_r.a libMLV.a -o raw2mlv -lm -lgomp -lstdc++
else
main: raw2mlv_c GetLibRAW LibMLV 
	$(CC) raw2mlv.o libraw_r.a libMLV.a -o raw2mlv -lm -lstdc++
endif

raw2mlv_c:
	$(CC) $(FLAGS) raw2mlv.c

LibMLV:
	$(MAKE) -C ../LibMLV/build/unix
	cp ../LibMLV/build/unix/libMLV.a libMLV.a

GetLibRAW:
	./getlibraw.sh