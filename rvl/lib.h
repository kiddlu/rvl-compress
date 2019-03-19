#ifndef __LIB_H__
#define __LIB_H__
void EncodeVLE( int value );
int DecodeVLE();
int CompressRVL( short* input, char* output, int numPixels );
void DecompressRVL( char* input, short* output, int numPixels );
#endif
