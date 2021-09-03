#ifndef __PACKAGE__H
#define __PACKAGE__H

#include <inttypes.h>

#define PACKAGE_HEADER_SIZE (64)

#define PACKAGE_CONTENT_FORMAT_DATA     (0x01)
#define PACKAGE_CONTENT_FORMAT_LZ4      (0x02)
#define PACKAGE_CONTENT_FORMAT_ZSTD     (0x03)

#define PACKAGE_HEADER_VERSION          (0x01)

struct package_header
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

static inline uint8_t package_header_csum(char *ptr, int sz)
{
    uint8_t chk = 0;
    while (sz-- != 0) {
        chk -= (uint8_t)*ptr++;
	}
    return chk;
}


static inline void print_header_info(struct package_header *h)
{
    if(h == NULL){
        return;
        printf("header null\n");
    }

    printf("h->magic_num 0x%08x\n", h->magic_num);
    printf("content_size: %d Bytes\n", h->content_size);
    printf("origin_size : %d Bytes\n", h->origin_size);
    printf("content_format:  0x%02x\n", h->content_format);
    printf("content_csum  :  0x%02x\n", h->content_csum);
    printf("origin_csum   :  0x%02x\n", h->origin_csum);
    printf("header_ver    :  0x%02x\n", h->header_ver);

    printf("package_rate  :  %d%%\n", 
    (h->content_size + PACKAGE_HEADER_SIZE) * 100 / h->origin_size );

}

#define PACKAGE_DEPTH_IMAGE_MAGIC_NUM       (0x49445842)

uint32_t package_shrink(char *in, uint32_t ilen, char *out, uint32_t olen);

#endif
