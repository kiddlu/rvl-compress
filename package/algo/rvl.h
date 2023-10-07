#ifndef _RVL_H_
#define _RVL_H_
int rvl_compress(const char *src, char *dst, int orin_size);

int rvl_decompress(const char *src, char *dst, int orin_size);

int rvl_mt_compress(const char *src, char *dst, int orin_size);

int rvl_mt_decompress(char *src, char *dst, int orin_size);
#endif
