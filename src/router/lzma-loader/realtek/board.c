/*
 * Arch specific code for ramips based boards
 *
 * Copyright (C) 2013 John Crispin <blogic@openwrt.org>
 * Copyright (C) 2018 Tobias Schramm <tobleminer@gmail.com>
 * Copyright (C) 2023 Antonio Vázquez <antoniovazquezblanco@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <stdint.h>
#include <stddef.h>
#include "clock.h"
#include "board.h"
#include "printf.h"


struct rtl83xx_soc_info soc_info;
static char soc_name[16];
static char rtl83xx_system_type[32];

#define KSEG0 0x80000000
#define KSEG1 0xa0000000

#define _ATYPE_ __PTRDIFF_TYPE__
#define _ATYPE32_ int

#define _ACAST32_ (_ATYPE_)(_ATYPE32_)

#define CPHYSADDR(a) ((_ACAST32_(a)) & 0x1fffffff)

#define KSEG0ADDR(a) (CPHYSADDR(a) | KSEG0)
#define KSEG1ADDR(a) (CPHYSADDR(a) | KSEG1)

static inline unsigned int readl(const volatile void *addr)
{
	return *(const volatile unsigned int *)addr;
}
static inline void writel(unsigned int value, volatile void *addr)
{
	*(volatile unsigned int *)addr = value;
}

#define BIT(val) (1 << val)
#define RTL838X_SW_BASE ((volatile void *)0xBB000000)
#define RTL838X_SOC_BASE ((volatile void *)0xB8000000)
#define UART_BASE_ADDR (RTL838X_SOC_BASE + 0x2000)
#define WDT_BASE_ADDR_RTL83XX (RTL838X_SOC_BASE + 0x3150)
#define WDT_BASE_ADDR_RTL93XX (RTL838X_SOC_BASE + 0x3260)
#define WDT_ENABLE BIT(31)
#define WDT_TICKS_PHASE1(val) (val <= 32 ? ((val - 1) << 22) : (31 << 22))
#define WDT_TICKS_PHASE2(val) (val <= 32 ? ((val - 1) << 22) : (31 << 15))
#define WDT_PRESCALE(val) (val <= 3 ? (val << 29) : (3 << 29))
#define WDT_RESET_MODE_SOC 0
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

#define OTTO_WDT_REG_CTRL 0x8

#define RTL8389_FAMILY_ID (0x8389)
#define RTL8328_FAMILY_ID (0x8328)
#define RTL8390_FAMILY_ID (0x8390)
#define RTL8350_FAMILY_ID (0x8350)
#define RTL8380_FAMILY_ID (0x8380)
#define RTL8330_FAMILY_ID (0x8330)
#define RTL9300_FAMILY_ID (0x9300)
#define RTL9310_FAMILY_ID (0x9310)

#define RTL838X_MODEL_NAME_INFO		(0x00D4)
#define RTL838X_CHIP_INFO		(0x00D8)
#define RTL839X_MODEL_NAME_INFO		(0x0FF0)
#define RTL839X_CHIP_INFO		(0x0FF4)
#define RTL93XX_MODEL_NAME_INFO		(0x0004)
#define RTL93XX_CHIP_INFO		(0x0008)


unsigned int pll_reset_value;

#define DRAM_CONFIG_REG 0xb8001004

unsigned int rtl83xx_board_get_memory(void)
{
	unsigned int dcr = readl((volatile void *)DRAM_CONFIG_REG);
	char ROWCNTv[] = { 11, 12, 13, 14, 15, 16 };
	char COLCNTv[] = { 8, 9, 10, 11, 12 };
	char BNKCNTv[] = { 1, 2, 3 };
	char BUSWIDv[] = { 0, 1, 2 };

	return 1 << (BNKCNTv[(dcr >> 28) & 0x3] + BUSWIDv[(dcr >> 24) & 0x3] + ROWCNTv[(dcr >> 20) & 0xf] +
		     COLCNTv[(dcr >> 16) & 0xf]);
}

unsigned int rtl931x_board_get_memory(void)
{
	unsigned int v = readl((volatile void *)0xB814304C);
	unsigned int b = v >> 12;
	unsigned int r = (v >> 6) & 0x3F;
	unsigned int c = v & 0x3F;
	return 1 << (b + r + c);
}

unsigned int board_get_memory(void)
{
	switch (soc_info.family) {
	case RTL9310_FAMILY_ID:
		return rtl931x_board_get_memory();
	default:
		return rtl83xx_board_get_memory();
	}
}
static void rtl838x_watchdog(void)
{
	//soc reset mode 0 + ctrl enable
	unsigned int v = WDT_ENABLE | WDT_TICKS_PHASE1(32) | WDT_TICKS_PHASE2(32) | WDT_PRESCALE(3) | WDT_RESET_MODE_SOC;
	writel(v, WDT_BASE_ADDR_RTL83XX + OTTO_WDT_REG_CTRL);
}
static void rtl839x_watchdog(void)
{
	rtl838x_watchdog();
}

static void rtl930x_watchdog(void)
{
	unsigned int v = WDT_ENABLE | WDT_TICKS_PHASE1(32) | WDT_TICKS_PHASE2(32) | WDT_PRESCALE(3) | WDT_RESET_MODE_SOC;
	writel(v, WDT_BASE_ADDR_RTL93XX + OTTO_WDT_REG_CTRL);
}

static void rtl931x_watchdog(void)
{
	rtl930x_watchdog();
}

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

void board_reset(void)
{
	switch (soc_info.family) {
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

void trigger_watchdog(void)
{
	switch (soc_info.family) {
	case RTL8380_FAMILY_ID:
		rtl838x_watchdog();
		break;
	case RTL8390_FAMILY_ID:
		rtl839x_watchdog();
		break;
	case RTL9300_FAMILY_ID:
		rtl930x_watchdog();
		break;
	case RTL9310_FAMILY_ID:
		rtl931x_watchdog();
		break;
	}
}

void board_watchdog(void)
{
	printf("Init Watchdog...\n");
	trigger_watchdog();
}

void start_memtest(void)
{
	volatile unsigned long *addr, *start, *end;
	unsigned long val;
	unsigned long readback;

	unsigned long incr;
	unsigned long pattern;

	start = (unsigned long *)(KSEG1ADDR(workspace));
#define MEMEND (KSEG1 + (board_get_memory() <= (256 << 20) ? board_get_memory() : 256 << 20))
	end = (unsigned long *)(MEMEND - 0x200000);

	pattern = 0xa0000000;

	incr = 1;
	int i;
	printf("Starting memory test from 0x%08lX to 0x%08lX\n", (unsigned long)start, (unsigned long)end);
	for (i = 0; i < 3; i++) {
		printf("\rPattern %08lX  Writing..."
		       "%12s"
		       "\b\b\b\b\b\b\b\b\b\b",
		       pattern, "");

		for (addr = start, val = pattern; addr < end; addr++) {
			*addr = val;
			val += incr;
		}

		printf(" Reading...\n");

		for (addr = start, val = pattern; addr < end; addr++) {
			readback = *addr;
			if (readback != val) {
				printf("\nMem error @ 0x%08X: "
				       "found %08lX, expected %08lX\n",
				       (unsigned int)addr, readback, val);
			}
			val += incr;
		}

		/*
		 * Flip the pattern each time to make lots of zeros and
		 * then, the next time, lots of ones.  We decrement
		 * the "negative" patterns and increment the "positive"
		 * patterns to preserve this feature.
		 */
		if (pattern & 0x80000000) {
			pattern = -pattern; /* complement & increment */
		} else {
			pattern = ~pattern;
		}
		incr = -incr;
	}
}

const char *get_system_type(void)
{
	return rtl83xx_system_type;
}

static uint32_t read_model_name(void)
{
	uint32_t model, id;

	model = sw_r32(RTL838X_MODEL_NAME_INFO);
	id = model >> 16 & 0xffff;
	if ((id >= 0x8380 && id <= 0x8382) || id == 0x8330 || id == 0x8332) {
		soc_info.id = id;
		soc_info.family = RTL8380_FAMILY_ID;
		return model;
	}

	model = sw_r32(RTL839X_MODEL_NAME_INFO);
	id = model >> 16 & 0xffff;
	if ((id >= 0x8391 && id <= 0x8396) || (id >= 0x8351 && id <= 0x8353)) {
		soc_info.id = id;
		soc_info.family = RTL8390_FAMILY_ID;
		return model;
	}

	model = sw_r32(RTL93XX_MODEL_NAME_INFO);
	id = model >> 16 & 0xffff;
	if (id >= 0x9301 && id <= 0x9303) {
		soc_info.id = id;
		soc_info.family = RTL9300_FAMILY_ID;
		soc_info.revision = model & 0xf;
		return model;
	} else if (id >= 0x9311 && id <= 0x9313) {
		soc_info.id = id;
		soc_info.family = RTL9310_FAMILY_ID;
		soc_info.revision = model & 0xf;
		return model;
	}

	return 0;
}

static void parse_model_name(uint32_t model)
{
	int val, offset, num_chars, pos;
	char suffix[5] = {};

	if (soc_info.family == RTL9300_FAMILY_ID ||
	    soc_info.family == RTL9310_FAMILY_ID) {
		/*
		 * RTL93xx seems to have a flag for engineering samples
		 * instead of a third character.
		 */
		num_chars = 2;
	} else {
		num_chars = 3;
	}

	for (pos = 0; pos < num_chars; pos++) {
		offset = 11 - pos * 5;
		val = (model & (0x1f << offset)) >> offset;

		if (val == 0 || val > 24)
			break;

		suffix[pos] = 'A' + (val - 1);
	}

	if (num_chars == 2 && (model & 0x30)) {
		suffix[pos] = 'E';
		suffix[pos+1] = 'S';
		pos += 2;
	}

	if (pos >= 2 && suffix[pos-2] == 'E' && suffix[pos-1] == 'S') {
		soc_info.testchip = true;
	}

	snprintf(soc_name, sizeof(soc_name), "RTL%04X%s",
		 soc_info.id, suffix);

	soc_info.name = soc_name;
}

static void read_chip_info(void)
{
	uint32_t val = 0;

	switch (soc_info.family) {
	case RTL8380_FAMILY_ID:
		sw_w32(0x3, RTL838X_INT_RW_CTRL);
		sw_w32(0xa << 28, RTL838X_CHIP_INFO);
		val = sw_r32(RTL838X_CHIP_INFO);
		soc_info.revision = (val >> 16) & 0x1f;
		break;

	case RTL8390_FAMILY_ID:
		sw_w32(0xa << 28, RTL839X_CHIP_INFO);
		val = sw_r32(RTL839X_CHIP_INFO);
		soc_info.revision = (val >> 16) & 0x1f;
		break;

	case RTL9300_FAMILY_ID:
	case RTL9310_FAMILY_ID:
		sw_w32(0xa << 16, RTL93XX_CHIP_INFO);
		val = sw_r32(RTL93XX_CHIP_INFO);
		break;
	}

	soc_info.cpu = val & 0xffff;
}

static void rtl83xx_set_system_type(void) {
	char revision = '?';

	if (soc_info.revision > 0 && soc_info.revision <= 24)
		revision = 'A' + (soc_info.revision - 1);

	snprintf(rtl83xx_system_type, sizeof(rtl83xx_system_type),
		 "Realtek %s rev %c (%04X)", soc_info.name, revision, soc_info.cpu);
}

static void detect(void)
{
	uint32_t model;
	model = read_model_name();
	parse_model_name(model);
	read_chip_info();
	rtl83xx_set_system_type();
	printf("Running on %s with %dMB\n", get_system_type(), board_get_memory() >> 20);
	printf("clock period is %d\n", get_clock_period());
	board_watchdog(); // init watchdog and let it run for maximum time, of something hangs board will reset after 60 seconds or so
	//	start_memtest();
}

void board_init(void)
{
	clock_initialize();
	detect();
}

void board_putchar(int ch, void *ctx)
{
	while ((*((volatile unsigned int *)(UART_BASE_ADDR + 0x14)) & 0x20000000) == 0)
		;
	*((volatile unsigned char *)UART_BASE_ADDR) = ch;

	if (ch == '\n')
		board_putchar('\r', ctx);
}
