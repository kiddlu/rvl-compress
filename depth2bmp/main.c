#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

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

extern int depth2bmp_hist(uint16_t *depth_img, uint8_t *bmp_img,int width, int height);

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
    char *output = malloc(2*320* 200 * 4);


    olen = depth2bmp_hist(data, output, 320, 200);

    fp = fopen(argv[2], "wb+"); 
    fwrite(output, olen, 1, fp);
    fclose(fp);

    return 0;
}

