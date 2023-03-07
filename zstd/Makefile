all: compress decompress

compress: compress.c zstd.c
	gcc -O2 -o $@ $^

decompress: decompress.c zstd.c
	gcc -O2 -o $@ $^

clean:
	rm compress decompress *.o out.*
