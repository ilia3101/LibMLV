gcc -c -O3 libmlv_write.c
gcc -c -O3 write_mlv.c

gcc *.o -o write_mlv
rm *.o