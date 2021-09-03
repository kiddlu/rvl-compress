#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/stat.h>

#include "zstd.h"
#include "header.h"

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
    printf("useage: exe in out\n");
}

void memdump(void *addr, uint32_t size)
{
    int index;

    while (size > 0) {
        fprintf(stdout, "%p: ", addr);
        index = 0;

        // 32 bytes in the line
        while (index < 32) {
            fprintf(stdout, "%02x ", *((unsigned char *)addr));

            index++;
            addr++;
            size--;

            if (size <= 0)
                break;
        }
        fprintf(stdout, "\n");
    }
    fprintf(stdout, "\n");
    return;
}

void print_header_info(struct depth_header *h)
{
    if(h == NULL){
        return;
        printf("header null\n");
    } else if(h->magic_num != HEADER_MAGIC_NUM) {
        printf("h->magic_num 0x%08x error\n", h->magic_num);
        return;
    }

    printf("content_size: %d Bytes\n", h->content_size);
    printf("origin_size : %d Bytes\n", h->origin_size);
    
    printf("content_format:  0x%02x\n", h->content_format);
    printf("content_csum  :  0x%02x\n", h->content_csum);
    printf("origin_csum   :  0x%02x\n", h->origin_csum);
    printf("header_ver    :  0x%02x\n", h->header_ver);

}


int main(int argc, char* argv[])
{
    if(argc != 3) {
        usage();
        return -1;
    } else if ( 0 != access(argv[1], F_OK)) {
        usage();
        return -1;
    }

    FILE *fp = fopen(argv[1], "rb");

	struct depth_header *h = malloc(PACKAGE_HEADER_SIZE);
    fread(h, PACKAGE_HEADER_SIZE, 1, fp);
    print_header_info(h);
 
    uint32_t ilen = h->content_size;
    char *ibuf = malloc(ilen);
    fread(ibuf, ilen, 1, fp);
 
    uint32_t olen = h->origin_size;
    char *output = malloc(olen);

    if(h->content_format == CONTENT_FORMAT_ZSTD) {
        ZSTD_decompress(output, olen, ibuf, ilen);
    }

    printf("\n");
    printf("Calc content_csum  :  0x%02x\n", header_csum(ibuf, ilen));
    printf("Calc origin_csum   :  0x%02x\n", header_csum(output, olen));

    fp = fopen(argv[2], "wb+"); 
    fwrite(output, olen, 1, fp);
    fclose(fp);

    return 0;
}

