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
#include "printf.h"

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

unsigned int pll_reset_value;

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

void board_reset(void)
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

void board_watchdog(void)
{
	printf("Init Watchdog...\n");
	switch (family) {
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
	printf("clock period is %d\n", get_clock_period());
}

void board_init(void)
{
	clock_initialize();
	detect();
}

void board_putc(int ch)
{
	while ((*((volatile unsigned int *)(UART_BASE_ADDR + 0x14)) & 0x20000000) == 0)
		;
	*((volatile unsigned char *)UART_BASE_ADDR) = ch;
}
