all: compress decompress mjpeg

compress: compress.c rvl.c lz4.c package.c
	gcc -O2 -o $@ $^

decompress: decompress.c rvl.c lz4.c
	gcc -O2 -o $@ $^

mjpeg: mjpeg.c
	gcc -O2 -o $@ $^

clean:
	rm compress decompress mjpeg *.o out.*
