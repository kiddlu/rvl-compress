all: compress decompress mjpeg benchmark

compress: compress.c ./algo/rvl.c ./algo/lz4.c package.c
	gcc -fomit-frame-pointer -fstrict-aliasing -ffast-math -O3 -o $@ $^ -lpthread

decompress: decompress.c ./algo/rvl.c ./algo/lz4.c
	gcc -fomit-frame-pointer -fstrict-aliasing -ffast-math -O3  -o $@ $^ -lpthread

benchmark:  benchmark.c ./algo/chronometer.c ./algo/rvl.c ./algo/lz4.c ./algo/shrinker.c
	gcc -fomit-frame-pointer -fstrict-aliasing -ffast-math -O3  -o $@ $^ -lpthread

mjpeg: mjpeg.c
	gcc -fomit-frame-pointer -fstrict-aliasing -ffast-math -O3 -o $@ $^

clean:
	rm compress decompress mjpeg benchmark
