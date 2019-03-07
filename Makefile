all: compress decompress

compress: compress.c lib.o
	gcc -o $@ $^

decompress: decompress.c lib.o
	gcc -o $@ $^

lib.o: lib.c
	gcc -c $^ -o $@

clean:
	rm compress decompress *.o out.*
