#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/stat.h>

#include "./algo/rvl.h"
#include "./algo/lz4.h"

#include "package.h"

uint32_t package_shrink(char *in, uint32_t ilen, char *out, uint32_t olen)
{
	if(in == NULL || out == NULL || ilen == 0) {
		return 0;
	}

	char *content = out + PACKAGE_HEADER_SIZE;
	struct package_header *header = (struct package_header *)out;

    header->header_ver = PACKAGE_HEADER_VERSION;
    header->magic_num =  PACKAGE_DEPTH_IMAGE_MAGIC_NUM;
#if 0
    extern int os_time_get(uint32_t *tv_sec, uint32_t *tv_usec);
    os_time_get(&(header->timestamp_sec), &(header->timestamp_usec));
#else
    header->timestamp_sec = 0;
    header->timestamp_usec = 0;
#endif
    header->origin_size  = ilen;
    header->origin_csum  = package_header_csum(in, ilen);

#if 0
    header->content_format = PACKAGE_CONTENT_FORMAT_LZ4;
    header->content_size = LZ4_compress_default(in, content, ilen, olen - PACKAGE_HEADER_SIZE);
#else
    header->content_format = PACKAGE_CONTENT_FORMAT_RVL;
    header->content_size = rvl_mt_compress(in, content, ilen);
#endif

    header->content_csum = package_header_csum(content, header->content_size);

    return ((header->content_size) + PACKAGE_HEADER_SIZE);
}

