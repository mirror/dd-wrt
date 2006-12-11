/*
 * lzma_misc.c
 * 
 * Decompress LZMA compressed vmlinuz 
 * Version 0.9 Copyright (c) Ming-Ching Tiew mctiew@yahoo.com
 * Program adapted from misc.c for 2.6 kernel
 * Date: 3 June 2005 
 * Source released under GPL
 */

#include <linux/linkage.h>
#include <linux/vmalloc.h>
#include <linux/tty.h>
#include <asm/io.h>

#define OF(args)  args
#define STATIC static

#undef memset
#undef memcpy

/*
 * Why do we do this? Don't ask me..
 *
 * Incomprehensible are the ways of bootloaders.
 */
static void* memcpy(void *, __const void *, size_t);

typedef unsigned char  uch;
typedef unsigned short ush;
typedef unsigned long  ulg;

#define WSIZE 0x8000		/* Window size must be at least 32k, */
				/* and a power of two */

static uch *inbuf;	     /* input buffer */

static unsigned insize = 0;  /* valid bytes in inbuf */
static unsigned inptr = 0;   /* index of next byte to be processed in inbuf */

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
static void error(char *m);
  
/*
 * This is set up by the setup-routine at boot-time
 */
static unsigned char *real_mode; /* Pointer to real-mode data */

#define RM_EXT_MEM_K   (*(unsigned short *)(real_mode + 0x2))
#ifndef STANDARD_MEMORY_BIOS_CALL
#define RM_ALT_MEM_K   (*(unsigned long *)(real_mode + 0x1e0))
#endif
#define RM_SCREEN_INFO (*(struct screen_info *)(real_mode+0))

extern char input_data[];
extern int input_len;

static long bytes_out = 0;
static uch *output_data;

static void putstr(const char *);

extern int end;
static long free_mem_ptr = (long)&end;
static long free_mem_end_ptr;

#define INPLACE_MOVE_ROUTINE  0x1000
#define LOW_BUFFER_START      0x2000
#define LOW_BUFFER_MAX       0x90000
#define HEAP_SIZE             0x3000
static unsigned int low_buffer_end, low_buffer_size;
static int high_loaded =0;
static uch *high_buffer_start /* = (uch *)(((ulg)&end) + HEAP_SIZE)*/;

static char *vidmem = (char *)0xb8000;
static int vidport;
static int lines, cols;

static void scroll(void)
{
	int i;

	memcpy ( vidmem, vidmem + cols * 2, ( lines - 1 ) * cols * 2 );
	for ( i = ( lines - 1 ) * cols * 2; i < lines * cols * 2; i += 2 )
		vidmem[i] = ' ';
}

static void putstr(const char *s)
{
	int x,y,pos;
	char c;

	x = RM_SCREEN_INFO.orig_x;
	y = RM_SCREEN_INFO.orig_y;

	while ( ( c = *s++ ) != '\0' ) {
		if ( c == '\n' ) {
			x = 0;
			if ( ++y >= lines ) {
				scroll();
				y--;
			}
		} else {
			vidmem [ ( x + cols * y ) * 2 ] = c; 
			if ( ++x >= cols ) {
				x = 0;
				if ( ++y >= lines ) {
					scroll();
					y--;
				}
			}
		}
	}

	RM_SCREEN_INFO.orig_x = x;
	RM_SCREEN_INFO.orig_y = y;

	pos = (x + cols * y) * 2;	/* Update cursor position */
	outb_p(14, vidport);
	outb_p(0xff & (pos >> 9), vidport+1);
	outb_p(15, vidport);
	outb_p(0xff & (pos >> 1), vidport+1);
}

static void* memcpy(void* __dest, __const void* __src,
			    size_t __n)
{
	int i;
	char *d = (char *)__dest, *s = (char *)__src;

	for (i=0;i<__n;i++) d[i] = s[i];
	return __dest;
}

/* ===========================================================================
 * Fill the input buffer. This is called only when the buffer is empty
 * and at least one byte is really needed.
 */
static int fill_inbuf(void)
{
	if (insize != 0) {
		error("ran out of input data");
	}

	inbuf = input_data;
	insize = input_len;
	inptr = 1;
	return inbuf[0];
}

static void error(char *x)
{
	putstr("\n\n");
	putstr(x);
	putstr("\n\n -- System halted");

	while(1);	/* Halt */
}

#define STACK_SIZE (4096)

long user_stack [STACK_SIZE];

struct {
	long * a;
	short b;
	} stack_start = { & user_stack [STACK_SIZE] , __BOOT_DS };

static void setup_normal_output_buffer(void)
{
#ifdef STANDARD_MEMORY_BIOS_CALL
	if (RM_EXT_MEM_K < 1024) error("Less than 2MB of memory");
#else
	if ((RM_ALT_MEM_K > RM_EXT_MEM_K ? RM_ALT_MEM_K : RM_EXT_MEM_K) < 1024) error("Less than 2MB of memory");
#endif
	output_data = (char *)0x100000; /* Points to 1M */
	free_mem_end_ptr = (long)real_mode;
}

struct moveparams {
	uch *low_buffer_start;  int lcount;
	uch *high_buffer_start; int hcount;
};

static void setup_output_buffer_if_we_run_high(struct moveparams *mv)
{
	high_buffer_start = (uch *)(((ulg)&end) + HEAP_SIZE);
#ifdef STANDARD_MEMORY_BIOS_CALL
	if (RM_EXT_MEM_K < (3*1024)) error("Less than 4MB of memory");
#else
	if ((RM_ALT_MEM_K > RM_EXT_MEM_K ? RM_ALT_MEM_K : RM_EXT_MEM_K) <
			(3*1024))
		error("Less than 4MB of memory");
#endif	
	mv->low_buffer_start = output_data = (char *)LOW_BUFFER_START;
	low_buffer_end = ((unsigned int)real_mode > LOW_BUFFER_MAX
	  ? LOW_BUFFER_MAX : (unsigned int)real_mode) & ~0xfff;
	low_buffer_size = low_buffer_end - LOW_BUFFER_START;
	high_loaded = 1;
	free_mem_end_ptr = (long)high_buffer_start;
	if ( (0x100000 + low_buffer_size) > ((ulg)high_buffer_start)) {
		high_buffer_start = (uch *)(0x100000 + low_buffer_size);
		mv->hcount = 0; /* say: we need not to move high_buffer */
	}
	else mv->hcount = -1;
	mv->high_buffer_start = high_buffer_start;
}

static void close_output_buffer_if_we_run_high(struct moveparams *mv)
{
	if (bytes_out > low_buffer_size) {
		mv->lcount = low_buffer_size;
		if (mv->hcount)
			mv->hcount = bytes_out - low_buffer_size;
	} else {
		mv->lcount = bytes_out;
		mv->hcount = 0;
	}
}

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
	unsigned char* p;
        
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
        p= (char*)&uncompressedSize;	
        for (i = 0; i < 4; i++) 
            *p++ = get_byte();
            
        // skip high order bytes
        for (i = 0; i < 4; i++) 
        	get_byte();
        	
        // point it beyond uncompresedSize
        state.Probs = (CProb*) (high_buffer_start + uncompressedSize);
	// decompress kernel
	if (LzmaDecode( &state, &callback,
	   (unsigned char*)high_buffer_start, uncompressedSize, &i) == LZMA_RESULT_OK)
	{
		if ( i != uncompressedSize )
		   error( "kernel corrupted!\n");
		//copy it back to low_buffer
	 	if( uncompressedSize > low_buffer_size )
	 	{
	 	    memcpy((char*)LOW_BUFFER_START, high_buffer_start, low_buffer_size);
	 	    memcpy(high_buffer_start, high_buffer_start+low_buffer_size, 
	 		uncompressedSize-low_buffer_size);	
	        }
	        else 		
	 	    memcpy((char*)LOW_BUFFER_START, high_buffer_start, uncompressedSize );
		bytes_out = i;
		return 0;
	}
	return 1;
}


static int read_byte(void *object, const unsigned char **buffer, SizeT *bufferSize)
{
	static unsigned int i = 0;
	static unsigned char val;
	*bufferSize = 1;
	val = get_byte();
	*buffer = &val;
        if ( i++ % ( 1024 * 50 ) == 0 )
               putstr(".");
	return LZMA_RESULT_OK;
}	

asmlinkage int decompress_kernel(struct moveparams *mv, void *rmode)
{
	real_mode = rmode;

	if (RM_SCREEN_INFO.orig_video_mode == 7) {
		vidmem = (char *) 0xb0000;
		vidport = 0x3b4;
	} else {
		vidmem = (char *) 0xb8000;
		vidport = 0x3d4;
	}

	lines = RM_SCREEN_INFO.orig_video_lines;
	cols = RM_SCREEN_INFO.orig_video_cols;

	if (free_mem_ptr < 0x100000) setup_normal_output_buffer();
	else setup_output_buffer_if_we_run_high(mv);

	putstr("DD-WRT Kernel ...");
	if( lzma_unzip() != 0 )
	{
	   error("inflate error\n");
	}
	putstr("Ok, booting the kernel.\n");
	if (high_loaded) close_output_buffer_if_we_run_high(mv);
	return high_loaded;
}
