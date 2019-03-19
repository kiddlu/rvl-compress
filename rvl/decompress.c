#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

#include "lib.h"

unsigned int get_file_size(FILE *fp)
{
   unsigned int length;

    fseek(fp, 0L, SEEK_END);
    length = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

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
    uint32_t ilen = get_file_size(fp);

    char *ibuf = malloc(ilen);
    fread(ibuf, ilen, 1, fp);

    uint32_t olen = 640 *480 * sizeof(short);
    short *output = malloc(olen);

    DecompressRVL(ibuf, output, 640*480);
    
    fp = fopen(argv[2], "wb+"); 
    fwrite(output, olen, 1, fp);
    fclose(fp);

    return 0;
}

