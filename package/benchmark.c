#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/stat.h>

#include "./algo/cputime_api.h"

#include "./algo/lz4.h"
#include "./algo/rvl.h"
#include "./algo/shrinker.h"

#define BENCHMARK_TEST_TIME (3.0)

unsigned int get_file_size(FILE *fp)
{
   unsigned int length;
#if 1
    fseek(fp, 0L, SEEK_END);
    length = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
#else
    struct stat st;
    fstat(fileno(fp), &st);
    length = st.st_size;
#endif
    return length;
}

void usage(void)
{
    printf("useage: exe file\n");
}

void lz4_benchmark(char *data, uint32_t len)
{
    printf("lz4 benchmark:\n");

    uint32_t compress_olen;
    uint32_t decompress_olen;
    char *compress_output = malloc(len);
    char *decompress_output = malloc(len);

    unsigned int iterations = 0;

    double total_compress_time = 0.0;
    double total_decompress_time = 0.0;
    double total_time = 0.0;

    double compress_speed = 0.0;
    int    compress_rate  = 0;

    double decompress_speed = 0.0;

    double compress_time_elapsed = 0.0;
    double decompress_time_elapsed = 0.0;

    cputime_chronometer chrono;

    while (total_time <= BENCHMARK_TEST_TIME) {
        ++iterations;

        cputime_chronometer_start(&chrono);
        compress_olen = LZ4_compress_default(data, compress_output, len, len);
        compress_time_elapsed = cputime_chronometer_stop(&chrono);

        total_compress_time += compress_time_elapsed;
        compress_speed = ((1.0 * len * iterations) / (total_compress_time * 1000.0 * 1000.0));

        total_time += compress_time_elapsed;

        cputime_chronometer_start(&chrono);
        LZ4_decompress_safe(compress_output, decompress_output, len, len);
        decompress_time_elapsed = cputime_chronometer_stop(&chrono);

        total_decompress_time += decompress_time_elapsed;

        decompress_speed = ((1.0 * len * iterations) / (total_decompress_time * 1000.0 * 1000.0));

        total_time += decompress_time_elapsed;

    }

    free(compress_output);
    free(decompress_output);

    compress_rate=compress_olen*100/len;

    printf("\rCompress speed ");
    printf("%.0lf MB/s, rate %d%%", compress_speed, compress_rate);
    
    printf("\n");
    printf("Decompress speed ");
    printf("%.0lf MB/s", decompress_speed);

    printf("\nRun time %.3lfs (%i iterations)\n\n", total_time, iterations);

}

void rvl_benchmark(char *data, uint32_t len)
{
    printf("rvl benchmark:\n");

    uint32_t compress_olen;
    uint32_t decompress_olen;
    char *compress_output = malloc(len);
    char *decompress_output = malloc(len);

    unsigned int iterations = 0;

    double total_compress_time = 0.0;
    double total_decompress_time = 0.0;
    double total_time = 0.0;

    double compress_speed = 0.0;
    int    compress_rate  = 0;

    double decompress_speed = 0.0;

    double compress_time_elapsed = 0.0;
    double decompress_time_elapsed = 0.0;

    cputime_chronometer chrono;

    while (total_time <= BENCHMARK_TEST_TIME) {
        ++iterations;

        cputime_chronometer_start(&chrono);
        compress_olen = rvl_compress(data, compress_output, len);
        compress_time_elapsed = cputime_chronometer_stop(&chrono);

        total_compress_time += compress_time_elapsed;
        compress_speed = ((1.0 * len * iterations) / (total_compress_time * 1000.0 * 1000.0));

        total_time += compress_time_elapsed;

        cputime_chronometer_start(&chrono);
        rvl_decompress(compress_output, decompress_output, len);
        decompress_time_elapsed = cputime_chronometer_stop(&chrono);

        total_decompress_time += decompress_time_elapsed;

        decompress_speed = ((1.0 * len * iterations) / (total_decompress_time * 1000.0 * 1000.0));

        total_time += decompress_time_elapsed;

    }

    free(compress_output);
    free(decompress_output);

    compress_rate=compress_olen*100/len;

    printf("\rCompress speed ");
    printf("%.0lf MB/s, rate %d%%", compress_speed, compress_rate);
    
    printf("\n");
    printf("Decompress speed ");
    printf("%.0lf MB/s", decompress_speed);

    printf("\nRun time %.3lfs (%i iterations)\n\n", total_time, iterations);

}

void rvl_mt_benchmark(char *data, uint32_t len)
{
    printf("rvl mt benchmark:\n");

    uint32_t compress_olen;
    uint32_t decompress_olen;
    char *compress_output = malloc(len);
    char *decompress_output = malloc(len);

    unsigned int iterations = 0;

    double total_compress_time = 0.0;
    double total_decompress_time = 0.0;
    double total_time = 0.0;

    double compress_speed = 0.0;
    int    compress_rate  = 0;

    double decompress_speed = 0.0;

    double compress_time_elapsed = 0.0;
    double decompress_time_elapsed = 0.0;

    cputime_chronometer chrono;

    while (total_time <= BENCHMARK_TEST_TIME) {
        ++iterations;

        cputime_chronometer_start(&chrono);
        compress_olen = rvl_mt_compress(data, compress_output, len);
        compress_time_elapsed = cputime_chronometer_stop(&chrono);

        total_compress_time += compress_time_elapsed;
        compress_speed = ((1.0 * len * iterations) / (total_compress_time * 1000.0 * 1000.0));

        total_time += compress_time_elapsed;

        cputime_chronometer_start(&chrono);
        rvl_mt_decompress(compress_output, decompress_output, len);
        decompress_time_elapsed = cputime_chronometer_stop(&chrono);

        total_decompress_time += decompress_time_elapsed;

        decompress_speed = ((1.0 * len * iterations) / (total_decompress_time * 1000.0 * 1000.0));

        total_time += decompress_time_elapsed;

    }

    free(compress_output);
    free(decompress_output);

    compress_rate=compress_olen*100/len;

    printf("\rCompress speed ");
    printf("%.0lf MB/s, rate %d%%", compress_speed, compress_rate);
    
    printf("\n");
    printf("Decompress speed ");
    printf("%.0lf MB/s", decompress_speed);

    printf("\nRun time %.3lfs (%i iterations)\n\n", total_time, iterations);

}

void shrinker_benchmark(char *data, uint32_t len)
{
    printf("shrinker benchmark:\n");

    uint32_t compress_olen;
    uint32_t decompress_olen;
    char *compress_output = malloc(len);
    char *decompress_output = malloc(len);

    unsigned int iterations = 0;

    double total_compress_time = 0.0;
    double total_decompress_time = 0.0;
    double total_time = 0.0;

    double compress_speed = 0.0;
    int    compress_rate  = 0;

    double decompress_speed = 0.0;

    double compress_time_elapsed = 0.0;
    double decompress_time_elapsed = 0.0;

    cputime_chronometer chrono;

    while (total_time <= BENCHMARK_TEST_TIME) {
        ++iterations;

        cputime_chronometer_start(&chrono);
        compress_olen = shrinker_compress(data, compress_output, len);
        compress_time_elapsed = cputime_chronometer_stop(&chrono);

        total_compress_time += compress_time_elapsed;
        compress_speed = ((1.0 * len * iterations) / (total_compress_time * 1000.0 * 1000.0));

        total_time += compress_time_elapsed;

        cputime_chronometer_start(&chrono);
        shrinker_decompress(compress_output, decompress_output, compress_olen);
        decompress_time_elapsed = cputime_chronometer_stop(&chrono);

        total_decompress_time += decompress_time_elapsed;

        decompress_speed = ((1.0 * len * iterations) / (total_decompress_time * 1000.0 * 1000.0));

        total_time += decompress_time_elapsed;

    }

    free(compress_output);
    free(decompress_output);

    compress_rate=compress_olen*100/len;

    printf("\rCompress speed ");
    printf("%.0lf MB/s, rate %d%%", compress_speed, compress_rate);
    
    printf("\n");
    printf("Decompress speed ");
    printf("%.0lf MB/s", decompress_speed);

    printf("\nRun time %.3lfs (%i iterations)\n\n", total_time, iterations);

}

int main(int argc, char* argv[])
{
	int ret;
    if(argc != 2) {
        usage();
        return -1;
    } else if ( 0 != access(argv[1], F_OK)) {
        usage();
        return -1;
    }

    FILE *fp = fopen(argv[1], "rb");
    uint32_t len = get_file_size(fp);

    char *data = malloc(len);
    ret = fread(data, len, 1, fp);

    lz4_benchmark(data, len);

    rvl_benchmark(data, len);

    rvl_mt_benchmark(data, len);

    //shrinker_benchmark(data, len);

    free(data);

    return 0;
}
