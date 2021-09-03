#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/stat.h>

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
    uint32_t len = get_file_size(fp);

    char *data = malloc(len);
    ret = fread(data, len, 1, fp);

    uint32_t olen;
    char *output = malloc(len + PACKAGE_HEADER_SIZE);

	olen = package_shrink(data, len, output, len + PACKAGE_HEADER_SIZE);
    print_header_info((struct package_header *)output);

    fp = fopen(argv[2], "wb+");
    ret = fwrite(output, olen, 1, fp);
    fclose(fp);

    return 0;
}

