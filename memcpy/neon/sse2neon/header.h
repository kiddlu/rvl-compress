#ifndef __HEADER__H
#define __HEADER__H

#include <inttypes.h>

#define PACKAGE_HEADER_SIZE (256)

#define PACKAGE_HEADER_MAGIC_NUM        (0x49445842)

#define PACKAGE_CONTENT_FORMAT_DATA     (0x01)
#define PACKAGE_CONTENT_FORMAT_LZ4      (0x02)
#define PACKAGE_CONTENT_FORMAT_ZSTD     (0x03)

#define PACKAGE_HEADER_VERSION          (0x01)

struct depth_header
{
    uint32_t magic_num;

    uint32_t content_size;

    uint32_t origin_size;

    uint8_t  content_format;
    uint8_t  content_csum;
    uint8_t  origin_csum;
    uint8_t  header_ver;

	//option
    uint32_t img_width;
    uint32_t img_height;
    uint32_t pixel_byte;

    uint8_t  reserved[PACKAGE_HEADER_SIZE - 28];
}__attribute__((packed));


static inline uint8_t header_csum(char *ptr, int sz)
{
    uint8_t chk = 0;
    while (sz-- != 0) {
        chk -= (uint8_t)*ptr++;
	}
    return chk;
}

#endif
