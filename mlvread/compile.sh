gcc -c -O3 mlvread.c
gcc -c -O3 ../src/MLVReader.c

gcc *.o -o mlvplay