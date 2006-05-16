/*
 * Initialization and support routines for self-booting
 * compressed image.
 *
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id$
 */

#include <typedefs.h>
#include <osl.h>
#include <sbutils.h>
#include <bcmdevs.h>
#include <bcmutils.h>
#include <sbconfig.h>
#include <sbextif.h>
#include <sbchipc.h>
#include <hndmips.h>
#include <sbmemc.h>
#include <sflash.h>
#include <bcmsrom.h>

void c_main(unsigned long ra);

static sb_t *sbh;
static chipcregs_t *cc;

static struct sflash *sflash;

extern char text_start[], text_end[];
extern char data_start[], data_end[];
extern char bss_start[], bss_end[];

#define INBUFSIZ 4096
#define WSIZE 0x8000    	/* window size--must be a power of two, and */
				/*  at least 32K for zip's deflate method */

static uchar *inbuf;		/* input buffer */
static ulong insize;		/* valid bytes in inbuf */
static ulong inptr;		/* index of next byte to be processed in inbuf */

static uchar *outbuf;		/* output buffer */
static ulong bytes_out;		/* valid bytes in outbuf */

static ulong inoff;		/* offset of input data */

static int
fill_inbuf(void)
{
	int bytes;

	for (insize = 0; insize < INBUFSIZ; insize += bytes, inoff += bytes) {
		if (sflash) {
			if ((bytes = sflash_read(cc, inoff, INBUFSIZ - insize, &inbuf[insize])) < 0)
				return bytes;
		} else {
			*((uint32 *) &inbuf[insize]) = *((uint32 *) KSEG1ADDR(SB_FLASH1 + inoff));
			bytes = sizeof(uint32);
		}
	}

	inptr = 1;

	return inbuf[0];
}

/* Defines for gzip/bzip */
#define	malloc(size)	MALLOC(NULL, size)
#define	free(addr)	MFREE(NULL, addr, 0)

static void
error(char *x)
{
	printf("\n\n%s\n\n -- System halted", x);

	for (;;);
}

#if defined(USE_GZIP)

/*
 * gzip declarations
 */

#define OF(args) args
#define STATIC static

#define memzero(s, n)	memset ((s), 0, (n))

typedef unsigned char  uch;
typedef unsigned short ush;
typedef unsigned long  ulg;

#define get_byte()  (inptr < insize ? inbuf[inptr++] : fill_inbuf())

/* Diagnostic functions (stubbed out) */

#define Assert(cond,msg)
#define Trace(x)
#define Tracev(x)
#define Tracevv(x)
#define Tracec(c,x)
#define Tracecv(c,x)

static uchar *window;		/* Sliding window buffer */
static unsigned outcnt;		/* bytes in window buffer */

static void
gzip_mark(void **ptr)
{
	/* I'm not sure what the pourpose of this is, there are no malloc
	 * calls without free's in the gunzip code.
	 */
}

static void
gzip_release(void **ptr)
{
}

static void flush_window(void);

#include "gzip_inflate.c"

/* ===========================================================================
 * Write the output window window[0..outcnt-1] and update crc and bytes_out.
 * (Used for the decompressed data only.)
 */
static void
flush_window(void)
{
	ulg c = crc;
	unsigned n;
	uch *in, *out, ch;

	in = window;
	out = &outbuf[bytes_out];
	for (n = 0; n < outcnt; n++) {
		ch = *out++ = *in++;
		c = crc_32_tab[((int)c ^ ch) & 0xff] ^ (c >> 8);
	}
	crc = c;
	bytes_out += (ulg)outcnt;
	outcnt = 0;
	putc('.');
}

#elif defined(USE_BZIP2)

/*
 * bzip2 declarations
 */

void bz_internal_error (int i)
{
	char msg[128];

	sprintf(msg, "Bzip2 internal error: %d", i);
	error(msg);
}

#include "bzip2_inflate.c"

static int
bunzip2(void)
{
	bz_stream bzstream;
	int ret = 0;

        bzstream.bzalloc = 0;
        bzstream.bzfree = 0;
	bzstream.opaque = 0;
	bzstream.avail_in = 0;

        if ((ret = BZ2_bzDecompressInit(&bzstream, 0, 1)) != BZ_OK)
		return ret;

	for (;;) {
		if (bzstream.avail_in == 0) {
			fill_inbuf();
			bzstream.next_in = inbuf;
			bzstream.avail_in = insize;
		}
		bzstream.next_out = &outbuf[bytes_out];
		bzstream.avail_out = WSIZE;
	 	if ((ret = BZ2_bzDecompress(&bzstream)) != BZ_OK)
			break;
		bytes_out = bzstream.total_out_lo32;
		putc('.');
	}

	if (ret == BZ_STREAM_END)
		ret = BZ2_bzDecompressEnd(&bzstream);

	if (ret == BZ_OK)
		ret = 0;

	return ret;
}

#endif

extern char input_data[];
extern int input_len;

static void
load(void)
{
	int ret = 0;

	/* Offset from beginning of flash */
#ifdef	CONFIG_XIP
	inoff = ((ulong)text_end - (ulong)text_start) + ((ulong)input_data - (ulong)data_start);
#else
	inoff = (ulong) input_data - (ulong) text_start;
#endif
	outbuf = (uchar *) LOADADDR;
	bytes_out = 0;
	inbuf = malloc(INBUFSIZ);	/* input buffer */

#if defined(USE_GZIP)
	window = malloc(WSIZE);
	printf("Decompressing...");
	makecrc();
	ret = gunzip();
#elif defined(USE_BZIP2)
	/* Small decompression algorithm uses up to 2300k of memory */
	printf("Decompressing...");
	ret = bunzip2();
#else
	printf("Copying...");
	while (bytes_out < input_len) {
		fill_inbuf();
		memcpy(&outbuf[bytes_out], inbuf, insize);
		bytes_out += insize;
	}
#endif
	if (ret) {
		printf("error %d\n", ret);
	} else
		printf("done\n");
}

static void
sflash_self(chipcregs_t *cc)
{
	unsigned char *start = text_start;
	unsigned char *end = data_end;
	unsigned char *cur = start;
	unsigned int erasesize, len;

	while (cur < end) {
		/* Erase sector */
		printf("Erasing sector 0x%x...", (cur - start));
		if ((erasesize = sflash_erase(cc, cur - start)) < 0) {
			printf("error\n");
			break;
		}
		while (sflash_poll(cc, cur - start));
		printf("done\n");

		/* Write sector */
		printf("Writing sector 0x%x...", (cur - start));
		while (erasesize) {
			if ((len = sflash_write(cc, cur - start, erasesize, cur)) < 0)
				break;
			while (sflash_poll(cc, cur - start));
			cur += len;
			erasesize -= len;
		}
		if (erasesize) {
			printf("error\n");
			break;
		}
		printf("done\n");
	}
}

void
c_main(unsigned long ra)
{
	/* Basic initialization */
	sbh = (sb_t *)osl_init();

#ifndef CFG_UNCACHED
	/* Initialize and turn caches on */
	caches_on();
#endif

	cc = sb_setcore(sbh, SB_CC, 0);

	/* Initialize serial flash */
	sflash = cc ? sflash_init(cc) : NULL;

	/* Copy self to flash if we booted from SDRAM */
	if (PHYSADDR(ra) < SB_FLASH1) {
		if (sflash)
			sflash_self(cc);
	}

	/* Load binary */
	load();

	/* Flush all caches */
	blast_dcache();
	blast_icache();

	/* Jump to load address */
	((void (*)(void)) LOADADDR)();
}
