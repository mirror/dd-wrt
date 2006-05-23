/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *   Code to un-compress linux image
 *
 *  Copyright 2004 IDT Inc. (rischelp@idt.com)
 *         
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 **************************************************************************
 * May 2004 rkt
 *
 * Based on Johannes Stezenbach's (js@convergence.de) code.
 *
 * 
 *
 **************************************************************************
 */

#include <linux/types.h>

/*
 * gzip declarations
 */
#define OF(args)  args
#define STATIC static
#define memzero(s, n)     memset ((s), 0, (n))
typedef unsigned char uch;
typedef unsigned short ush;
typedef unsigned long ulg;
#define WSIZE 0x8000		/* Window size must be at least 32k, */
				/* and a power of two */
static uch *inbuf;		/* input buffer */
static uch window[WSIZE];	/* Sliding window buffer */

/* gzip flag byte */
#define ASCII_FLAG   0x01	/* bit 0 set: file probably ASCII text */
#define CONTINUATION 0x02	/* bit 1 set: continuation of multi-part gzip file */
#define EXTRA_FIELD  0x04	/* bit 2 set: extra field present */
#define ORIG_NAME    0x08	/* bit 3 set: original file name present */
#define COMMENT      0x10	/* bit 4 set: file comment present */
#define ENCRYPTED    0x20	/* bit 5 set: file is encrypted */
#define RESERVED     0xC0	/* bit 6,7:   reserved */


static unsigned insize;	/* valid bytes in inbuf */
static unsigned inptr;	/* index of next byte to be processed in inbuf */
static unsigned outcnt;	/* bytes in output buffer */

void variable_init(void);
#if ZDEBUG > 0
static void puts(const char *);
extern void putc_init(void);
extern void putc(unsigned char c);
#endif
static int fill_inbuf(void);
static void flush_window(void);
static void error(char *m);
static void gzip_mark(void **);
static void gzip_release(void **);

extern char input_data[];

extern char input_data_end[];

#if ZDEBUG > 0
void int2hex(unsigned long val)
{
        unsigned char buf[10];
        int i;
        for (i = 7;  i >= 0;  i--) {
                buf[i] = "0123456789ABCDEF"[val & 0x0F];
                val >>= 4;
        }
        buf[8] = '\0';
        puts(buf);
}
#endif

static unsigned long byte_count;

int get_byte(void)
{
#if ZDEBUG > 1
	static int printCnt;
#endif
	unsigned char c = (inptr < insize ? inbuf[inptr++] : fill_inbuf());
	byte_count++;
	
#if ZDEBUG > 1
	if (printCnt++ < 32) {
		puts("byte count = ");
		int2hex(byte_count);
		puts(" byte val = ");
		int2hex(c);
		puts("\n");
	}
#endif
	return c;
}

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

/*
 * This is set up by the setup-routine at boot-time
 */

static long bytes_out;
static uch *output_data;
static unsigned long output_ptr;


static void *malloc(int size);
static void free(void *where);
static void error(char *m);
static void gzip_mark(void **);
static void gzip_release(void **);

static unsigned long free_mem_ptr;
static unsigned long free_mem_end_ptr;

#include "../../../../../../lib/inflate.c"

static void *malloc(int size)
{
	void *p;
	
	if (size < 0)
		error("Malloc error\n");
	if (free_mem_ptr <= 0) error("Memory error\n");
	
	free_mem_ptr = (free_mem_ptr + 3) & ~3;	/* Align */
	
	p = (void *) free_mem_ptr;
	free_mem_ptr += size;
	
	if (free_mem_ptr >= free_mem_end_ptr)
		error("\nOut of memory\n");
	
	return p;
}

static void free(void *where)
{				/* Don't care */
}

static void gzip_mark(void **ptr)
{
	*ptr = (void *) free_mem_ptr;
}

static void gzip_release(void **ptr)
{
	free_mem_ptr = (long) *ptr;
}
#if ZDEBUG > 0
static void puts(const char *s)
{
	while (*s) {
		if (*s == 10)
			putc(13);
		putc(*s++);
	}
}
#endif

void *memset(void *s, int c, size_t n)
{
	int i;
	char *ss = (char *) s;
	
	for (i = 0; i < n; i++)
		ss[i] = c;
	return s;
}

void *memcpy(void *__dest, __const void *__src, size_t __n)
{
	int i;
	char *d = (char *) __dest, *s = (char *) __src;
	
	for (i = 0; i < __n; i++)
		d[i] = s[i];
	return __dest;
}

/* ===========================================================================
 * Fill the input buffer. This is called only when the buffer is empty
 * and at least one byte is really needed.
 */
static int fill_inbuf(void)
{
	if (insize != 0) {
		error("ran out of input data\n");
	}
	
	inbuf = input_data;
	insize = &input_data_end[0] - &input_data[0];
	inptr = 1;
	return inbuf[0];
}

/* ===========================================================================
 * Write the output window window[0..outcnt-1] and update crc and bytes_out.
 * (Used for the decompressed data only.)
 */
static void flush_window(void)
{
	ulg c = crc;		/* temporary variable */
	unsigned n;
	uch *in, *out, ch;
	
	in = window;
	out = &output_data[output_ptr];
	for (n = 0; n < outcnt; n++) {
		ch = *out++ = *in++;
		c = crc_32_tab[((int) c ^ ch) & 0xff] ^ (c >> 8);
	}
	crc = c;
	bytes_out += (ulg) outcnt;
	output_ptr += (ulg) outcnt;
	outcnt = 0;
}

#if ZDEBUG > 0
void check_mem(void)
{
	int i;
	
	puts("\ncplens = ");
	for (i = 0; i < 10; i++) {
		int2hex(cplens[i]);
		puts(" ");
	}
	puts("\ncplext = ");
	for (i = 0; i < 10; i++) {
		int2hex(cplext[i]);
		puts(" ");
	}
	puts("\nborder = ");
	for (i = 0; i < 10; i++) {
		int2hex(border[i]);
		puts(" ");
	}
	puts("\n");
}
#endif

static void error(char *x)
{
#if ZDEBUG > 1
	check_mem();
	puts("\n\n");
	puts(x);
	puts("byte_count = ");
	int2hex(byte_count);
	puts("\n");
	puts("\n\n -- Error. System halted");
#endif
	while (1);		/* Halt */
}

void variable_init(void)
{
	byte_count = 0;
	output_data = (char *) LOADADDR;
	free_mem_ptr = FREE_RAM;
	free_mem_end_ptr = END_RAM;
#if ZDEBUG > 1
	puts("output_data      0x");
	int2hex((unsigned long)output_data); puts("\n");
	puts("free_mem_ptr     0x");
	int2hex(free_mem_ptr); puts("\n");
	puts("free_mem_end_ptr 0x");
	int2hex(free_mem_end_ptr); puts("\n");
	puts("input_data       0x");
	int2hex((unsigned long)input_data); puts("\n");
#endif
}

int decompress_kernel(void)
{
#if ZDEBUG > 0
	putc_init();
#if ZDEBUG > 2
	check_mem();
#endif
#endif
	
	variable_init();
	
	makecrc();
#if ZDEBUG > 0
	puts("\n");
	puts("Uncompressing Linux... \n");
#endif
	gunzip();		// ...see inflate.c
#if ZDEBUG > 0
	puts("Ok, booting the kernel.\n");
#endif
	
#if ZDEBUG > 1
	{
		unsigned long *p = (unsigned long *)LOADADDR;
		int2hex(p[0]); puts("\n");
		int2hex(p[1]); puts("\n");
		int2hex(p[2]); puts("\n");
		int2hex(p[3]); puts("\n");
	}
#endif
	
	return 0;
}
