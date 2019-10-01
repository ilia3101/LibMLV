gcc -c -O3 libmlv_write.c
gcc -c -O3 write_mlv.c

gcc *.o -o mlv_write
rm *.o