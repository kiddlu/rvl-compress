#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/stat.h>

#include "lz4.h"

unsigned int get_file_size(FILE *fp)
{
   unsigned int length;
#if 0
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
    olen = LZ4_compress_default(data, output, len, len);
    
    fp = fopen(argv[2], "wb+"); 
    fwrite(output, olen, 1, fp);
    fclose(fp);

    return 0;
}

