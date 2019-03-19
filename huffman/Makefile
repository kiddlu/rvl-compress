all: compress decompress

compress: compress.c huffman.o
	gcc -o $@ $^

decompress: decompress.c huffman.o
	gcc -o $@ $^

huffman.o: huffman.c
	gcc -c $^ -o $@

clean:
	rm compress decompress *.o out.*
