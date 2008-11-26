/*
 * misc.c
 * 
 * This is a collection of several routines from gzip-1.0.3 
 * adapted for Linux.
 *
 * malloc by Hannu Savolainen 1993 and Matthias Urlichs 1994
 *
 * Modified for ARM Linux by Russell King
 *
 * Nicolas Pitre <nico@visuaide.com>  1999/04/14 :
 *  For this code to run directly from Flash, all constant variables must
 *  be marked with 'const' and all other variables initialized at run-time 
 *  only.  This way all non constant variables will end up in the bss segment,
 *  which should point to addresses in RAM and cleared to 0 on start.
 *  This allows for a much quicker boot time.
 */

unsigned int __machine_arch_type;


#ifdef STANDALONE_DEBUG
#define putstr printf
#else



#include <linux/compiler.h>
#include <asm/arch/uncompress.h>




static void puts(char *buf)
{
int i=0;
while (buf[i]!=0)
    {
    putc(buf[i]);
    if (buf[i] == '\n') putc('\r');
    }
}



#endif

#define __ptr_t void *



/*
 * gzip delarations
 */
#define OF(args)  args
#define STATIC static

typedef unsigned char  uch;
typedef unsigned short ush;
typedef unsigned long  ulg;

#define WSIZE 0x8000		/* Window size must be at least 32k, */
				/* and a power of two */

static uch *inbuf;		/* input buffer */
static uch window[WSIZE];	/* Sliding window buffer */

static unsigned insize=0;		/* valid bytes in inbuf */
static unsigned inptr=0;		/* index of next byte to be processed in inbuf */
static unsigned outcnt;		/* bytes in output buffer */

/* gzip flag byte */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define CONTINUATION 0x02 /* bit 1 set: continuation of multi-part gzip file */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define ENCRYPTED    0x20 /* bit 5 set: file is encrypted */
#define RESERVED     0xC0 /* bit 6,7:   reserved */

#define get_byte()  (inptr < insize ? inbuf[inptr++] : fill_inbuf())

/* Diagnostic functions */
#ifdef DEBUG
#  define Assert(cond,msg) {if(!(cond)) error(msg);}
#  define Trace(x) fprintf x
#  define Tracev(x) {if (verbose) fprintf x ;}
#  define Tracevv(x) {if (verbose>1) fprintf x ;}
#  define Tracec(c,x) {if (verbose && (c)) fprintf x ;}
#  define Tracecv(c,x) {if (verbose>1 && (c)) fprintf x ;}
#else
#  define Assert(cond,msg)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#endif

static int  fill_inbuf(void);
static void flush_window(void);
static void error(char *m);

extern char input_data[];
extern char input_data_end[];

static ulg output_ptr=0;
static uch *output_data;
static ulg bytes_out;


extern int end;
static ulg free_mem_ptr;
static ulg free_mem_ptr_end;

#define _LZMA_IN_CB
#include "LzmaDecode.h"
#include "LzmaDecode.c"

static int read_byte(void *object, const unsigned char **buffer, SizeT *bufferSize);


/*
 * Do the lzma decompression
 */
static int lzma_unzip(void)
{

	unsigned int i;
        CLzmaDecoderState state;
	unsigned int uncompressedSize = 0;
        
        ILzmaInCallback callback;
        callback.Read = read_byte;

	// lzma args
	i = get_byte();
	state.Properties.lc = i % 9, i = i / 9;
        state.Properties.lp = i % 5, state.Properties.pb = i / 5;
        
        // skip dictionary size
        for (i = 0; i < 4; i++) 
        	get_byte();
        // get uncompressed size
	uncompressedSize = (get_byte()) +
		(get_byte() << 8) +
		(get_byte() << 16) +
		(get_byte() << 24);
            
        // skip high order bytes
        for (i = 0; i < 4; i++) 
        	get_byte();
        // point it beyond uncompresedSize
        state.Probs = (CProb*) (output_data + uncompressedSize);
	// decompress kernel
	if (LzmaDecode( &state, &callback,
	   (unsigned char*)output_data, uncompressedSize, &i) == LZMA_RESULT_OK)
	{
		if ( i != uncompressedSize )
		   error( "kernel corrupted!\n");
		//copy it back to low_buffer
		bytes_out = i;
		output_ptr = i;
		return 0;
	}
	return 1;
}


static unsigned int icnt = 0;
static int read_byte(void *object, const unsigned char **buffer, SizeT *bufferSize)
{
	static unsigned char val;
	*bufferSize = 1;
	val = get_byte();
	*buffer = &val;
        if ( icnt++ % ( 1024 * 10 ) == 0 )
               putc('.');
	return LZMA_RESULT_OK;
}	




/* ===========================================================================
 * Fill the input buffer. This is called only when the buffer is empty
 * and at least one byte is really needed.
 */
 

static int fill_inbuf(void)
{
	if (insize != 0)
		error("ran out of input data");

	inbuf = input_data;
	insize = &input_data_end[0] - &input_data[0];

	inptr = 1;
	return inbuf[0];
}

/* ===========================================================================
 * Write the output window window[0..outcnt-1] and update crc and bytes_out.
 * (Used for the decompressed data only.)
 */

#ifndef arch_error
#define arch_error(x)
#endif

static void error(char *x)
{
	arch_error(x);

	puts("\n\n");
	puts(x);
	puts("\n\n -- System halted");

	while(1);	/* Halt */
}

void __div0(void)
{
	error("division by zero");
}

#ifndef STANDALONE_DEBUG

ulg
decompress_kernel(ulg output_start, ulg free_mem_ptr_p, ulg free_mem_ptr_end_p,
		  int arch_id)
{
	output_data		= (uch *)output_start;	/* Points to kernel start */
	free_mem_ptr		= free_mem_ptr_p;
	free_mem_ptr_end	= free_mem_ptr_end_p;
	__machine_arch_type	= arch_id;

	arch_decomp_setup();

	char num[5];
	num[0]=((arch_id>>24)&0xff)+'0';
	num[1]=((arch_id>>16)&0xff)+'0';
	num[2]=((arch_id>>8)&0xff)+'0';
	num[3]=(arch_id&0xff)+'0';
	num[4]=0;
	puts("DD-WRT LZMA Loader v1.0\nArch ID is 0x");
	puts(num);
	puts("\nUncompressing Linux");
	lzma_unzip();
	puts("\ndone, booting the kernel.\n");
	return output_ptr;
}
#else

char output_buffer[1500*1024];

int main()
{
	output_data = output_buffer;
	printf("Uncompressing Linux...");
	lzma_unzip();
	printf("done.\n");
	return 0;
}
#endif
	
