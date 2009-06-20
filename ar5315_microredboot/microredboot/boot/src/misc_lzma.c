/*
 * misc_lzma.c
 * originally written for xscale based linux kernel decompression
 * now adapted for AR531X based redboot stub and kernel loader
 * copyright 2009 Sebastian Gottschall / NewMedia-NET GmbH / DD-WRT.COM
 * licensed under GPL conditions
 * this stub will load and decompress redboot if the reset button is pushed, otherwise it parses the redboot directory for a partition named linux*, vmlinux* or kernel*
 * if such a partition has been found, it will be decompressed and executed, if not. redboot is started. if a decompression error occures while loading the linux partition, 
 * the redboot is started too.
 * take care about the ramconfig.h header since it must contain the correct ram size and gpio button value for the reset button
 * this code is partially based on redboot and linux sources
 */

#ifdef STANDALONE_DEBUG
#define putstr printf
#else

#include <linux/kernel.h>

#include <asm/uaccess.h>
#include "ramconfig.h"
#include "uncompress.h"
#include "spiflash.h"
#include "printf.h"

#endif

#define __ptr_t void *

typedef unsigned char uch;
typedef unsigned short ush;
typedef unsigned long ulg;

static uch *inbuf;		/* input buffer */
static unsigned int nvramdetect = 0;

static unsigned insize = 0;	/* valid bytes in inbuf */
static unsigned inptr = 0;	/* index of next byte to be processed in inbuf */
static unsigned outcnt;		/* bytes in output buffer */

static void fill_inbuf(void);

static inline unsigned char get_byte(void)
{
	static unsigned int vall;

	if (((unsigned int)inptr % 4) == 0) {
		vall = *(unsigned int *)inbuf;
		inbuf += 4;
	}
	return *(((unsigned char *)&vall) + (inptr++ & 3));
}

//#define get_byte()  (inptr < insize ? inbuf[inptr++] : fill_inbuf())

static void flush_window(void);
static void error(char *m);

extern unsigned char input_data[];
extern unsigned char input_data_end[];

static ulg output_ptr = 0;
static uch *output_data;
static ulg bytes_out;

extern int end;
static ulg free_mem_ptr;
static ulg free_mem_ptr_end;

#define _LZMA_IN_CB

#include "lib/LzmaDecode.h"
static unsigned int icnt = 0;
static inline int read_byte(unsigned char **buffer, UInt32 * bufferSize)
{
	static unsigned char val;
	*bufferSize = 1;
	val = get_byte();
	*buffer = &val;
	if (icnt++ % (1024 * 10) == 0)
		putc('.');
	return LZMA_RESULT_OK;
}

#include "lib/LzmaDecode.c"

int bootoffset = 0x800004bc;

/*
 * Do the lzma decompression
 */

static int disaster = 0;
static int lzma_unzip(void)
{

	unsigned int i;
	unsigned int uncompressedSize = 0;
	unsigned char *workspace;
	unsigned int lc, lp, pb;
	if (inptr >= insize)
		fill_inbuf();

	// lzma args
	i = get_byte();
	lc = i % 9, i = i / 9;
	lp = i % 5, pb = i / 5;

	// skip dictionary size
	for (i = 0; i < 4; i++)
		get_byte();
	// get uncompressed size
	int a, b, c, d;
	a = get_byte();
	b = get_byte();
	c = get_byte();
	d = get_byte();
	uncompressedSize = (a) + (b << 8) + (c << 16) + (d << 24);
	if (uncompressedSize > 0x400000 || lc > 3 || pb > 3 || lp > 3) {
		if (disaster) {
			error
			    ("\ndata corrupted in recovery RedBoot too, this is a disaster condition. please re-jtag\n");
		}
		disaster = 1;
		puts("\ndata corrupted!\nswitching to recovery RedBoot\nloading");
		inbuf = input_data;
		insize = &input_data_end[0] - &input_data[0];
		inptr = 0;
		output_data = (uch *) 0x80000400;
		bootoffset = 0x800004bc;
		return lzma_unzip();

	}
	workspace = output_data + uncompressedSize;
	// skip high order bytes
	for (i = 0; i < 4; i++)
		get_byte();
	// decompress kernel
	if (LzmaDecode
	    (workspace, ~0, lc, lp, pb, (unsigned char *)output_data,
	     uncompressedSize, &i) == LZMA_RESULT_OK) {
		if (i != uncompressedSize) {
			if (disaster) {
				error
				    ("data corrupted in recovery RedBoot too, this is a disaster condition. please re-jtag\n");
			}
			disaster = 1;
			puts("\ndata corrupted!\nswitching to recovery RedBoot\nloading");
			inbuf = input_data;
			insize = &input_data_end[0] - &input_data[0];
			inptr = 0;
			output_data = (uch *) 0x80000400;
			bootoffset = 0x800004bc;
			return lzma_unzip();
		}
		//copy it back to low_buffer
		bytes_out = i;
		output_ptr = i;
		return 0;
	}
	return 1;
}

/*if (((unsigned int)offset % 4) == 0) {
	vall = *(unsigned int *)buffer;
	buffer += 4;
}
    if (buffer>=BufferLim)
	{
	ExtraBytes=1;
	return 0xff;
	}
  return *(((unsigned char *)&vall) + (offset++ & 3));
*/

struct fis_image_desc {
	unsigned char name[16];	// Null terminated name
	unsigned long flash_base;	// Address within FLASH of image
	unsigned long mem_base;	// Address in memory where it executes
	unsigned long size;	// Length of image
	unsigned long entry_point;	// Execution entry point
	unsigned long data_length;	// Length of actual data
	unsigned char _pad[256 - (16 + 7 * sizeof(unsigned long))];
	unsigned long desc_cksum;	// Checksum over image descriptor
	unsigned long file_cksum;	// Checksum over image data
};

#ifdef AR5312
#include "arch/ar5312.c"
#else
#include "arch/ar2315.c"
#endif
/*
 * searches for a directory entry named linux* vmlinux* or kernel and returns its flash address (it also initializes entrypoint and load address)
 */
static unsigned int getLinux(void)
{
	int count = 0;
	unsigned char *p =
	    (unsigned char *)(flashbase + flashsize - (sectorsize * 2));
	struct fis_image_desc *fis = (struct fis_image_desc *)p;
	while (fis->name[0] != 0xff && count < 10) {
		if (!strncmp(fis->name, "linux", 5)
		    || !strncmp(fis->name, "vmlinux", 7)
		    || !strcmp(fis->name, "kernel")) {
			printf
			    ("found bootable image: [%s] at [0x%08X] EP [0x%08X]\n",
			     fis->name, fis->flash_base, fis->entry_point);
			bootoffset = fis->entry_point;
			output_data = (uch *) fis->mem_base;
			return fis->flash_base;
		}
		p += 256;
		fis = (struct fis_image_desc *)p;
		count++;
	}
	puts("no bootable image found, try default location 0xbfc10000\n");
	bootoffset = 0x80041000;
	output_data = (uch *) 0x80041000;
	return 0xbfc10000;
}

/* ===========================================================================
 * Fill the input buffer. This is called only when the buffer is empty
 * and at least one byte is really needed.
 */
static int resettrigger = 0;

static void fill_inbuf(void)
{
	if (insize != 0)
		error("ran out of input data");
	if (resettrigger) {
		inbuf = (uch *) linuxaddr;
		insize = 0x400000;
		inptr = 0;
	} else {
		inbuf = input_data;
		insize = &input_data_end[0] - &input_data[0];
		inptr = 0;
	}
	return;
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

	printf("\n\n%s\n\n -- System halted", x);

	while (1) ;		/* Halt */
}

/*
 * checks if the reset button is pressed, return 1 if the button is pressed and 0 if not
 */
static int resetTouched(void)
{
	int trigger = getGPIO(RESETBUTTON & 0x0f);
	if (RESETBUTTON & 0xf0)
		trigger = 1 - trigger;
	return trigger;
}

struct nvram_header {
	__u32 magic;
	__u32 len;
	__u32 crc_ver_init;	/* 0:7 crc, 8:15 ver, 16:27 init, mem. test 28, 29-31 reserved */
	__u32 config_refresh;	/* 0:15 config, 16:31 refresh */
	__u32 config_ncdl;	/* ncdl values for memc */
};

struct nvram_tuple {
	char *name;
	char *value;
	struct nvram_tuple *next;
};

#define NVRAM_SPACE 0x10000
#define NVRAM_MAGIC			0x48534C46	/* 'NVFL' */

static char nvram_buf[65536] __attribute__((aligned(4096))) = {
0};				//

/*
 * simple dd-wrt nvram implementation (read only)
 */
static void nvram_init(void)
{
	struct nvram_header *header;
	__u32 off, lim;
	int i;
	flashdetect();
	for (i = 0; i < 4; i++) {
		header =
		    (struct nvram_header *)(flashbase + flashsize -
					    (sectorsize * (3 + i)));
		if (header->magic == NVRAM_MAGIC && header->len > 0
		    && header->len <= NVRAM_SPACE) {
			printf
			    ("DD-WRT NVRAM with size = %d found on [0x%08X]\n",
			     header->len, header);
			nvramdetect = (unsigned int)header;
			unsigned int *src = header;
			unsigned int *dst = nvram_buf;
			for (i = 0; i < NVRAM_SPACE / 4; i++)
				dst[i] = src[i];
			return;
		}
	}
}

static char *nvram_get(const char *name)
{
	char *var, *value, *end, *eq;

	if (!name)
		return NULL;

	if (!nvram_buf[0])
		nvram_init();

	/* Look for name=value and return value */
	var = &nvram_buf[sizeof(struct nvram_header)];
	end = nvram_buf + sizeof(nvram_buf) - 2;
	end[0] = end[1] = '\0';
	for (; *var; var = value + strlen(value) + 1) {
		if (!(eq = strchr(var, '=')))
			break;
		value = eq + 1;
		if ((eq - var) == strlen(name)
		    && strncmp(var, name, (eq - var)) == 0)
			return value;
	}

	return NULL;
}

ulg
decompress_kernel(ulg output_start, ulg free_mem_ptr_p, ulg free_mem_ptr_end_p)
{
	output_data = (uch *) output_start;
	free_mem_ptr = free_mem_ptr_p;
	free_mem_ptr_end = free_mem_ptr_end_p;
	disable_watchdog();
	arch_decomp_setup();
	printf("MicroRedBoot v1.2, (c) 2009 DD-WRT.COM (%s)\n", __DATE__);
	nvram_init();
	char *ddboard = nvram_get("DD_BOARD");
	if (ddboard)
		printf("Board: %s\n", ddboard);
	char *resetbutton = nvram_get("resetbutton_enable");
	if (resetbutton && !strcmp(resetbutton, "1"))
		puts("reset button manual override detected! (nvram var resetbutton_enable=1)\n");
	if (resetTouched() || (resetbutton && !strcmp(resetbutton, "1"))) {
		puts("Reset Button triggered\nBooting Recovery RedBoot\n");

		int count = 5;
		while (count--) {
			if (!resetTouched())	// check if reset button is unpressed again
				break;
			udelay(1000000);
		}
		if (count <= 0) {
			puts("reset button 5 seconds pushed, erasing nvram\n");

			if (!flashdetect())
				flash_erase_nvram(flashsize, sectorsize);
		}

		bootoffset = 0x800004bc;
		resettrigger = 0;
	} else {
		flashdetect();
		linuxaddr = getLinux();
		puts("Booting Linux\n");
		resettrigger = 1;
		/* initialize clock */
		HAL_CLOCK_INITIALIZE(RTC_PERIOD);

		/* important, enable ethernet bus, if the following lines are not initialized linux will not be able to use the ethernet mac, taken from redboot source */
		enable_ethernet();
	}
	puts("loading");
	lzma_unzip();
	puts("\n\n\n");

	return output_ptr;
}
