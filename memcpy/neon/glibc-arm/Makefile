all: compress
#all: compress decompress

compress: compress.c memcpy.S
	gcc -O2 -o $@ $^

decompress: decompress.c
	gcc -O2 -o $@ $^

clean:
	rm compress decompress *.o out.*
