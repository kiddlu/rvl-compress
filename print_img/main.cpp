#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

#include "print_img.h"

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
    printf("useage: exe in\n");
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
    if(argc != 2) {
        usage();
        return -1;
    } else if ( 0 != access(argv[1], F_OK)) {
        usage();
        return -1;
    }

    FILE *fp = fopen(argv[1], "rb");
    uint32_t len = get_file_size(fp);

    char *data = (char *)malloc(len);
    fread(data, len, 1, fp);
#if 0
    printf("\n");
    printf("hexdump of file %s (%d bytes):\n", argv[1], len);

    memdump(data, len);
#endif

    uint32_t olen;
    char *output = (char *)malloc(len*2);

    print_img(data, len, 0, 0, 0);

    return 0;
}

