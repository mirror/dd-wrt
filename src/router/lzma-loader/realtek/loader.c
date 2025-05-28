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

static unsigned inptr = 0; /* index of next byte to be processed in inbuf */

#ifdef CONFIG_KERNEL_CMDLINE
#define kernel_argc 2
static const char kernel_cmdline[] = CONFIG_KERNEL_CMDLINE;
static const char *kernel_argv[] = {
	NULL,
	kernel_cmdline,
	NULL,
};
#endif /* CONFIG_KERNEL_CMDLINE */

static void reset(void);

static void halt(void)
{
	print("\nSystem reset!\n");
	reset();
}

static __inline__ unsigned long get_be32(void *buf)
{
	unsigned char *p = buf;

	return (((unsigned long)p[0] << 24) + ((unsigned long)p[1] << 16) + ((unsigned long)p[2] << 8) + (unsigned long)p[3]);
}

static __inline__ unsigned char get_byte(void)
{
	static unsigned int vall = 0;

	if (((unsigned int)inptr % 4) == 0) {
		vall = *(unsigned int *)lzma_data;
		lzma_data += 4;
	}
	return *(((unsigned char *)&vall) + (inptr++ & 3));
}

/* reading 4 bytes at once is way more efficient and speeds up decompression */
static unsigned int icnt = 1;
static inline int read_byte(void *object, const unsigned char **buffer, UInt32 *bufferSize)
{
	static unsigned char val;
	*bufferSize = 1;
	val = get_byte();
	*buffer = &val;
	if (icnt++ % (1024 * 10) == 0) {
		printf("[%d%%]\r", (100 * icnt) / lzma_datasize);
	}
	return LZMA_RESULT_OK;
}

static int lzma_decompress(unsigned char *outStream)
{
	unsigned int i;
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
	vs.Probs = (CProb *)workspace;
	printf("Decompressing %d bytes to address 0x%08x\n", lzma_outsize, kernel_la);
	// skip high order bytes
	for (i = 0; i < 4; i++)
		get_byte();
	// decompress kernel
	callback.Read = &read_byte;
	i = 0;
	ret = LzmaDecode(&vs, &callback, outStream, lzma_outsize, &i);
	if (ret != LZMA_RESULT_OK) {
		printf("LzmaDecode error %d at %08x, osize:%d ip:%d op:%d\n", ret, lzma_data + inptr, lzma_outsize, inptr, i);
		return 0;
	}
	return ret;
}
static inline unsigned int readl(const volatile void *addr)
{
	return *(const volatile unsigned int *)addr;
}
static inline void writel(unsigned int value, volatile void *addr)
{
	*(volatile unsigned int *)addr = value;
}

#define RTC_DENOMINATOR 100
#define RTC_PERIOD 110000000 / RTC_DENOMINATOR

#define MACRO_START do {
#define MACRO_END \
	}         \
	while (0)
static unsigned int clock_period;

#define clock_initialize(_period_)     \
	MACRO_START                    \
	asm volatile("mtc0 $0,$9\n"    \
		     "nop; nop; nop\n" \
		     "mtc0 %0,$11\n"   \
		     "nop; nop; nop\n" \
		     :                 \
		     : "r"(_period_)); \
	clock_period = _period_;       \
	MACRO_END

#define clock_reset(_vector_, _period_) \
	MACRO_START                     \
	asm volatile("mtc0 $0,$9\n"     \
		     "nop; nop; nop\n"  \
		     "mtc0 %0,$11\n"    \
		     "nop; nop; nop\n"  \
		     :                  \
		     : "r"(_period_));  \
	MACRO_END

#define clock_read(_pvalue_)                           \
	MACRO_START                                    \
	register unsigned int result;                  \
	asm volatile("mfc0   %0,$9\n" : "=r"(result)); \
	*(_pvalue_) = result;                          \
	MACRO_END

/*
 * udelay implementation based on cpu cycle counter
 */
static void udelay(int us)
{
	unsigned int val1, val2;
	int diff;
	long usticks;
	long ticks;

	// Calculate the number of counter register ticks per microsecond.

	usticks = (RTC_PERIOD * RTC_DENOMINATOR) / 1000000;

	// Make sure that the value is not zero. This will only happen if the
	// CPU is running at < 2MHz.
	if (usticks == 0)
		usticks = 1;

	while (us > 0) {
		int us1 = us;

		// Wait in bursts of less than 10000us to avoid any overflow
		// problems in the multiply.
		if (us1 > 10000)
			us1 = 10000;

		us -= us1;

		ticks = us1 * usticks;

		clock_read(&val1);
		while (ticks > 0) {
			do {
				clock_read(&val2);
			} while (val1 == val2);
			diff = val2 - val1;
			if (diff < 0)
				diff += RTC_PERIOD;
			ticks -= diff;
			val1 = val2;
		}
	}
}

#define msleep(val) udelay(val * 1000)
#define RTL838X_SW_BASE ((volatile void *)0xBB000000)
#define sw_r32(reg) readl(RTL838X_SW_BASE + reg)
#define sw_w32(val, reg) writel(val, RTL838X_SW_BASE + reg)

#define RTL838X_MODEL_NAME_INFO (0x00D4)
#define RTL839X_MODEL_NAME_INFO (0x0FF0)
#define RTL93XX_MODEL_NAME_INFO (0x0004)
#define RTL838X_PLL_CML_CTRL (0x0FF8)
#define RTL838X_INT_RW_CTRL (0x0058)
#define RTL838X_RST_GLB_CTRL_0 (0x003c)
#define RTL838X_RST_GLB_CTRL_1 (0x0040)
#define RTL839X_RST_GLB_CTRL (0x0014)
#define RTL930X_RST_GLB_CTRL_0 (0x000c)
#define RTL931X_RST_GLB_CTRL (0x0400)

unsigned int pll_reset_value;

static void rtl838x_restart(void)
{
	unsigned int pll = sw_r32(RTL838X_PLL_CML_CTRL);

	pll_reset_value = sw_r32(RTL838X_PLL_CML_CTRL);
	printf("System restart.\n");
	printf("PLL control register: %x, applying reset value %x\n", pll, pll_reset_value);

	sw_w32(3, RTL838X_INT_RW_CTRL);
	sw_w32(pll_reset_value, RTL838X_PLL_CML_CTRL);
	sw_w32(0, RTL838X_INT_RW_CTRL);

	/* Reset Global Control1 Register */
	sw_w32(1, RTL838X_RST_GLB_CTRL_1);
}

static void rtl839x_restart(void)
{
	/* SoC reset vector (in flash memory): on RTL839x platform preferred way to reset */
	void (*f)(void) = (void *)0xbfc00000;

	printf("System restart.\n");
	/* Reset SoC */
	sw_w32(0xFFFFFFFF, RTL839X_RST_GLB_CTRL);
	/* and call reset vector */
	f();
}

static void rtl930x_restart(void)
{
	printf("System restart.\n");
	sw_w32(0x1, RTL930X_RST_GLB_CTRL_0);
}

static void rtl931x_restart(void)
{
	unsigned int v;

	printf("System restart.\n");
	sw_w32(1, RTL931X_RST_GLB_CTRL);
	v = sw_r32(RTL931X_RST_GLB_CTRL);
	sw_w32(0x101, RTL931X_RST_GLB_CTRL);
	msleep(15);
	sw_w32(v, RTL931X_RST_GLB_CTRL);
	msleep(15);
	sw_w32(0x101, RTL931X_RST_GLB_CTRL);
}

#define RTL8389_FAMILY_ID (0x8389)
#define RTL8328_FAMILY_ID (0x8328)
#define RTL8390_FAMILY_ID (0x8390)
#define RTL8350_FAMILY_ID (0x8350)
#define RTL8380_FAMILY_ID (0x8380)
#define RTL8330_FAMILY_ID (0x8330)
#define RTL9300_FAMILY_ID (0x9300)
#define RTL9310_FAMILY_ID (0x9310)

static int model;
static int family;
static char *name;

static void reset(void)
{
	switch (family) {
	case RTL8380_FAMILY_ID:
		rtl838x_restart();
		break;
	case RTL8390_FAMILY_ID:
		rtl839x_restart();
		break;
	case RTL9300_FAMILY_ID:
		rtl930x_restart();
		break;
	case RTL9310_FAMILY_ID:
		rtl931x_restart();
		break;
	}
	while (1)
		;
}
static void identify_rtl9302(void)
{
	switch (sw_r32(RTL93XX_MODEL_NAME_INFO) & 0xfffffff0) {
	case 0x93020810:
		name = "RTL9302A 12x2.5G";
		break;
	case 0x93021010:
		name = "RTL9302B 8x2.5G";
		break;
	case 0x93021810:
		name = "RTL9302C 16x2.5G";
		break;
	case 0x93022010:
		name = "RTL9302D 24x2.5G";
		break;
	case 0x93014010:
		name = "RTL9301H 4x2.5G";
		break;
	case 0x93016810:
		name = "RTL9301H 15x1G";
		break;
	case 0x93020800:
		name = "RTL9302A";
		break;
	case 0x93021000:
		name = "RTL9302B";
		break;
	case 0x93021800:
		name = "RTL9302C";
		break;
	case 0x93022000:
		name = "RTL9302D";
		break;
	case 0x93023001:
		name = "RTL9302F";
		break;
	case 0x93036810:
		name = "RTL9303 8x10G";
		break;
	default:
		name = "RTL9302";
	}
}

static void detect(void)
{
	model = sw_r32(RTL838X_MODEL_NAME_INFO);
	//	if (model)
	//		printf("RTL838X model is %x\n", model);
	model = model >> 16 & 0xFFFF;

	if ((model != 0x8328) && (model != 0x8330) && (model != 0x8332) && (model != 0x8380) && (model != 0x8382) &&
	    (model != 0x8381)) {
		model = sw_r32(RTL839X_MODEL_NAME_INFO);
		//		if (model)
		//			printf("RTL839X model is %x\n", model);
		model = model >> 16 & 0xFFFF;
	}

	if ((model & 0x8380) != 0x8380 && (model & 0x8390) != 0x8390) {
		model = sw_r32(RTL93XX_MODEL_NAME_INFO);
		//		if (model)
		//			printf("RTL93XX model is %x\n", model);
		model = model >> 16 & 0xFFFF;
	}
	switch (model) {
	case 0x8328:
		name = "RTL8328";
		family = RTL8328_FAMILY_ID;
		break;
	case 0x8332:
		name = "RTL8332";
		family = RTL8380_FAMILY_ID;
		break;
	case 0x8380:
		name = "RTL8380";
		family = RTL8380_FAMILY_ID;
		break;
	case 0x8381:
		name = "RTL8381";
		family = RTL8380_FAMILY_ID;
		break;
	case 0x8382:
		name = "RTL8382";
		family = RTL8380_FAMILY_ID;
		break;
	case 0x8390:
		name = "RTL8390";
		family = RTL8390_FAMILY_ID;
		break;
	case 0x8391:
		name = "RTL8391";
		family = RTL8390_FAMILY_ID;
		break;
	case 0x8392:
		name = "RTL8392";
		family = RTL8390_FAMILY_ID;
		break;
	case 0x8393:
		name = "RTL8393";
		family = RTL8390_FAMILY_ID;
		break;
	case 0x8396:
		name = "RTL8396";
		family = RTL8390_FAMILY_ID;
		break;
	case 0x9301:
		family = RTL9300_FAMILY_ID;
		name = "RTL9301";
		identify_rtl9302();
		break;
	case 0x9302:
		name = "RTL9302";
		identify_rtl9302();
		family = RTL9300_FAMILY_ID;
		break;
	case 0x9303:
		name = "RTL9303";
		identify_rtl9302();
		family = RTL9300_FAMILY_ID;
		break;
	case 0x9310:
		name = "RTL9310";
		family = RTL9310_FAMILY_ID;
		break;
	case 0x9311:
		name = "RTL9311";
		family = RTL9310_FAMILY_ID;
		break;
	case 0x9312:
		name = "RTL9312";
		family = RTL9310_FAMILY_ID;
		break;
	case 0x9313:
		name = "RTL9313";
		family = RTL9310_FAMILY_ID;
		break;
	default:
		name = "DEFAULT";
		family = 0;
	}
	printf("Board Model is %s\n", name);
	printf("clock period is %d\n", clock_period);
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
	clock_initialize(RTC_PERIOD);

	print("\n\nDD-WRT Kernel lzma Loader\n");
	print("Copyright (C) 2011 Gabor Juhos <juhosg@openwrt.org>\n");
	print("Copyright (C) 2025 Sebastian Gottschall <s.gottschall@dd-wrt.com>\n");
	detect();
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
