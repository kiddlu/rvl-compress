static void EncodeVLE(int value, int **pBuffer, int *word, int *nibblesWritten)
{
	do
	{
		int nibble = value & 0x7;       /* lower 3 bits */
		if ( value >>= 3 )
			nibble |= 0x8;          /* more to come */
		(*word)	<<= 4;
		(*word)	|= nibble;
		if ( ++(*nibblesWritten) == 8 )    /* output word */
		{
			*(*pBuffer)++	= (*word);
			(*nibblesWritten)	= 0;
			(*word)		= 0;
		}
	}
	while ( value );
}

static int DecodeVLE(int **pBuffer, int *word, int *nibblesWritten)
{
	unsigned int	nibble;
	int		value = 0, bits = 29;
	do
	{
		if ( !(*nibblesWritten) )
		{
			(*word)		= *(*pBuffer)++; /* load word */
			(*nibblesWritten)	= 8;
		}
		nibble	= (*word) & 0xf0000000;
		value	|= (nibble << 1) >> bits;
		(*word)	<<= 4;
		(*nibblesWritten)--;
		bits -= 3;
	}
	while ( nibble & 0x80000000 );
	return(value);
}

static int CompressRVL( short* input, char* output, int numPixels )
{
	int *buffer = (int *) output;
	int *pBuffer = (int *) output;
	int word = 0;
	int nibblesWritten	= 0;
	short	*end		= input + numPixels;
	short	previous	= 0;
	while ( input != end )
	{
		int zeros = 0, nonzeros = 0;
		for (; (input != end) && !*input; input++, zeros++ )
                ;
		EncodeVLE( zeros, &pBuffer, &word, &nibblesWritten);                             /* number of zeros */
		for ( short* p = input; (p != end) && *p++; nonzeros++ )
			;
		EncodeVLE( nonzeros,  &pBuffer, &word, &nibblesWritten);                          /* number of nonzeros */
		for ( int i = 0; i < nonzeros; i++ )
		{
			short	current		= *input++;
			int	delta		= current - previous;
			int	positive	= (delta << 1) ^ (delta >> 31);
			EncodeVLE( positive, &pBuffer, &word, &nibblesWritten);                  /* nonzero value */
			previous = current;
		}
	}
	if ( nibblesWritten )                                   /* last few values */
		*pBuffer++ = word << 4 * (8 - nibblesWritten);

	return(((char *)pBuffer - (char*)buffer));      /* num bytes */
}

static void DecompressRVL( char* input, short* output, int numPixels )
{
	int *buffer = (int *) input;
	int *pBuffer = (int *) input;
	int word = 0;
	int nibblesWritten	= 0;
	short	current, previous = 0;
	int	numPixelsToDecode = numPixels;
	while ( numPixelsToDecode )
	{
		int zeros = DecodeVLE(&pBuffer, &word, &nibblesWritten);                        /* number of zeros */
		numPixelsToDecode -= zeros;
		for (; zeros; zeros-- )
			*output++ = 0;
		int nonzeros = DecodeVLE(&pBuffer, &word, &nibblesWritten);                     /* number of nonzeros */
		numPixelsToDecode -= nonzeros;
		for (; nonzeros; nonzeros-- )
		{
			int	positive	= DecodeVLE(&pBuffer, &word, &nibblesWritten);  /* nonzero value */
			int	delta		= (positive >> 1) ^ -(positive & 1);
			current		= previous + delta;
			*output++	= current;
			previous	= current;
		}
	}
}

int rvl_compress(char *src, char *dst, int orin_size)
{
    short *input = (short *)src;
    char *output = dst;
    int numPixels = orin_size / sizeof(short);

    return CompressRVL(input, output, numPixels);
}

int rvl_decompress(char *src, char *dst, int orin_size)
{
    char *input = src;
    short *output = (short *)dst;
    int numPixels = orin_size / sizeof(short);

    DecompressRVL(input, output, numPixels);

    return 0;
}

/* multithread support */
#define DEF_MT_NR	4
#define MAX_MT_NR	8

struct rvl_mt_info
{
    int  mt_nr;
    int  mt_size[MAX_MT_NR];
    char reserved[128 - 4*(MAX_MT_NR+1)];
};

struct rvl_mt_args
{
    char *input;
    char *output;
    int numPixels;
};

int rvl_mt_compress(char *src, char *dst, int orin_size)
{
    char *input = src;
    char *output = dst;
    int numPixels = orin_size / sizeof(short);
    int total_size = 0;

    struct rvl_mt_info *info =(struct rvl_mt_info *)output;
    info->mt_nr = DEF_MT_NR;

    output += sizeof(struct rvl_mt_info);
    total_size += sizeof(struct rvl_mt_info);

    for(int i = 0; i < info->mt_nr; i++) {
        info->mt_size[i] = CompressRVL((short *)input, output, numPixels/info->mt_nr);
        input  += orin_size / info->mt_nr;
        output += info->mt_size[i];
        total_size += info->mt_size[i];
    }

    return total_size;
}

#if 0
int rvl_mt_decompress(char *src, char *dst, int orin_size)
{
    char *input = src;
    char *output = dst;
    int numPixels = orin_size / sizeof(short);

    struct rvl_mt_info *info =(struct rvl_mt_info *)input;
    input += sizeof(struct rvl_mt_info);

    for(int i = 0; i < info->mt_nr; i++) {
        DecompressRVL(input, (short *)output, numPixels/info->mt_nr);
        input  += info->mt_size[i];
        output += orin_size/info->mt_nr;
    }
    return 0;
}
#else
#include <pthread.h>

void *rvl_mt_decomp_thread(void *arg)
{
    struct rvl_mt_args *args = (struct rvl_mt_args *)arg;

    DecompressRVL(args->input, (short *)args->output, args->numPixels);

    return NULL;
}

int rvl_mt_decompress(char *src, char *dst, int orin_size)
{
    char *input = src;
    char *output = dst;
    int numPixels = orin_size / sizeof(short);

    struct rvl_mt_info *info =(struct rvl_mt_info *)input;
    input += sizeof(struct rvl_mt_info);

    pthread_t thread_id[MAX_MT_NR];
    struct rvl_mt_args args[MAX_MT_NR];

    for(int i=0; i < info->mt_nr; i++) {
        args[i].input = input;
        args[i].output = output;
        args[i].numPixels = numPixels/info->mt_nr;

        input  += info->mt_size[i];
        output += orin_size/info->mt_nr;
    }

    for(int i=0; i < info->mt_nr; i++) {
        pthread_create(&(thread_id[i]), NULL,
            rvl_mt_decomp_thread, (void*)&(args[i]));
    }

    for(int i=0; i < info->mt_nr; i++) {
        pthread_join(thread_id[i], NULL);
    }

    return 0;
}
#endif
