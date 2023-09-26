#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/stat.h>


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



void dumphex(void *data, unsigned int size)
{
	char ascii[17];
	unsigned int i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		printf("%02X ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[i];
		} else {
			ascii[i % 16] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			printf(" ");
			if ((i+1) % 16 == 0) {
				printf("|  %s \n", ascii);
			} else if (i+1 == size) {
				ascii[(i+1) % 16] = '\0';
				if ((i+1) % 16 <= 8) {
					printf(" ");
				}
				for (j = (i+1) % 16; j < 16; ++j) {
					printf("   ");
				}
				printf("|  %s \n", ascii);
			}
		}
	}
}

#define COLOR_TIMESTAMP_OFFSET    (0x80)

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

    int ilen = get_file_size(fp);
    char *ibuf = malloc(ilen);
    ret = fread(ibuf, ilen, 1, fp);

    uint64_t timestamp = *(uint64_t *)(ibuf + COLOR_TIMESTAMP_OFFSET);
    printf("timestamp     :  %"PRIu64"\n", timestamp);

/*
	dumphex(ibuf, mjpeg_header_len+8);

    *(uint64_t *)(ibuf + mjpeg_header_len + 4 - 8) =  (uint64_t)0x99112233445566FF;
    timestamp = *(uint64_t *)(ibuf + mjpeg_header_len + 4 - 8);
    printf("timestamp     :  %"PRIu64"\n", timestamp);

	dumphex(ibuf, mjpeg_header_len+8);
*/
    return 0;
}

