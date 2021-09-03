all: compress decompress

compress: compress.c
	gcc -O2 -o $@ $^ -lzstd

decompress: decompress.c
	gcc -O2 -o $@ $^ -lzstd

clean:
	rm compress decompress *.o out.*
