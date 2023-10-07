#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/stat.h>

#include "./algo/rvl.h"
#include "./algo/lz4.h"

#include "package.h"

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

int main(int argc, char* argv[])
{
	int ret;
    if(argc != 3) {
        usage();
        return -1;
    } else if ( 0 != access(argv[1], F_OK)) {
        usage();
        return -1;
    }

    FILE *fp = fopen(argv[1], "rb");

	struct package_header *h = malloc(PACKAGE_HEADER_SIZE);
    ret = fread(h, PACKAGE_HEADER_SIZE, 1, fp);
    print_header_info(h);

    uint32_t ilen = h->content_size;
    char *ibuf = malloc(ilen);
    ret = fread(ibuf, ilen, 1, fp);

    uint32_t olen = h->origin_size;
    char *output = malloc(olen);

    if(h->content_format == PACKAGE_CONTENT_FORMAT_LZ4) {
        LZ4_decompress_safe(ibuf, output, ilen, olen);
    }
    else if(h->content_format == PACKAGE_CONTENT_FORMAT_RVL) {
        rvl_mt_decompress(ibuf, output, olen);
    }

    printf("\n");
    printf("Calc content_csum  :  0x%02x\n", package_header_csum(ibuf, ilen));
    printf("Calc origin_csum   :  0x%02x\n", package_header_csum(output, olen));

    fp = fopen(argv[2], "wb+"); 
    ret = fwrite(output, olen, 1, fp);
    fclose(fp);

    return 0;
}

