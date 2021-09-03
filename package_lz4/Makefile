all: compress decompress

compress: compress.c lz4.c package.c
	gcc -O2 -o $@ $^

decompress: decompress.c lz4.c
	gcc -O2 -o $@ $^

clean:
	rm compress decompress *.o out.*
