CC=gcc
FLAGS=-c -O3

ifeq ($(shell uname -s), Linux)
main: libraw_r.a raw2mlv_c LibMLV
	$(CC) raw2mlv.o libraw_r.a ../lib/libmlv.a -o raw2mlv -lm -lgomp -lstdc++
else
main: libraw_r.a raw2mlv_c LibMLV
	$(CC) raw2mlv.o libraw_r.a ../lib/libmlv.a -o raw2mlv -lm -lstdc++
endif

raw2mlv_c: camera_matrices.c read_raw.c
	$(CC) $(FLAGS) raw2mlv.c

LibMLV:
	$(MAKE) -C ../

libraw_r.a:
	./getlibraw.sh
