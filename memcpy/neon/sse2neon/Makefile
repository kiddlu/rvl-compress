all: compress decompress

compress: compress.c neon_memcpy.c
	gcc -O2 -o $@ $^

decompress: decompress.c neon_memcpy.c
	gcc -O2 -o $@ $^

clean:
	rm compress decompress *.o out.*
