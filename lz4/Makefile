all: compress decompress

compress: compress.c lz4.o
	gcc -o $@ $^

decompress: decompress.c lz4.o
	gcc -o $@ $^

lz4.o: lz4.c
	gcc -c $^ -o $@

clean:
	rm compress decompress *.o out.*
