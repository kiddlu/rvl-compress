#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

typedef struct __attribute__ ((__packed__)) tagBITMAPFILEHEADER {
        uint16_t    bfType;
        uint32_t   bfSize;
        uint16_t    bfReserved1;
        uint16_t    bfReserved2;
        uint32_t   bfOffBits;
} BITMAPFILEHEADER;

typedef struct __attribute__ ((__packed__)) tagBITMAPINFOHEADER{
        uint32_t      biSize;
        int32_t       biWidth;
        int32_t       biHeight;
        uint16_t       biPlanes;
        uint16_t       biBitCount;
        uint32_t      biCompression;
        uint32_t      biSizeImage;
        int32_t       biXPelsPerMeter;
        int32_t       biYPelsPerMeter;
        uint32_t      biClrUsed;
        uint32_t      biClrImportant;
} BITMAPINFOHEADER;

int depth2bmp_hist(uint16_t *depth_img, uint8_t *bmp_img, int width, int height)
{
    int histSize = 9000;
    uint16_t *depth_img_orig = depth_img;

    for(int i = 0; i < width * height ; ++i)
    {
        depth_img[i] = depth_img[i] >> 3;
    }

	unsigned int nPointsCount = 0;

	float *histgram = malloc(histSize*sizeof(float));
	memset(histgram, 0, histSize*sizeof(float));

	for(int y = 0; y < height; ++y)
	{
		for(int x = 0; x < width; ++x, ++depth_img)
		{
			if(*depth_img != 0)
			{
				histgram[*depth_img]++;
				nPointsCount++;
			}
		}
	}

	for(int nIndex = 1; nIndex < histSize; ++nIndex)
	{
        histgram[nIndex] += histgram[nIndex - 1];
	}

	if(nPointsCount)
	{
		for(int nIndex = 1; nIndex < histSize; ++nIndex)
		{
			histgram[nIndex] = (256 * (1.0f - (histgram[nIndex] / nPointsCount)));
		}
	}

		
    BITMAPFILEHEADER bmp_header;  
    BITMAPINFOHEADER bmp_hinfo;  
    bmp_header.bfType = 0x4D42;  
    bmp_header.bfSize =(uint32_t)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + width* height * 3);  
    bmp_header.bfReserved1 = 0;  
    bmp_header.bfReserved2 = 0;  
    bmp_header.bfOffBits = (uint32_t)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)) ;  
    bmp_hinfo.biSize = sizeof(BITMAPINFOHEADER);  
    bmp_hinfo.biWidth = width;  
    bmp_hinfo.biHeight = height;  
    bmp_hinfo.biPlanes = 1;  
    bmp_hinfo.biBitCount = 24;  
    bmp_hinfo.biCompression = 0;  
    bmp_hinfo.biSizeImage = 4 * width * height;  
    bmp_hinfo.biXPelsPerMeter = 0;  
    bmp_hinfo.biYPelsPerMeter = 0;  
    bmp_hinfo.biClrUsed = 0;  
    bmp_hinfo.biClrImportant = 0; 
 
	memcpy(bmp_img, &bmp_header, sizeof(BITMAPFILEHEADER));
 	memcpy(bmp_img + sizeof(BITMAPFILEHEADER), &bmp_hinfo, sizeof(BITMAPINFOHEADER));

		
	uint8_t *bmp_img_body = bmp_img + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		
    for(int i = 0, j =0; i < width * height * 3; i += 3, j += 1) 
    {
             *(bmp_img_body+i+0) = 0;
             *(bmp_img_body+i+1) = histgram[depth_img_orig[j]];
             *(bmp_img_body+i+2) = histgram[depth_img_orig[j]];
    }

	return (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + width*height*3);
}
