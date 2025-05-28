/*
 * LZMA compressed kernel loader for Atheros AR7XXX/AR9XXX based boards
 *
 * Copyright (C) 2011 Gabor Juhos <juhosg@openwrt.org>
 *
 * Some parts of this code was based on the OpenWrt specific lzma-loader
 * for the BCM47xx and ADM5120 based boards:
 *	Copyright (C) 2004 Manuel Novoa III (mjn3@codepoet.org)
 *	Copyright (C) 2005 Mineharu Takahara <mtakahar@yahoo.com>
 *	Copyright (C) 2005 by Oleg I. Vdovikin <oleg@cs.msu.su>
 *
 * The image_header structure has been taken from the U-Boot project.
 *	(C) Copyright 2008 Semihalf
 *	(C) Copyright 2000-2005
 *	Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#include "config.h"
#include "cache.h"
#include "printf.h"
#include "LzmaDecode.h"

extern void board_putc(int ch);

static void print(char *s)
{
	int i = 0;
	while (s[i]) {
		board_putc(s[i]);
		if (s[i] == '\n')
			board_putc('\r');
		i++;
	}
}

#define KSEG0 0x80000000
#define KSEG1 0xa0000000

#define KSEG1ADDR(a) ((((unsigned)(a)) & 0x1fffffffU) | KSEG1)

//#define LZMA_DEBUG

#ifdef LZMA_DEBUG
#define DBG(f, a...) print(f)
#else
#define DBG(f, a...) \
	do {         \
	} while (0)
#endif

#define IH_MAGIC_OKLI 0x4f4b4c49 /* 'OKLI' */

#define IH_NMLEN 32 /* Image Name Length		*/

typedef struct image_header {
	uint32_t ih_magic; /* Image Header Magic Number	*/
	uint32_t ih_hcrc; /* Image Header CRC Checksum	*/
	uint32_t ih_time; /* Image Creation Timestamp	*/
	uint32_t ih_size; /* Image Data Size		*/
	uint32_t ih_load; /* Data	 Load  Address		*/
	uint32_t ih_ep; /* Entry Point Address		*/
	uint32_t ih_dcrc; /* Image Data CRC Checksum	*/
	uint8_t ih_os; /* Operating System		*/
	uint8_t ih_arch; /* CPU architecture		*/
	uint8_t ih_type; /* Image Type			*/
	uint8_t ih_comp; /* Compression Type		*/
	uint8_t ih_name[IH_NMLEN]; /* Image Name		*/
} image_header_t;

/* beyond the image end, size not known in advance */
extern unsigned char workspace[];
extern void board_init(void);

static CLzmaDecoderState lzma_state;
static unsigned char *lzma_data;
static unsigned long lzma_datasize;
static unsigned long lzma_outsize;
static unsigned long kernel_la;

static unsigned inptr = 0;	/* index of next byte to be processed in inbuf */

#ifdef CONFIG_KERNEL_CMDLINE
#define kernel_argc 2
static const char kernel_cmdline[] = CONFIG_KERNEL_CMDLINE;
static const char *kernel_argv[] = {
	NULL,
	kernel_cmdline,
	NULL,
};
#endif /* CONFIG_KERNEL_CMDLINE */

static void halt(void)
{
	print("\nSystem halted!\n");
	for (;;)
		;
}

static __inline__ unsigned long get_be32(void *buf)
{
	unsigned char *p = buf;

	return (((unsigned long)p[0] << 24) + ((unsigned long)p[1] << 16) + ((unsigned long)p[2] << 8) + (unsigned long)p[3]);
}

static inline unsigned char get_byte(void)
{
	static unsigned int vall;

	if (((unsigned int)inptr % 4) == 0) {
		vall = *(unsigned int *)lzma_data;
		lzma_data += 4;
	}
	return *(((unsigned char *)&vall) + (inptr++ & 3));
}

/* reading 4 bytes at once is way more efficient and speeds up decompression */
static unsigned int icnt = 1;
static inline int read_byte(void *object, const unsigned char **buffer, UInt32 * bufferSize)
{
	static unsigned char val;
	*bufferSize = 1;
	val = get_byte();
	*buffer = &val;
	if (icnt++ % (1024 * 10) == 0)
		board_putc('.');
	return LZMA_RESULT_OK;
}


static int lzma_decompress(unsigned char *outStream)
{

	unsigned int i;
	unsigned char *workspace;
	unsigned int lc, lp, pb;

	CLzmaDecoderState vs;
	ILzmaInCallback callback;
	int ret;
	
	// lzma args
	i = get_byte();
	lc = i % 9, i = i / 9;
	lp = i % 5, pb = i / 5;

	vs.Properties.lc = lc;
	vs.Properties.lp = lp;
	vs.Properties.pb = pb;

	// skip dictionary size
	for (i = 0; i < 4; i++)
		get_byte();
	// get uncompressed size
	int a, b, c, d;
	a = get_byte();
	b = get_byte();
	c = get_byte();
	d = get_byte();
	lzma_outsize = (a) + (b << 8) + (c << 16) + (d << 24);
	workspace = outStream + lzma_outsize;
	vs.Probs = (CProb *) workspace;
	printf("Decompressing %d bytes to address 0x%08x\n", lzma_outsize, kernel_la);
	// skip high order bytes
	for (i = 0; i < 4; i++)
		get_byte();
	// decompress kernel
	callback.Read = &read_byte;
	i = 0;
	ret = LzmaDecode(&vs, &callback, outStream, lzma_outsize, &i);
	if ( ret != LZMA_RESULT_OK) {
		printf("LzmaDecode error %d at %08x, osize:%d ip:%d op:%d\n", ret, lzma_data + inptr, lzma_outsize, inptr, i);
		return 0;
	}
	return ret;
}

static void lzma_init_data(void)
{
	extern unsigned char _lzma_data_start[];
	extern unsigned char _lzma_data_end[];

	kernel_la = LOADADDR;
	lzma_data = _lzma_data_start;
	lzma_datasize = _lzma_data_end - _lzma_data_start;
	inptr = 0;
}

void loader_main(unsigned long reg_a0, unsigned long reg_a1, unsigned long reg_a2, unsigned long reg_a3)
{
	void (*kernel_entry)(unsigned long, unsigned long, unsigned long, unsigned long);
	int res;

	board_init();

	print("\n\nDD-WRT Kernel lzma Loader\n");
	print("Copyright (C) 2011 Gabor Juhos <juhosg@openwrt.org>\n");
	print("Copyright (C) 2025 Sebastian Gottschall <s.gottschall@dd-wrt.com>\n");

	lzma_init_data();

	res = lzma_decompress((unsigned char *)kernel_la);
	if (res != LZMA_RESULT_OK) {
		print("failed, ");
		switch (res) {
		case LZMA_RESULT_DATA_ERROR:
			print("data error!\n");
			break;
		default:
			print("unknown error!\n");
		}
		halt();
	} else {
		print("\ndone!\n");
	}

	flush_cache(kernel_la, lzma_outsize);

	print("Starting kernel...\n\n");

#ifdef CONFIG_KERNEL_CMDLINE
	reg_a0 = kernel_argc;
	reg_a1 = (unsigned long)kernel_argv;
	reg_a2 = 0;
	reg_a3 = 0;
#endif

	kernel_entry = (void *)kernel_la;
	kernel_entry(reg_a0, reg_a1, reg_a2, reg_a3);
}
