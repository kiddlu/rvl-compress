#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/stat.h>

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
    } else if(h->magic_num != PACKAGE_HEADER_MAGIC_NUM) {
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
    uint32_t len = get_file_size(fp);

    char *data = malloc(len);
    fread(data, len, 1, fp);

#if 0
    printf("\n");
    printf("hexdump of file %s (%d bytes):\n", argv[1], len);

    memdump(data, len);
#endif

    uint32_t olen;
    char *output = malloc(len);
	
	for(int i=0;i<3000;i++)
    memcpy(output, data, len);
    olen = len;

	struct depth_header *header = malloc(PACKAGE_HEADER_SIZE);
    header->magic_num =  PACKAGE_HEADER_MAGIC_NUM;
    header->content_size = olen;
    header->origin_size  = len;
    header->content_format = PACKAGE_CONTENT_FORMAT_DATA;
    header->content_csum = header_csum(output, olen);
    header->origin_csum  = header_csum(data, len);
    header->header_ver = PACKAGE_HEADER_VERSION;
    print_header_info(header);

    fp = fopen(argv[2], "wb+");
    fwrite(header, PACKAGE_HEADER_SIZE, 1, fp);
    fwrite(output, olen, 1, fp);
    fclose(fp);

    return 0;
}

